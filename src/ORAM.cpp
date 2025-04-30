#include "ORAM.h"
#include <cmath>
#include <algorithm>

ORAM::ORAM(int number_of_blocks,string data_location)
    : conf(Config()), original_data_ram(data_location,this->conf),
      byte_ops(), rng(std::random_device{}()),
      stash_reals_count(0), read_count(0), not_found(0)
{
    // tables.reserve(conf.NUMBER_OF_LEVELS);
    // size_t current_number_of_blocks = conf.MU;
    // for (int i = 0; i < conf.NUMBER_OF_LEVELS; ++i)
    // {
    //     tables.emplace_back(conf);
    // }
}


void ORAM::cleanWriteMemory()
{
    for (auto &table : tables)
    {
        table.cleanWriteMemory();
    }
    HashTable &final_table = tables.back();
    final_table.cleanWriteMemory();
}

void ORAM ::initialBuild(const std::string &data_location)
{
    HashTable &final_table = tables.back();
    LocalRAM &final_ram = final_table.data_ram;
    original_data_ram.generate_random_memory(final_table.conf.N);
    final_table.data_ram = original_data_ram;
    final_table.rebuild(final_table.conf.N);
    original_data_ram = final_table.data_ram; // 这一句不知道要不要
    final_table.data_ram = final_ram;
}

// 生成哈希键（使用AES加密）
// size_t ORAM::generateHashKey(const Block& block) const {
//     const auto& key_data = block.getKeyData();
//     return byte_ops_.keyToPseudoRandomNumber(
//         reinterpret_cast<const size_t&>(key_data[0]),
//         conf.HASH_MOD_LIMIT
//     );
// }

Block generateRandomBlock(uint8_t state = NULL)
{   std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<size_t> dist(0, 1ULL << 28);
    size_t key = dist(rng);
    std::uniform_int_distribution<int> dist2(0, 1ULL << 28);
    int value = dist2(rng);
    if(state!= NULL)
    {
        return Block(size_t(value),int(key), state);
    }
    return Block(key, value, 1);
}

Block ORAM::access(const std::string &op, const Block &block)
{
    size_t key = block.key;
    size_t value = block.data;
    // size_t original_key = generateHashKey(block);


    bool is_found = false;
    Block result_block;
    const size_t original_key = key;

    // 本地存储查找
    auto it = local_stash.find(key);
    if (it != local_stash.end())
    {
        result_block = it->second;

        is_found = true;

        // 生成伪随机替换键
        std::uniform_int_distribution<size_t> dist(1ULL << 27, 1ULL << 28);
        size_t dummy_hash = dist(rng);
        local_stash.emplace(dummy_hash, Block(dummy_hash, dummy_hash, 2));
    }

    // 哈希表层级查找
    for (auto &table : tables)
    {
        if (table.is_built && !is_found)
        {
            Block block = table.lookup(key);
            if (block.key == key)
            {
                result_block = block;
                is_found = true;
                std::uniform_int_distribution<size_t> dist(1ULL, 1ULL << 28);
                key = dist(rng); // 生成新的伪随机键
            }
            else if (table.is_built && is_found)
            {
                table.lookup(key);
            }
        }
    }

    // stash不论查找结果都要加东西进去
    if (!is_found || local_stash.count(original_key))
    {
        Block new_block = generateRandomBlock(0);
        local_stash[new_block.key] = new_block;
    }
    else
    {
        ++stash_reals_count;
    }

    // 操作处理
    //这里容易出错 要检查
    if(!is_found)
    {
        not_found++;
    }
    else if (op == "read")
    {
        local_stash[original_key] = result_block;
    }
    else if (op == "write")
    {
        Block new_block(result_block.key, value, true);
        local_stash[original_key] = new_block;
    }

    // 重建逻辑
    if (++read_count >= conf.MU)
    {
        read_count = 0;
        rebuild();
        local_stash.clear();
        stash_reals_count = 0;
    }

    return result_block;
}

void ORAM::rebuild()
{
    if(!tables[0].is_built)
    {
        rebuildLevelOne();
        return;
    }

    //提取已经built的层
    extractLevelOne();
    for(auto &table = tables.begin()+1; table != tables.end(); ++table)
    {
        if(table->is_built)
        {
            table->extract();
        }else break;
    }

    for(int i = 1; i<tables.size(); i++)
    {
        HashTable &previous_table = tables[i-1],&current_table = tables[i];
        if(current_table.is_built)
        {
            current_table.copyToEndOfBins(previous_table.bins_ram, previous_table.reals_count);
            current_table.intersperse();
            current_table.is_built = false;
        }else{
            current_table.data_ram = previous_table.data_ram;
            current_table.rebuild(previous_table.reals_count);
            return;
        }
    }
    HashTable &final_table = tables.back();


    // 为了提高效率 再最后的构建中 写地址调整
    final_table.conf.FINAL = true;
    vector<uint8_t> dummy_states;
    dummy_states.push_back(0),dummy_states.push_back(2);
    final_table.binsTightCompaction(dummy_states);
    final_table.rebuild(final_table.conf.N);
}


