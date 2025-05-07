#include "localRam.h"
#include <algorithm>  // for std::copy

// 初始化静态成员
int LocalRAM::BLOCK_READ = 0;
int LocalRAM::BLOCK_WRITE = 0;
int LocalRAM::RT_READ = 0;
int LocalRAM::RT_WRITE = 0;

// 构造函数
LocalRAM::LocalRAM(const std::string& location, Config conf)
    : location(location), conf(conf) {
    memory.clear();
    memory.reserve(2*conf.N);
    reset_counters();  // 初始化时重置计数器
}

// 读取单个块
Block LocalRAM::readBlock(int location) {
    BLOCK_READ++;
    return memory[location];
}

// 写入单个块
void LocalRAM::writeBlock(int location, const Block& block) {
    BLOCK_WRITE++;
    memory[location] = block;
}

// 读取连续块
std::vector<Block> LocalRAM::readChunk(int start, int end) {
    BLOCK_READ += end - start;
    auto chunkStart = memory.begin() + start;
    auto chunkEnd = memory.begin() + end;
    return std::vector<Block>(chunkStart, chunkEnd);
}

// 写入连续块
void LocalRAM::writeChunk(int start, int end, const std::vector<Block>& blocks) {

    BLOCK_WRITE += end - start;
    if(start>=memory.size()){
        memory.resize(start+blocks.size());
    }
    if (start + blocks.size() > memory.size()) {
        memory.resize(start + blocks.size());
    }
    auto chunkStart = memory.begin() + start;
    std::copy(blocks.begin(), blocks.end(), chunkStart);
}

// 读取多个不连续块
std::vector<Block> LocalRAM::readChunks(size_t start[], size_t end[], int size) {
    RT_READ += 1;
    std::vector<Block> result;
    for (int i = 0; i < size; ++i) {
        auto chunkStart = memory.begin() + start[i];
        auto chunkEnd = memory.begin() + end[i];
        std::vector<Block> chunk(chunkStart, chunkEnd);
        result.insert(result.end(), chunk.begin(), chunk.end());
    }
    return result;
}

// 写入多个不连续块
void LocalRAM::writeChunks(size_t start[], size_t end[], const vector<Block>& blocks,size_t size) {
    RT_WRITE += 1;
    size_t block_count = 0;
    for(size_t i = 0; i < size; ++i) {
        printf("start: %zu, end: %zu\n", start[i], end[i]);
    }
    for (int i = 0; i < size; ++i) {
        size_t chunkSize = end[i] - start[i];
        std::vector<Block> blocks_in_chunk(blocks.begin() + block_count , blocks.begin() + block_count + chunkSize);
        writeChunk(start[i], end[i], blocks_in_chunk);
        block_count += chunkSize;
    }
    for(size_t i = 0;i<memory.size();i++){
        Block block = memory[i];
        if(block.state==1){
            printf("write: key: %zu, data: %d, which bin : %d, i : %d\n", block.key, block.data, i/conf.BIN_SIZE, i);
            if((i%conf.BIN_SIZE!=0)){
                memory[i].state = 0;
            }
        }
    }
    printf("second time:\n");
    for(size_t i = 0;i<memory.size();i++){
        Block block = memory[i];
        if(block.state==1){
            printf("write: key: %zu, data: %d, which bin : %d, i : %d\n", block.key, block.data, i/conf.BIN_SIZE, i);
        }
    }
}

// 读取多个离散块
std::vector<Block> LocalRAM::readBlocks(size_t locations[], size_t size) {
    RT_READ += 1;
    std::vector<Block> result;
    for (size_t i = 0; i < size; ++i) {
        result.push_back(memory[locations[i]]);
    }
    return result;
}

// 写入多个离散块
void LocalRAM::writeBlocks(size_t locations[], const std::vector<Block>& blocks) {
    RT_WRITE += 1;
    size_t size = blocks.size();
    for (size_t i = 0; i < size; ++i) {
        writeBlock(locations[i], blocks[i]);
    }
}

// 获取内存大小
int LocalRAM::getSize() const {
    return conf.FINAL ? 2 * conf.N : conf.N;
}

// 生成空块
Block LocalRAM::generate_empty_ball_with_key(size_t key) {
    Block empty_ball(key, 0);
    empty_ball.state = 1;
    return empty_ball;
}

// 初始化随机内存
void LocalRAM::generate_random_memory(size_t number_of_balls) {
    for (size_t i = 0; i < number_of_balls; ++i) {
        Block block = generate_empty_ball_with_key(i);
        this->memory.push_back(block);
    }
}

// 重置计数器
void LocalRAM::reset_counters() {
    BLOCK_READ = 0;
    BLOCK_WRITE = 0;
    RT_READ = 0;
    RT_WRITE = 0;
}