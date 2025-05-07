//
// Created by 21885 on 2025/4/1.
//
#include "HashTable.h"
#include <algorithm>

HashTable::HashTable(Config conf)
    : conf(conf), byte_ops(), data_ram(conf.DATA_LOCATION, conf),
      bins_ram(conf.BINS_LOCATION, conf), overflow_ram(conf.OVERFLOW_LOCATION, conf),
      second_overflow_ram(conf.OVERFLOW_SECOND_LOCATION, conf), cuckoo(), threshold_generator(conf)
{
    // 初始化随机数生成器
    std::random_device rd;
    rand_engine.seed(rd());
}

vector<Block> HashTable::createDummies(size_t count)
{
    std::vector<Block> dummies;
    for (size_t i = 0; i < count; ++i)
    {
        dummies.push_back(Block(0, 0, 0));
    }
    return dummies;
}

Block HashTable::getRandomBlock()
{
    std::uniform_int_distribution<int> dist(0, conf.NUMBER_OF_BINS - 1);
    int random_index = dist(rand_engine);
    return data_ram.readBlock(random_index);
}

// 待检验
void HashTable::createReadMemory()
{
    int i = 0;
    for (i = 0; i < conf.N; i += conf.BIN_SIZE)
    {
        // 生成随机块并写入bins_ram
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(i), chunks.second.push_back(i + conf.BIN_SIZE);
        vector<Block> random_bin;
        for (int j = 0; j < conf.BIN_SIZE; ++j)
        {
            random_bin.push_back(getRandomBlock());
        }
        data_ram.writeChunks(chunks.first.data(), chunks.second.data(), random_bin, chunks.first.size());
    }
    for (; i < 2 * conf.N; i += conf.BIN_SIZE)
    {
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(i), chunks.second.push_back(i + conf.BIN_SIZE);
        bins_ram.writeChunks(chunks.first.data(), chunks.second.data(), createDummies(conf.BIN_SIZE), chunks.first.size());
    }
}

// 待检验
void HashTable::cleanWriteMemory()
{
    std::pair<vector<size_t>, vector<size_t>> chunks;

    // 清理bins_ram中的数据
    // printf("N:%d\n", conf.N);
    // printf("BIN_SIZE:%d\n", conf.BIN_SIZE);
    // printf("NUMBER_OF_BINS:%d\n", conf.NUMBER_OF_BINS);
    size_t i = 0;
    for (i = 0; i < conf.N * 2; i += conf.BIN_SIZE)
    {
        chunks.first.push_back(i);
        chunks.second.push_back(i + conf.BIN_SIZE);
        // // 生成虚拟块并写入bins_ram
        // vector<vector<Block>> dummy_bin;
        // dummy_bin.push_back(createDummies(conf.BIN_SIZE));

        bins_ram.writeChunks(chunks.first.data(), chunks.second.data(), createDummies(conf.BIN_SIZE), chunks.first.size());
        chunks.first.clear(), chunks.second.clear();
    }
    // 清理overflow_ram中的数据
    int FINAL_OVERFLOW_SIZE = pow(2, log2(conf.N * conf.EPSILON + conf.LOG_LAMBDA * conf.NUMBER_OF_BINS));
    for (size_t i = 0; i < FINAL_OVERFLOW_SIZE; i += conf.BIN_SIZE)

    {
        chunks.first.push_back(i);
        chunks.second.push_back(i + conf.BIN_SIZE);
        // 生成虚拟块并写入bins_ram
        // vector<vector<Block>> dummy_bin;
        // dummy_bin.push_back(createDummies(conf.BIN_SIZE));
        overflow_ram.writeChunks(chunks.first.data(), chunks.second.data(), createDummies(conf.BIN_SIZE), chunks.first.size());
        chunks.first.clear(), chunks.second.clear();
    }
}

void HashTable::emptyData()
{
    for (int i = 0; i < conf.N; i += conf.BIN_SIZE)

    {
        // vector<vector<Block>> dummy_bin ;
        // dummy_bin.push_back(createDummies(conf.BIN_SIZE));
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(i), chunks.second.push_back(i + conf.BIN_SIZE);
        data_ram.writeChunks(chunks.first.data(), chunks.second.data(), createDummies(conf.BIN_SIZE), chunks.first.size());
    }
}