void ORAM::extractLevelOne()
{
    HashTable hash_table_one = tables[0];
    vector<Block> blocks = hash_table_one.data_ram.readChunk(0, hash_table_one.conf.BIN_SIZE);
    vector<Block> blocks_in_stash;
    for (auto block : hash_table_one.local_stash)
    {

        blocks_in_stash.push_back(block.second);
    }
    for(auto block : local_stash)
    {
        blocks_in_stash.push_back(block.second);
    }
    blocks.insert(blocks.end(), blocks_in_stash.begin(), blocks_in_stash.end());
    blocks_in_stash.clear();
    for(auto block : blocks)
    {
        if(block.state == 0)
        {
            continue;
        }
        blocks_in_stash.push_back(block);
    }
    cuckoo.shuffleBlocks(blocks_in_stash);
    hash_table_one.bins_ram.writeChunk(0, hash_table_one.conf.BIN_SIZE, blocks_in_stash);
    hash_table_one.is_built = false;
}

void ORAM::tightCompactionLevelOne()
{
    HashTable hash_table_one = tables[0];
    vector<Block> blocks = hash_table_one.bins_ram.readChunk(0, hash_table_one.conf.BIN_SIZE);
    vector<uint8_t> states;
    states.push_back(0);
    blocks = hash_table_one.localTightCompaction(blocks, states);
    vector<Block> blocks_in_stash;
    for(auto block : hash_table_one.local_stash)
    {
        blocks_in_stash.push_back(block.second);
    }
    vector<Block> tmp_blocks(blocks.begin(), blocks.begin()+(conf.MU - blocks.size()));
    tmp_blocks.insert(tmp_blocks.end(), blocks_in_stash.begin(), blocks_in_stash.end());
    cuckoo.shuffleBlocks(tmp_blocks);
    std:pair<vector<size_t>, vector<size_t>> chunks;
    chunks.first.push_back(0);
    chunks.second.push_back(conf.MU);
    hash_table_one.bins_ram.writeChunks(chunks.first.data(),chunks.second.data(),tmp_blocks);

}


void ORAM::intersperseStashAndLevelOne()
{
    HashTable hash_table_one = tables[0];
    vector<Block> blocks_in_stash;
    for(auto block : hash_table_one.local_stash)
    {
        blocks_in_stash.push_back(block.second);
    }

    std::pair<vector<size_t>, vector<size_t>> chunks;
    chunks.first.push_back(0);
    chunks.second.push_back(conf.MU);
    vector<Block> blocks = hash_table_one.bins_ram.readChunks(chunks.first.data(), chunks.second.data(), chunks.first.size());
    blocks_in_stash.insert(blocks_in_stash.end(), blocks.begin(), blocks.end());
    chunks.first.clear(), chunks.second.clear();
    chunks.first.push_back(0),chunks.second.push_back(2*hash_table_one.conf.MU);
    hash_table_one.bins_ram.writeChunks(chunks.first.data(), chunks.second.data(), blocks_in_stash);
    hash_table_one.reals_count+=stash_reals_count;
    hash_table_one.is_built = false;
}


void ORAM::rebuildLevelOne()
{
    HashTable hash_table_one = tables[0];
    CuckooHash cuckoo_hash = CuckooHash();
    vector<Block> blocks_in_stash;
    for(auto block : hash_table_one.local_stash)
    {
        blocks_in_stash.push_back(block.second);
    }
    cuckoo_hash.insert_bulk(blocks_in_stash);
    std::pair<vector<size_t>, vector<size_t>> chunks;
    chunks.first.push_back(0),chunks.second.push_back(hash_table_one.conf.BIN_SIZE);
    vector<Block> blocks_in_cuckoo = cuckoo_hash.get_cuckoo_data();
    hash_table_one.bins_ram.writeChunks(chunks.first.data(), chunks.second.data(), blocks_in_cuckoo);
    hash_table_one.local_stash = hash_table_one.byte_ops.blocksToDictionary(cuckoo_hash.get_stash());
    hash_table_one.is_built = true;
    hash_table_one.reals_count = stash_reals_count;
}