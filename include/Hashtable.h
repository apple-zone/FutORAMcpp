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
#include "CuckooHash.h"
#include "ObliviousSort.h"

class HashTable {
public:
    explicit HashTable(const Config& config);
    void rebuild(size_t reals);
    Block lookup(const std::vector<uint8_t>& key);
    void extract();

private:
    // 配置和组件
    Config conf;
    ByteOperations byte_ops;
    std::unique_ptr<UntrustedStorageInterface> data_ram;
    std::unique_ptr<UntrustedStorageInterface> bins_ram;
    std::unique_ptr<UntrustedStorageInterface> overflow_ram;
    std::unique_ptr<UntrustedStorageInterface> second_overflow_ram;
    CuckooHash cuckoo;

    // 临时存储
    std::unordered_map<std::vector<uint8_t>, Block> local_stash;
    std::vector<Block> dummy_blocks;

    // 核心方法
    void createDummies(size_t count);
    void tightCompaction(size_t num_bins, UntrustedStorageInterface& storage);
    void obliviousBallsIntoBins();
    void cuckooHashBins();
    void moveSecretLoad();
    void ballsIntoBins();

    // 辅助方法
    std::vector<Block> localTightCompaction(const std::vector<Block>& blocks);
    void writeTransposed(std::unique_ptr<UntrustedStorageInterface>& ram,
                         const std::vector<Block>& blocks, size_t offset);
    std::vector<Block> readTransposed(std::unique_ptr<UntrustedStorageInterface>& ram,
                                      size_t offset, size_t length);

    // 加密相关
    std::mt19937_64 rand_engine;
    std::vector<uint8_t> generateRandomBlock();
};



#endif //FUTORAMCPP_HASHTABLE_H