void HashTable::rebuild(size_t reals)
{
    // for(Block block:data_ram.memory)
    // {
    //     printf("rebuild: key: %zu, data: %d\n", block.key, block.data);
    // }
    local_stash.clear();
    blocksIntoBins();
    moveSecretLoad();
    tightCompaction(conf.NUMBER_OF_BINS_IN_OVERFLOW, overflow_ram);
    cuckooHashBins();
    obliviousBlocksIntoBins();
    cuckooOverflow();
    is_built = true;
}

Block HashTable::lookup(size_t key)
{
    Block block = local_stash[key];
    Block result = Block(0, 0, 0); // 默认返回一个虚拟块
    if (block.state == 1)
    {
        result = block;
        local_stash[key] = Block(block.key, block.data, 0);
        reals_count--;
    }
    std::pair<size_t, size_t> location = cuckoo.get_possible_address(key);
    size_t table1_location = location.first;
    size_t table2_location = location.second;

    // 在溢出区查找
    size_t bin_num = byte_ops.keyToPseudoRandomNumber(key, conf.NUMBER_OF_BINS_IN_OVERFLOW);
    Block dummy_block = Block(0, 0, 0);

    // read
    vector<size_t> locations;
    locations.push_back(conf.BIN_SIZE * bin_num + table1_location);
    locations.push_back(conf.BIN_SIZE * bin_num + table2_location + conf.MU);
    vector<Block> blocks = bins_ram.readBlocks(locations.data(), 2);
    Block block1 = blocks[0], block2 = blocks[1];
    Block block1_write, block2_write;
    // table1
    if (block1.state == 1 && block1.key == key)
    {
        result = block1;
        // 生成新块 使用原数据和随机生成的key
        std::uniform_int_distribution<size_t> dist(1ULL << 27, 1ULL << 28);
        size_t new_key = dist(rand_engine);
        block1_write = Block(new_key, block1.data, 2);
        reals_count--;
    }
    else
    {
        block1_write = block1;
    }
    // table2
    if (block2.state == 1 && block2.key == key)
    {
        result = block2;
        // 生成新块 使用原数据和随机生成的key
        std::uniform_int_distribution<size_t> dist(1ULL << 27, 1ULL << 28);
        size_t new_key = dist(rand_engine);
        block2_write = Block(new_key, block2.data, 2);
        reals_count--;
    }
    else
    {
        block2_write = block2;
    }
    overflow_ram.writeBlocks(locations.data(), {block1_write, block2_write});
    // 如果至此已经查找成功 则随机生成一个key继续后面的虚假查找
    if (result.state == 1)
    {
        key = size_t(rand());
    }

    // 在bin中查找
    bin_num = byte_ops.keyToPseudoRandomNumber(key, conf.NUMBER_OF_BINS);
    locations.clear();
    locations.push_back(conf.BIN_SIZE * bin_num + table1_location);
    locations.push_back(conf.BIN_SIZE * bin_num + table2_location + conf.MU);

    // read
    blocks.clear();
    blocks = bins_ram.readBlocks(locations.data(), 2);
    block1 = blocks[0], block2 = blocks[1];

    // table1
    if (block1.state == 1 && block1.key == key)
    {
        result = block1;
        // 生成新块 使用原数据和随机生成的key
        std::uniform_int_distribution<size_t> dist(1ULL << 27, 1ULL << 28);
        size_t new_key = dist(rand_engine);
        block1_write = Block(new_key, block1.data, 2);
        reals_count--;
    }
    else
    {
        block1_write = block1;
    }
    // table2
    if (block2.state == 1 && block2.key == key)
    {
        result = block2;
        // 生成新块 使用原数据和随机生成的key
        std::uniform_int_distribution<size_t> dist(1ULL << 27, 1ULL << 28);
        size_t new_key = dist(rand_engine);
        block2_write = Block(new_key, block2.data, 2);
        reals_count--;
    }
    else
    {
        block2_write = block2;
    }
    bins_ram.writeBlocks(locations.data(), {block1_write, block2_write});
    return result;
}

void HashTable::binsTightCompaction(vector<uint8_t> &states)
{
    tightCompaction(conf.NUMBER_OF_BINS, bins_ram, &states);
}

