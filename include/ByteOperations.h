#ifndef BYTEOPERATIONS_H
#define BYTEOPERATIONS_H

#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <openssl/aes.h>
#include "Config.h"
#include "Block.h"
#include "localRam.h"

class ByteOperations
{
public:
    // 构造函数
    ByteOperations();
    explicit ByteOperations(const std::vector<size_t> &key, const Config &config);

    // 位操作
    bool isBitOn(uint64_t number, size_t bit_num) const;

    // 容量球操作
    size_t getCapacity( Block &capacity_ball) ;
    // std::vector<size_t> constructCapacityThresholdBall(uint32_t capacity, uint32_t threshold) const;
    // std::pair<uint32_t, uint32_t> deconstructCapacityThresholdBall(const std::vector<size_t> &ball) const;

    // 加密转换
    size_t blockToPseudoRandomNumber(const Block &ball, int limit = -1) const;
    size_t keyToPseudoRandomNumber(const size_t&key, int limit = -1) const;

    // 内存操作
    void writeTransposed(LocalRAM ram,vector<Block> blocks,size_t offset,size_t start) const;
    std::vector<Block> readTransposed(LocalRAM &ram, size_t offset, size_t start,
        size_t read_length) const;

    // 复杂内存操作
    std::pair<std::vector<std::vector<size_t>>, std::vector<size_t>>
    readTransposedGetMixedStripeIndexes(const std::vector<size_t> &ram,
                                        size_t offset, size_t start,
                                        size_t read_length,
                                        size_t mixed_start, size_t mixed_end) const;
    std::vector<Block> readTransposedAndShifted( LocalRAM &ram,
                                                               size_t offset, size_t start,
                                                               size_t read_length,
                                                               size_t shift_position) const;
    void obliviousShiftData(std::vector<size_t> &ram,
                            size_t number_of_bins,
                            size_t shift_position) const;

    std::vector<std::vector<size_t>> changeBallsStatus(
        const std::vector<std::vector<size_t>> &balls, size_t status) const;
    std::vector<size_t> changeBallStatus(const std::vector<size_t> &ball, size_t status) const;

    std::unordered_map<size_t,Block> blocksToDictionary(const std::vector<Block> &blocks) const;
    Block constructCapacityThresholdBlock(size_t capacity, int threshold) const;
    std::pair<size_t, int> deconstructCapacityThresholdBall(const Block &block) const;

private:
    const Config &conf;
    std::vector<size_t> key_;

    // 加密辅助方法
    void aesEncrypt(const std::vector<size_t> &plaintext, std::vector<size_t> &ciphertext) const;
    void initializeCipher();
    size_t calculateFieldSize() const;
    void validateKey() const;

    // 内存操作辅助
    size_t calculateBallOffset(size_t index, size_t offset, size_t start) const;
    void validateBallSize(const std::vector<size_t> &ball) const;
};

// 哈希特化
namespace std
{
    template <>
    struct hash<std::vector<size_t>>
    {
        size_t operator()(const vector<size_t> &v) const
        {
            return hash<string>()(string(v.begin(), v.end()));
        }
    };
}
#endif // BYTEOPERATIONS_H