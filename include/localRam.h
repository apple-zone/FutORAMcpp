#ifndef LOCALRAM_H
#define LOCALRAM_H

#include <vector>
#include <string>
#include "config.h"
#include "Block.h"

class LocalRAM {
public:
    // 静态计数器
    static int BLOCK_READ;
    static int BLOCK_WRITE;
    static int RT_READ;
    static int RT_WRITE;

    // 构造函数
    LocalRAM(const std::string& location, Config conf);

    // 基础读写操作
    Block readBlock(int location);
    void writeBlock(int location, const Block& block);

    // 批量读写操作
    std::vector<Block> readChunk(int start, int end);
    void writeChunk(int start, int end, const std::vector<Block>& blocks);

    // 随机访问操作
    std::vector<Block> readChunks(size_t start[], size_t end[], int size);
    void writeChunks(size_t start[], size_t end[], const vector<Block>& blocks,size_t size);
    std::vector<Block> readBlocks(size_t locations[], size_t size);
    void writeBlocks(size_t locations[], const std::vector<Block>& blocks);

    // 工具函数
    int getSize() const;
    Block generate_empty_ball_with_key(size_t key);
    void generate_random_memory(size_t number_of_balls);

    // 静态方法
    static void reset_counters();
    Config conf;
    std::vector<Block> memory;
    std::string location;
private:

};

#endif