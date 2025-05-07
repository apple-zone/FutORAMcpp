#ifndef CUCKOOHASH_H
#define CUCKOOHASH_H
#include <vector>
#include "Block.h"
#include "ByteOperations.h"
#include "config.h"
#include <random>


class CuckooHash {
private:
    Config conf;
    std::vector<Block> table1;
    std::vector<Block> table2;
    ByteOperations table1_byte_operation;
    ByteOperations table2_byte_operation;
    std::vector<Block> stash;

public:
    CuckooHash():conf(0), table1_byte_operation(), table2_byte_operation() {
        Block dummy = Block();
        table1 = std::vector<Block>(conf.MU, dummy);
        table2 = std::vector<Block>(conf.MU, dummy);
        stash = std::vector<Block>(conf.STASH_SIZE, dummy);
    }

    void shuffleBlocks(std::vector<Block>& blocks) {
        std::random_device rd;  // 用于生成随机种子
        std::default_random_engine rng(rd()); // 随机数生成器
        std::shuffle(blocks.begin(), blocks.end(), rng); // 打乱 balls
    }

    void insert_block(Block block) {
        vector<int> seen_locations; // 记录已见过的块的位置
        while (true)
        {
            int location = table1_byte_operation.blockToPseudoRandomNumber(block,conf.MU);
            Block& evicted_block = table1[location];
            table1[location] = block; // 插入块
            if(evicted_block.state == 0) {
                break; // 如果被驱逐的块是虚拟块，插入成功
            }
            block = evicted_block; // 否则，继续驱逐块
            location = table2_byte_operation.blockToPseudoRandomNumber(block,conf.MU);
            evicted_block = table2[location];
            table2[location] = block; // 插入块
            if(evicted_block.state == 0) {
                break; // 如果被驱逐的块是虚拟块，插入成功
            }
            block = evicted_block; // 否则，继续驱逐块
            if(seen_locations.size() >= 2*conf.MU) {
                if(block.state == 1)
                {
                    stash.push_back(block); // 如果被驱逐的块是实块，插入到stash中
                }
            }
            seen_locations.push_back(location); // 记录已见过的块的位置
        }
        
    }

    void insert_bulk(vector<Block> blocks) {
        // 1. 打乱 balls
        shuffleBlocks(blocks);

        // 2. 将 balls 插入到 cuckoo hash 表中
        for (const auto& block : blocks) {
            this->insert_block(block);
        }
        
    }

    std::pair<int,int> get_possible_address(size_t key)
    {
        int location1 = table1_byte_operation.keyToPseudoRandomNumber(key,conf.MU);
        int location2 = table2_byte_operation.keyToPseudoRandomNumber(key,conf.MU);
        return std::make_pair(location1, location2);
    }

    std::vector<Block> get_stash() {
        return stash;
    }

    std::vector<Block> get_cuckoo_data() {
        std::vector<Block> cuckoo_data;
        cuckoo_data.insert(cuckoo_data.end(), table1.begin(), table1.end());
        cuckoo_data.insert(cuckoo_data.end(), table2.begin(), table2.end());
        return cuckoo_data;
    }
};

#endif