// 核心方法实现
void HashTable::tightCompaction(size_t num_bins, LocalRAM &storage, vector<uint8_t> *states)
{
    if (states->size() == 0)
    {
        states->push_back(0);
    }
    size_t offset = conf.NUMBER_OF_BINS;
    float distance = 1;
    int midLocation = int(conf.EPSILON * conf.N);
    int iteration = 1;
    while (offset >= 1)
    {
        int start_loc = int(midLocation - midLocation * distance);
        if (iteration >= conf.RAND_CYCLIC_SHIFT_ITERATION)
        {
            randCyclicShift(num_bins, start_loc, storage);
        }
        _tightCompaction(start_loc, storage, offset, *states);

        offset = int(offset / 2);
        distance = (distance / 2);
        iteration++;
    }
}

void HashTable::randCyclicShift(int NUMBER_OF_BINS, size_t start_loc, LocalRAM &storage)
{
    if (NUMBER_OF_BINS == 1)
        return;
    if (NUMBER_OF_BINS <= 2)
    {
        size_t cur_read = start_loc;
        while (cur_read < NUMBER_OF_BINS)
        {
            int number_of_shifts = size_t(2 * conf.MU / NUMBER_OF_BINS);
            vector<Block> blocks = storage.readChunk(cur_read, cur_read + number_of_shifts * NUMBER_OF_BINS);
            vector<size_t> shift_amounts;
            for (int i = 0; i < number_of_shifts; i++)
            {
                shift_amounts.push_back(size_t(rand() % NUMBER_OF_BINS));
            }
            vector<Block> shifted_blocks;
            int start_index = 0;
            for (int i = 0; i < number_of_shifts; i++)
            {
                int shift_amount = shift_amounts[i];
                int end_index = start_index + NUMBER_OF_BINS;
                vector<Block> blocks_to_shift(blocks.begin() + start_index, blocks.begin() + end_index);
                std::rotate(blocks_to_shift.begin(), blocks_to_shift.begin() + shift_amount, blocks_to_shift.end());
                shifted_blocks.insert(shifted_blocks.end(), blocks_to_shift.begin(), blocks_to_shift.end());
                start_index = end_index;
            }
            storage.writeChunk(cur_read, cur_read + number_of_shifts * NUMBER_OF_BINS, shifted_blocks);
            cur_read += number_of_shifts * NUMBER_OF_BINS;
        }
    }
    else
    {
        throw std::runtime_error("Number of bins is too large for random cyclic shift.");
    }
}

void HashTable::_tightCompaction(size_t start_loc, LocalRAM &storage, size_t offset, vector<uint8_t> &states)
{
    for (int i = 0; i < offset; i++)
    {
        vector<Block> blocks = byte_ops.readTransposed(storage, offset, start_loc + i, 2 * conf.MU);
        blocks = localTightCompaction(blocks, states);
        byte_ops.writeTransposed(storage, blocks, offset, start_loc + i);
    }
}
// 辅助方法
std::vector<Block> HashTable::localTightCompaction(const std::vector<Block> &blocks,
                                                   std::vector<uint8_t> &states)
{
    std::vector<Block> dummy, result;
    for (auto block : blocks)
    {
        bool is_dummy = false;
        for(size_t state : states)
        {
            if (block.state == state)
            {
                dummy.push_back(block);
                is_dummy = true;
                break;
            }
        }
        if(!is_dummy) result.push_back(block);

    }
    result.insert(result.end(), dummy.begin(), dummy.end());
    return result;
}

