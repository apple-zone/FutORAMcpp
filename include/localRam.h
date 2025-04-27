#ifndef LOCALRAM_H
#define LOCALRAM_H

#include <vector>
#include "config.h"
#include "Block.h"

class LocalRAM {
public:
    int BLOCK_READ = 0;
    int BLOCK_WRITE = 0;
    int RT_READ = 0;
    int RT_WRITE = 0;
    Config conf;
    std::vector<Block> memory; // 存储块的向量
    string location; // 存储位置

    LocalRAM(const std::string& location, Config conf) : location(location), conf(conf) {
        memory.clear();
        memory.reserve(conf.DATA_SIZE); // 预留内存空间
    }

    Block readBlock(int location) {
        BLOCK_READ++;
        return memory[location];// 读取指定位置的块
    }
    
    void writeBlock(int location, const Block& block) {
        BLOCK_WRITE++;
        memory[location] = block; // 写入指定位置的块
    }

    std::vector<Block> readChunk(int start, int end)
    {
        BLOCK_READ += end - start;
        auto chunkStart = memory.begin() + start;
        auto chunkEnd = memory.begin() + end;
        return std::vector<Block>(chunkStart, chunkEnd); 
    }

    void writeChunk(int start, int end, const std::vector<Block>& blocks) {
        BLOCK_WRITE += end - start;
        auto chunkStart = memory.begin() + start;
        std::copy(blocks.begin(), blocks.end(), chunkStart); // 写入指定范围的块
    }

    std::vector<Block>readChunks(size_t start[], size-t end[] ,int size) {
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

    void writeChunks(size_t start[], size_t end[], const std::vector<Block>& blocks) {
        RT_WRITE += 1;
        int size = blocks.size();
        for (int i = 0; i < size; ++i) {
            writeChunk(start[i], end[i], std::vector<Block>(blocks.begin() + start[i], blocks.begin() + end[i]));
        }
    }

    std::vector<Block> readBalls(size_t locations,size_t size)
    {
        RT_READ += 1;
        std::vector<Block> result;
        for (int i = 0; i < size; ++i) {
            result.push_back(memory[locations + i]);
        }
        return result;
    }

    void writeBalls(size_t locations[], const std::vector<Block>& blocks) {
        RT_WRITE += 1;
        int size = blocks.size();
        for (size_t i = 0; i < size; ++i) {
            writeBlock(locations[i], blocks[i]);
        }
    }

    int getSize() const {
        if(conf.FINAL)
        {
            return 2*conf.DATA_SIZE;
        }
        return conf.DATA_SIZE;
    }

    Block generate_empty_ball_with_key(size_t key) {
        Block empty_ball = Block(key, 0);
        empty_ball.state = 0; 
        return empty_ball;
    }

    void generate_random_memory(size_t number_of_balls)
    {
        for (int i = 0; i < number_of_balls; ++i) {
            Block block = generate_empty_ball_with_key(i);
            memory.push_back(block); // 将块添加到内存中
        }
    }

    void reset_counters() {
        BLOCK_READ = 0;
        BLOCK_WRITE = 0;
        RT_READ = 0;
        RT_WRITE = 0;
    }   

};


#endif