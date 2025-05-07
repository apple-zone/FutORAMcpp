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
#include "ByteOperations.h"
#include "Config.h"
#include "localRam.h"
#include "ObliviousSort.h"
#include "ThresholdGenerator.h"
#include "cuckooHash.h"

class HashTable
{
public:
    HashTable(Config conf);
    void rebuild(size_t reals);
    Block lookup(size_t key);
    // 配置和组件
    Config conf;
    bool is_built = false;
    ByteOperations byte_ops;
    LocalRAM data_ram;
    LocalRAM bins_ram;
    std::unordered_map<size_t, Block> local_stash;
    LocalRAM overflow_ram;
    LocalRAM second_overflow_ram;
    CuckooHash cuckoo;
    ThresholdGenerator threshold_generator;
    size_t reals_count = 0;
    std::vector<Block> createDummies(size_t count);
    Block getRandomBlock();
    void createReadMemory();
    void cleanWriteMemory();
    void emptyData();
    void tightCompaction(size_t num_bins, LocalRAM &storage, vector<uint8_t> *states = nullptr);
    void _tightCompaction(size_t start_loc, LocalRAM &storage, size_t offset, vector<uint8_t> &states);
    void randCyclicShift(int NUMBER_OFBINS, size_t start_loc, LocalRAM &storage);
    void binsTightCompaction(vector<uint8_t> &states);
    void obliviousBlocksIntoBins();
    void cuckooHashBins();
    void moveSecretLoad();
    vector<Block> _moveSecretLoad(vector<size_t> &bins_capacity, vector<vector<Block>> &bins_tops, size_t iteration_num, std::pair<vector<size_t>, vector<size_t>> &chunks);
    void blocksIntoBins();
    void _blocksIntoBins(const std::vector<Block> &blocks);
    void cuckooOverflow();
    void addToStash(const std::vector<Block> &blocks);
    void copyToEndOfBins(LocalRAM &second_data_ram, size_t reals);
    void extract();
    void obliviousBlocksIntoBinsExtract();
    void _obliviousBlocksIntoBinsFirstIteration(ObliviousSort &oblivious_sort);
    void intersperse();
    void _intersperse(size_t start_loc, LocalRAM &storage, size_t offset);

    // 辅助方法
    std::vector<Block> localTightCompaction(const std::vector<Block> &blocks,
                                            std::vector<uint8_t> &states);

private:
    // 临时存储
    std::vector<Block> dummy_blocks = std::vector<Block>(conf.BIN_SIZE, Block(0,0,0));

    // 核心方法

    // 加密相关
    std::mt19937_64 rand_engine;
};

#endif // FUTORAMCPP_HASHTABLE_H