void HashTable::moveSecretLoad()
{
    // 读取数据块
    size_t current_bin = 0, iteration_num = 0;
    while (current_bin < conf.NUMBER_OF_BINS)
    {
        // 这里付出有多少bin是满的
        size_t num = int(1 / conf.EPSILON);
        vector<size_t> start, end;
        for (int i = 0; i < num; i++)
        {
            start.push_back((current_bin+ i) * conf.BIN_SIZE);
            end.push_back((current_bin+i) * conf.BIN_SIZE + 1);
        }
        vector<Block> bins = bins_ram.readChunks(start.data(), end.data(), num);
        vector<size_t> bins_capacity;
        for (int i = 0; i < bins.size(); i++)
        {
            bins_capacity.push_back(bins[i].key); // 这里体现出完全不同的地方 这里不能直接使用python中的字符转换 希望不会出事
        }
        start.clear();
        end.clear();
        for (int i = 0; i < bins_capacity.size(); i++)
        {

            size_t bin_num = i + current_bin;
            size_t end_of_bin = bin_num * conf.BIN_SIZE + bins_capacity[i]+1;
            size_t end_of_bin_minus_epsilon = end_of_bin - int(2 * conf.MU * conf.EPSILON);
            start.push_back(end_of_bin_minus_epsilon);
            end.push_back(end_of_bin);
        }
        vector<Block> blocks = bins_ram.readChunks(start.data(), end.data(), start.size());
        vector<vector<Block>> bins_tops;
        for (int i = 0; i < blocks.size(); i += int(2 * conf.MU * conf.EPSILON))
        {
            vector<Block> bin_top(blocks.begin() + i, blocks.begin() + i + int(2 * conf.MU * conf.EPSILON));
            bins_tops.push_back(bin_top);
        }
        auto chunks = std::make_pair(start, end);
        vector<Block> capacity_threshold_blocks = _moveSecretLoad(bins_capacity, bins_tops, iteration_num, chunks);
        iteration_num++;
        current_bin += num;
        std::pair<vector<size_t>,vector<size_t>> capacity_threshold_chunks;
        capacity_threshold_chunks.first.assign(start.begin(), start.begin() + capacity_threshold_blocks.size());
        capacity_threshold_chunks.second.assign(end.begin(), end.begin() + capacity_threshold_blocks.size());
        bins_ram.writeChunks(capacity_threshold_chunks.first.data(),capacity_threshold_chunks.second.data(), capacity_threshold_blocks, capacity_threshold_blocks.size());
    }
}

vector<Block> HashTable::_moveSecretLoad(vector<size_t> &bins_capacity, vector<vector<Block>> &bins_tops, size_t iteration_num, std::pair<vector<size_t>, vector<size_t>> &chunks)
{
    vector<Block> write_blocks, capacity_threshold_blocks, write_back_blocks;
    size_t count = 0;
    // for(Block block:this->bins_ram.memory)
    // {
    //     if(block.state==1)
    //     {
    //         printf("_move: key: %zu ,data: %d\n", block.key,block.data);
    //     }
    // }
    printf("\n");
    for (int i = 0; i < bins_capacity.size(); i++)
    {
        // 用于跳跃不存在的bins
        if (iteration_num * (1 / conf.EPSILON) + i >= conf.NUMBER_OF_BINS)
            break;

        int threshold = threshold_generator.generate();
        while (threshold >= bins_capacity[i])
        {
            printf("Threshold is larger than capacity of bin.");
        }
        size_t loc = bins_capacity[i] - threshold;
        // 将threshold后面的bins写入到write_blocks中
        vector<Block> sub_bins(bins_tops[i].end()-loc ,bins_tops[i].end() );
        write_blocks.insert(write_blocks.end(), sub_bins.begin(), sub_bins.end());
        count++;

        // 只把threshold前面的bins写入到write_back_blocks中
        vector<Block> sub_bins_2(bins_tops[i].begin(), bins_tops[i].end() - loc);
        vector<Block> dummy_blocks = createDummies(bins_tops[i].size() - sub_bins_2.size());
        sub_bins_2.insert(sub_bins_2.end(), dummy_blocks.begin(), dummy_blocks.end());

        write_back_blocks.insert(write_back_blocks.end(), sub_bins_2.begin(), sub_bins_2.end());
        capacity_threshold_blocks.push_back(byte_ops.constructCapacityThresholdBlock(bins_capacity[i], threshold));
    }
    // 填充虚假块
    vector<Block> dummy_blocks = createDummies(2 * conf.MU - write_blocks.size());
    write_blocks.insert(write_blocks.end(), dummy_blocks.begin(), dummy_blocks.end());
    // 写入转换过的overflow（以便后续的紧确压缩
    byte_ops.writeTransposed(bins_ram, write_blocks, conf.NUMBER_OF_BINS_IN_OVERFLOW, iteration_num);

    vector<size_t> start(chunks.first.begin(), chunks.first.begin() + count), end(chunks.second.begin(), chunks.second.begin() + count);

    bins_ram.writeChunks(start.data(), end.data(), write_back_blocks, start.size());
    return capacity_threshold_blocks;
}
void HashTable::blocksIntoBins()
{
    std::vector<Block> blocks;

    // for(Block block:data_ram.memory)
    // {
    //     printf("data_ram: key: %zu, data: %d\n", block.key, block.data);
    // }

    if (conf.FINAL)
    {
        std::swap(data_ram, bins_ram);
    }

    // for(Block block:bins_ram.memory)
    // {
    //     printf("bins_ram: key: %zu, data: %d\n", block.key, block.data);
    // }

    for (size_t i = 0; i < bins_ram.getSize(); i+=conf.BIN_SIZE)
    {
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(i), chunks.second.push_back(i + conf.BIN_SIZE);
        // 读取bin
        blocks = data_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
        // for(Block block : blocks)
        // {
        //     if (block.state == 1)
        //     {
        //         printf("key: %zu ,data: %d\n", block.key, block.data);
        //     }
        // }
        printf("blocks_into_bins:  %d\n", blocks.size());
        _blocksIntoBins(blocks);
    }

    conf.reset();
}

