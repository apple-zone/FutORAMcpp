//
// Created by 21885 on 2025/3/31.
//

#ifndef FUTORAMCPP_HASHTABLE_H
#define FUTORAMCPP_HASHTABLE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <random>
#include "Block.h"
#include "UntrustedStorageInterface.h"
#include "ByteOperations.h"
#include "Config.h"
#include "localRam.h"
#include "ObliviousSort.h"
#include "ThresholdGenerator.h"
#include "cuckooHash.h"


class HashTable {
public:
    HashTable( Config conf);
    void rebuild(size_t reals);
    Block lookup(const size_t key);
    void extract();    
    // 配置和组件
    Config conf;
    bool is_built = false;
    ByteOperations byte_ops;
    LocalRAM data_ram;
    LocalRAM bins_ram;
    std::unordered_map<size_t,Block> local_stash;
    LocalRAM overflow_ram;
    LocalRAM second_overflow_ram;
    CuckooHash cuckoo;
    ThresholdGenerator threshold_generator;
    size_t reals_count = 0;

private:


    // 临时存储
    std::unordered_map<std::vector<uint8_t>, Block> local_stash;
    std::vector<Block> dummy_blocks = std::vector<Block>(conf.BIN_SIZE, Block(-1, -1, false));

    // 核心方法
    std::vector<Block> createDummies(size_t count);
    Block getRandomBlock();
    void createReadMemory();
    void blocksIntoBins();
    void cleanWriteMemory();
    void tightCompaction(size_t num_bins, LocalRAM& storage,vector<bool>& isDummy);
    void binsTightCompaction(vector<bool>& isDummy);
    void obliviousBlocksIntoBins();
    void cuckooHashBins();
    void moveSecretLoad();
    void blocksIntoBins();
    void cuckooOverflow();
    void addToStash(const std::vector<Block>& blocks);
    void processblocksIntoBins(const std::vector<Block>& blocks);
    void copyToEndOfBins(LocalRAM& second_data_ram, size_t reals);
    void extract();
    void obliviousBlocksIntoBinsExtract();
    void intersperse();

    // 辅助方法
    std::vector<Block> localTightCompaction(const std::vector<Block>& blocks,
                                            std::vector<bool>& isDummy);

    // 加密相关
    std::mt19937_64 rand_engine;
};



#endif //FUTORAMCPP_HASHTABLE_H
