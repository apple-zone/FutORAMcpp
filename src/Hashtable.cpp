//
// Created by 21885 on 2025/4/1.
//
#include "HashTable.h"
#include <algorithm>
#include <openssl/evp.h>

HashTable::HashTable(const Config& config)
        : conf(config),
          byte_ops(config.MAIN_KEY, config),
          rand_engine(std::random_device{}()) {

    // 初始化存储接口
    data_ram = std::make_unique<UntrustedStorageInterface>();
    bins_ram = std::make_unique<UntrustedStorageInterface>();
    overflow_ram = std::make_unique<UntrustedStorageInterface>();
    second_overflow_ram = std::make_unique<UntrustedStorageInterface>();

    // 初始化dummy块
    dummy_blocks.reserve(config.BIN_SIZE);
    for (size_t i = 0; i < config.BIN_SIZE; ++i) {
        dummy_blocks.emplace_back(-1, -1, new int[Block::BLOCK_SIZE]{0});
    }
}

void HashTable::rebuild(size_t reals) {
    local_stash.clear();
    ballsIntoBins();
    moveSecretLoad();
    tightCompaction(conf.NUMBER_OF_BINS_IN_OVERFLOW, *overflow_ram);
    cuckooHashBins();
    obliviousBallsIntoBins();
    // 其他重建步骤...
}

Block HashTable::lookup(const std::vector<uint8_t>& key) {
    // 本地暂存查找
    auto it = local_stash.find(key);
    if (it != local_stash.end()) {
        Block result = it->second;
        result.leaf_id = -1; // 标记为已访问
        return result;
    }

    // Cuckoo哈希查找
    auto [pos1, pos2] = cuckoo.getPossibleAddresses(key);

    // 主存储查找
    Bucket bucket1 = bins_ram->ReadBucket(pos1);
    Bucket bucket2 = bins_ram->ReadBucket(pos2);

    // 查找逻辑...

    return Block(); // 返回找到的块或dummy块
}

// 核心方法实现
void HashTable::tightCompaction(size_t num_bins, UntrustedStorageInterface& storage) {
    for (size_t i = 0; i < num_bins; ++i) {
        Bucket bucket = storage.ReadBucket(i);
        std::vector<Block> valid_blocks;

        // 过滤无效块
        std::copy_if(bucket.blocks.begin(), bucket.blocks.end(),
                     std::back_inserter(valid_blocks),
                     [](const Block& b) { return b.leaf_id != -1; });

        // 填充dummy块
        valid_blocks.resize(bucket.blocks.size(), Block(-1, -1, new int[Block::BLOCK_SIZE]{0}));

        storage.WriteBucket(i, Bucket(valid_blocks));
    }
}

void HashTable::obliviousBallsIntoBins() {
    ObliviousSort sorter(conf);
    // 实现Oblivious排序逻辑...
}

void HashTable::cuckooHashBins() {
    for (size_t i = 0; i < conf.NUMBER_OF_BINS; ++i) {
        Bucket bucket = bins_ram->ReadBucket(i);
        // 实现Cuckoo哈希插入...
    }
}

// 辅助方法
std::vector<Block> HashTable::localTightCompaction(const std::vector<Block>& blocks) {
    std::vector<Block> result;
    std::vector<Block> dummies;

    for (const auto& block : blocks) {
        if (block.leaf_id == -1) {
            dummies.push_back(block);
        } else {
            result.push_back(block);
        }
    }
    result.insert(result.end(), dummies.begin(), dummies.end());
    return result;
}

std::vector<uint8_t> HashTable::generateRandomBlock() {
    std::vector<uint8_t> block(conf.BALL_SIZE);
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    for (auto& byte : block) {
        byte = dist(rand_engine);
    }
    return block;
}