void HashTable::_blocksIntoBins(const std::vector<Block> &blocks)
{
    std::unordered_map<size_t, std::vector<Block>> bin_map;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, conf.NUMBER_OF_BINS - 1);

    // printf("blocks number: %zu\n", blocks.size());
    // 分桶逻辑
    for (const auto &block : blocks)
    {
        size_t target_bin;
        // 这里随机分桶的逻辑可能要改
        if (block.state == 0)
        {
            target_bin = dist(gen)%conf.NUMBER_OF_BINS; // 随机分桶
            bin_map[target_bin].push_back(Block(0, 0, 0));
        }
        else
        {
            target_bin = byte_ops.blockToPseudoRandomNumber(block, conf.NUMBER_OF_BINS);
            bin_map[target_bin].push_back(block);
        }

    }

    // size_t total = 0;
    // for (const auto& pair : bin_map) {
    //     total += pair.second.size(); // pair.second 是 std::vector<Block>
    // }
    // printf("total: %zu\n", total);

    // 准备写入操作

    // 提取键值
    vector<std::pair<size_t, Block>> bins_capacity;

    for (auto bin_num : bin_map)
    {
        bins_capacity.push_back({bin_num.first, bins_ram.readBlock(bin_num.first*conf.BIN_SIZE)});
    }
    std::pair<vector<size_t>, vector<size_t>> write_chunks;
    std::vector<Block> write_blocks;

    size_t count_new = 0;
    for(Block block:this->bins_ram.memory)
    {
        if(block.state==1)
        {
            printf("before: key: %zu ,data: %d\n", block.key,block.data);
        }
    }
    for (auto [bin_num, cpacity_block] : bins_capacity)
    {
        for(Block block:this->bins_ram.memory)
        {
            if(block.state==1)
            {
                printf("start: key: %zu ,data: %d\n", block.key,block.data);
            }
        }

        size_t capacity = byte_ops.getCapacity(cpacity_block);
        printf("bin_num: %zu\n, capacity: %zu\n", bin_num, capacity);   
        // 容量检查
        if (capacity >= 2 * conf.MU - 1)
        {
            throw std::runtime_error("Bin " + std::to_string(bin_num) + " overflow");
        }

        // 以block为单位的BIN地址
        // 这里bin_loc到bin_write_loc中间的capacity和1分别代表原来的blocks和新加的capacity_block
        size_t bin_loc = bin_num * conf.BIN_SIZE;
        size_t bin_writer_loc = bin_loc + capacity + 1;
        vector<Block> new_blocks = bin_map[bin_num];

        // count_new += new_blocks.size();
        // 已经验证这里的new_capacity数量没错
        // 更新容量
        printf("capacity: %zu, new capacity: %zu\n",capacity, capacity + new_blocks.size());
        Block new_capacity_block = Block(capacity + new_blocks.size(), 0, 1);
        write_chunks.first.push_back(bin_loc),
            write_chunks.second.push_back(bin_loc + 1);
        write_blocks.push_back(new_capacity_block);

        // balls into bin
        write_chunks.first.push_back(bin_writer_loc),
            write_chunks.second.push_back(bin_writer_loc + new_blocks.size());
        // 这里的blocks是两个bin的拼接
        write_blocks.insert(write_blocks.end(), new_blocks.begin(), new_blocks.end());
    }
    // printf("count_new: %zu\n", count_new);
    this->bins_ram.writeChunks(write_chunks.first.data(), write_chunks.second.data(), write_blocks, write_chunks.first.size());
    // for(Block block:write_blocks)
    // {
    //     if(block.state==1)
    //     {
    //         printf("key: %zu ,data: %d\n", block.key,block.data);
    //     }
    // }
    // 这里write_blocks的结构是每个bin的容量块 然后数据块
    for(Block block:this->bins_ram.memory)
    {
        if(block.state==1)
        {
            printf("btob: key: %zu ,data: %d\n", block.key,block.data);
        }
    }
    printf("\n");
}

void HashTable::obliviousBlocksIntoBins()
{
    if (conf.NUMBER_OF_BINS_IN_OVERFLOW <= 1)
    {
        return;
    }
    ObliviousSort oblivious_sort(conf, byte_ops);
    _obliviousBlocksIntoBinsFirstIteration(oblivious_sort);
    LocalRAM next_ram = overflow_ram, current_ram = second_overflow_ram;
    for (int bit_num = 0; bit_num < log2(conf.NUMBER_OF_BINS_IN_OVERFLOW); bit_num++)
    {
        int first_bin_index = 0;
        for (int bin_index = 0; bin_index < conf.NUMBER_OF_BINS_IN_OVERFLOW / 2; bin_index++)
        {
            std::pair<vector<size_t>, vector<size_t>> chunks;
            chunks.first.push_back(first_bin_index * conf.BIN_SIZE), chunks.second.push_back(first_bin_index * conf.BIN_SIZE + conf.BIN_SIZE);
            vector<Block> first_bin = current_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
            chunks.first.clear(), chunks.second.clear();
            chunks.first.push_back((first_bin_index + 1 << bit_num) * conf.BIN_SIZE), chunks.second.push_back((first_bin_index + 1 << bit_num + 1) * conf.BIN_SIZE);
            vector<Block> second_bin = current_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());

            vector<Block> blocks = first_bin;
            blocks.insert(blocks.end(), second_bin.begin(), second_bin.end());
            // 这里的blocks是两个bin的拼接
            auto [bin_0, bin_1] = oblivious_sort.splitToBinsByBit(blocks, ceil(log2(conf.NUMBER_OF_BINS_IN_OVERFLOW)) - 1 - bit_num, conf.NUMBER_OF_BINS_IN_OVERFLOW);
            vector<Block> bins(bin_0.begin(), bin_0.end());
            bins.insert(bins.end(), bin_1.begin(), bin_1.end());
            chunks.first.clear(), chunks.second.clear();
            chunks.first.push_back(bin_index * 2 * conf.BIN_SIZE), chunks.second.push_back((bin_index + 1) * conf.BIN_SIZE + conf.BIN_SIZE);
            next_ram.writeChunks(chunks.first.data(), chunks.second.data(), bins,chunks.first.size());
            ++first_bin_index;
            if (first_bin_index % 1 << bit_num == 0)
            {
                first_bin_index += 1 << bit_num;
            }
        }
        LocalRAM temp = current_ram;
        current_ram = next_ram;
        next_ram = temp;
    }
    overflow_ram = current_ram;
    second_overflow_ram = next_ram;
}

void HashTable::_obliviousBlocksIntoBinsFirstIteration(ObliviousSort &oblivious_sort)
{
    size_t current_bin_pos = 0;
    for (int i = 0; i < conf.NUMBER_OF_BINS_IN_OVERFLOW / 2; i++)
    {
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(current_bin_pos), chunks.second.push_back((current_bin_pos + 1));
        vector<Block> blocks = overflow_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
        auto [bin_0, bin_1] = oblivious_sort.splitToBinsByBit(blocks, 0, conf.NUMBER_OF_BINS_IN_OVERFLOW);
        vector<Block> bins(bin_0.begin(), bin_0.end());
        bins.insert(bins.end(), bin_1.begin(), bin_1.end());
        chunks.first.clear(), chunks.second.clear();
        chunks.first.push_back(2 * current_bin_pos), chunks.second.push_back(2 * current_bin_pos + 2 * conf.BIN_SIZE);
        second_overflow_ram.writeChunks(chunks.first.data(), chunks.second.data(), bins,chunks.first.size());
        current_bin_pos += conf.BIN_SIZE;
    }
}

void HashTable::addToStash(const vector<Block> &blocks)
{
    for (auto block : blocks)
    {
        local_stash[block.key] = block;
    }
}

void HashTable::cuckooHashBins()
{
    size_t current_bin_index = 0;
    while (current_bin_index < conf.NUMBER_OF_BINS)
    {
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(current_bin_index * conf.BIN_SIZE);
        chunks.second.push_back((current_bin_index + 1) * conf.BIN_SIZE);
        // get the bin
        vector<Block> bin_data = bins_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
        // 这里将ball解码为capacity和threshold
        std::pair<size_t, size_t> inf = byte_ops.deconstructCapacityThresholdBall(bin_data[0]);
        size_t capacity = inf.first, threshold = inf.second;
        std::vector<Block> sub_vec(bin_data.begin() + 1, bin_data.begin() + 1 + threshold);
        // 将bin_data中的数据插入到cuckoo hash中
        CuckooHash cuckoo_hash;
        cuckoo_hash.insert_bulk(bin_data);
        // 将用cuckoohash组织过的数据写入stash
        addToStash(cuckoo_hash.get_stash());
        current_bin_index++;
    }
}

void HashTable::cuckooOverflow()
{
    // 读取数据块
    size_t current_bin = 0, iteration_num = 0;
    while (current_bin < conf.NUMBER_OF_BINS_IN_OVERFLOW)
    {
        // 获取bin
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(current_bin * conf.BIN_SIZE);
        chunks.second.push_back((current_bin + 1) * conf.BIN_SIZE);
        vector<Block> bin_data = overflow_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());

        // 生成布谷哈希
        CuckooHash cuckoo_hash;
        cuckoo_hash.insert_bulk(bin_data);

        // 写入overflow_ram
        vector<Block> cuckoo_data = cuckoo.get_cuckoo_data();
        overflow_ram.writeChunks(chunks.first.data(), chunks.second.data(), cuckoo_data,chunks.first.size());

        // 将用cuckoohash组织过的数据写入stash
        addToStash(cuckoo.get_stash());
        current_bin++;
    }
}




// 将前一层的数据写入到当前层
void HashTable::copyToEndOfBins(LocalRAM &seconf_data_ram, size_t reals)
{
    size_t current_pos = 0;
    this->reals_count += reals;

    while (current_pos < conf.N)
    {
        std::pair<vector<size_t>, vector<size_t>> chunks;
        chunks.first.push_back(current_pos);
        chunks.second.push_back(current_pos + conf.BIN_SIZE);
        vector<Block> blocks = bins_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
        chunks.first.clear(), chunks.second.clear();
        chunks.first.push_back(conf.N + current_pos);
        chunks.second.push_back(conf.N + current_pos + conf.BIN_SIZE);
        current_pos += conf.BIN_SIZE;
    }
}

void HashTable::extract()
{
    obliviousBlocksIntoBinsExtract();
    size_t blocks_written = 0;
    vector<Block> stash;
    for (auto block : local_stash)
    {
        if (block.second.state == 1 == 0)
        {
            continue;
        }
        stash.push_back(block.second);
    }
    for (int i = 0; i < conf.NUMBER_OF_BINS_IN_OVERFLOW; i++)
    {
        vector<Block> overflow_bin = overflow_ram.readChunk(i * conf.BIN_SIZE, (i + 1) * conf.BIN_SIZE);
        for (int j = 0; j < int(1 / conf.EPSILON); j++)
        {
            // 读取要extract的bin
            vector<Block> bin = bins_ram.readChunk((i * int(1 / conf.EPSILON) + j) * conf.BIN_SIZE, (i * int(1 / conf.EPSILON) + j + 1) * conf.BIN_SIZE);
            for (auto block : overflow_bin)
            {
                if (byte_ops.blockToPseudoRandomNumber(block, conf.NUMBER_OF_BINS_IN_OVERFLOW) == i * int(1 / conf.EPSILON) + j)
                {
                    bin.push_back(block);
                }
            }
            for (auto block : stash)
            {
                if (byte_ops.blockToPseudoRandomNumber(block, conf.NUMBER_OF_BINS_IN_OVERFLOW) == i * int(1 / conf.EPSILON) + j)
                {
                    bin.push_back(block);
                }
            }
            // 这里需要将bin中的数据写入到bins_ram中
            vector<Block> bin_to_write;
            for (auto block : bin)
            {
                if (block.state == 0)
                {
                    continue;
                }
                bin_to_write.push_back(block);
            }
            bins_ram.writeChunk(blocks_written, blocks_written + bin_to_write.size(), bin_to_write);
            blocks_written += bin_to_write.size();
        }
    }
}

void HashTable::obliviousBlocksIntoBinsExtract()
{
    if (conf.NUMBER_OF_BINS_IN_OVERFLOW <= 1)
    {
        vector<Block> bin = overflow_ram.readChunk(0, conf.BIN_SIZE);
        vector<uint8_t> states;
        states.push_back(0);
        bin = localTightCompaction(bin, states);
        overflow_ram.writeChunk(0, conf.BIN_SIZE, bin);
        return;
    }
    ObliviousSort oblivious_sort(conf, byte_ops);
    LocalRAM current_ram = overflow_ram, next_ram = second_overflow_ram;
    for (int bit_num = 0; bit_num < log2(conf.NUMBER_OF_BINS_IN_OVERFLOW); bit_num++)
    {
        size_t first_bin_index = 0;
        for (int bin_index = 0; bin_index < ceil(conf.NUMBER_OF_BINS_IN_OVERFLOW / 2); ++bin_index)
        {
            std::pair<vector<size_t>, vector<size_t>> chunks;
            chunks.first.push_back(first_bin_index * conf.BIN_SIZE);
            chunks.second.push_back((first_bin_index + 1) * conf.BIN_SIZE);
            vector<Block> first_bin = current_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
            chunks.first.clear(), chunks.second.clear();
            chunks.first.push_back((first_bin_index + pow(2, bit_num)) * conf.BIN_SIZE);
            chunks.second.push_back((first_bin_index + pow(2, bit_num) + 1) * conf.BIN_SIZE);

            auto [bin_0, bin_1] = oblivious_sort.splitToBinsByBit(first_bin, bit_num, conf.NUMBER_OF_BINS_IN_OVERFLOW);

            chunks.first.clear(), chunks.second.clear();
            chunks.first.push_back(bin_index * 2 * conf.BIN_SIZE);
            chunks.second.push_back((bin_index + 1) * 2 * conf.BIN_SIZE);
            vector<Block> bins(bin_0.begin(), bin_0.end());
            bins.insert(bins.end(), bin_1.begin(), bin_1.end());
            next_ram.writeChunks(chunks.first.data(), chunks.second.data(), bins,chunks.first.size());
            first_bin_index++;
            if (first_bin_index % 2 * pow(2, bit_num) == 0)
            {
                first_bin_index += 2 * pow(2, bit_num);
            }
        }
        LocalRAM temp = current_ram;
        current_ram = next_ram;
        next_ram = temp;
    }
    overflow_ram = current_ram;
    second_overflow_ram = next_ram;
}

void HashTable::intersperse()
{
    size_t offset = conf.NUMBER_OF_BINS;
    double distance_from_center = 1;
    size_t midLocation = int(conf.EPSILON * conf.N);
    while (offset >= 1)
    {
        offset /= 2;
        distance_from_center /= 2;
    }
    while (offset <= conf.NUMBER_OF_BINS)
    {
        size_t start_loc = int(midLocation - midLocation * distance_from_center);
        _intersperse(start_loc, bins_ram, offset);

        offset *= 2;
        distance_from_center *= 2;
    }
}

void HashTable::_intersperse(size_t start_loc, LocalRAM &storage, size_t offset)
{
    for (int i = 0; i < offset; i++)
    {
        vector<Block> blocks = byte_ops.readTransposed(storage, offset, start_loc + i, 2 * conf.MU);
        // 创建一个随机数生成器 打乱向量中的元素
        std::default_random_engine rng;
        std::shuffle(blocks.begin(), blocks.end(), rng);
        byte_ops.writeTransposed(storage, blocks, offset, start_loc + i);
    }
}