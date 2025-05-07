#include "ByteOperations.h"
#include <algorithm>
#include <stdexcept>
#include "Block.h"

ByteOperations::ByteOperations()
    : conf(conf), key_()
{
    // Initialize the AES key using the provided configuration
    // AES_set_encrypt_key(key_.data(), 128, &aes_key_);
}

// ByteOperations::ByteOperations( const Config &config)
//     : conf(config)
// {

// }

// string ByteOperations::getRandomString(size_t key)
// {

// }

bool ByteOperations::isBitOn(uint64_t number, size_t bit_num) const
{
    return (number & (1ULL << bit_num)) != 0;
}

size_t ByteOperations::getCapacity(Block &capacity_ball)
{
    return capacity_ball.key;
}

// Block ByteOperations::constructCapacityThresholdBall(
//     size_t capacity, size_t threshold)
// {
//     Block new_block = Block(capacity, threshold);
//     return new_block;
// }

// std::pair<size_t, size_t> ByteOperations::deconstructCapacityThresholdBall(const Block block)
// {
//     return std::pair<size_t, size_t>(block.data, block.key);
// }

size_t ByteOperations::blockToPseudoRandomNumber(
    const Block &ball, int limit) const
{
    size_t ballkey = ball.key;
    return keyToPseudoRandomNumber(ballkey, limit);
}

size_t AES_EncryptSizeT(size_t input,
                        const unsigned char *key,
                        int limit)
{
    // 初始化AES密钥结构体
    AES_KEY aes_key;
    if (AES_set_encrypt_key(key, 128, &aes_key) < 0)
    {
        throw std::runtime_error("AES key setup failed");
    }

    // 转换size_t为字节数组
    unsigned char plaintext[16] = {0};        // 初始化为全0
    memcpy(plaintext, &input, sizeof(input)); // 复制原始数据

    // 执行AES-128-ECB加密
    unsigned char ciphertext[16];
    AES_ecb_encrypt(plaintext, ciphertext, &aes_key, AES_ENCRYPT);

    // 转换加密结果回数值
    size_t result = 0;
    memcpy(&result, ciphertext, sizeof(result));

    // 应用范围限制
    if (limit > 0)
    {
        result %= static_cast<size_t>(limit);
    }

    return result;
}

size_t ByteOperations::keyToPseudoRandomNumber(const size_t &key, int limit) const
{
    // AES密钥
    unsigned char aes_key[16] = {0}; // 128位密钥

    // 将key转换为字节数组
    memcpy(aes_key, &key, sizeof(key)); // 复制原始数据

    return AES_EncryptSizeT(key, aes_key, limit);
};

void ByteOperations::writeTransposed(LocalRAM ram, vector<Block> blocks, size_t offset, size_t start) const
{
    std::vector<size_t> start_positions, end_positions;
    for (int i = 0; i < blocks.size(); ++i)
    {
        start_positions.push_back(start + i * offset);
        end_positions.push_back(start + i * offset + 1);
    }
    ram.writeChunks(start_positions.data(), end_positions.data(), blocks, blocks.size());
}

std::vector<Block> ByteOperations::readTransposed(
    LocalRAM &ram, size_t offset, size_t start,
    size_t read_length) const
{
    vector<size_t> start_pos, end_pos;
    for (int i = 0; i < read_length; ++i)
    {
        start_pos.push_back((start + i * offset * conf.BLOCK_SIZE));
        end_pos.push_back(start + i * offset * conf.BLOCK_SIZE + conf.BLOCK_SIZE);
    }
    return ram.readChunks(start_pos.data(), end_pos.data(), read_length);
}

std::vector<Block> ByteOperations::readTransposedAndShifted(
    LocalRAM &ram, size_t offset, size_t start,
    size_t read_length, size_t shift_position) const
{
    std::pair<std::vector<size_t>, std::vector<size_t>> chunks;
    size_t shift = 0;
    for (int i = 0; i < read_length; i++)
    {
        if (start + i * offset < shift_position)
            ++shift;
        chunks.first.push_back(start + i * offset + shift_position);
        chunks.second.push_back(start + i * offset + shift_position + 1);
    }
    std::vector<Block> result = ram.readChunks(chunks.first.data(), chunks.second.data(), read_length);
    shift %= result.size();
    std::rotate(result.begin(), result.begin() + shift, result.end());
    return result;
}

// 辅助方法实现
size_t ByteOperations::calculateBallOffset(size_t index, size_t offset, size_t start) const
{
    return start + index * offset * conf.BLOCK_SIZE;
}

void ByteOperations::validateBallSize(const std::vector<size_t> &ball) const
{
    if (ball.size() != conf.BLOCK_SIZE)
    {
        throw std::invalid_argument("Invalid ball size: " +
                                    std::to_string(ball.size()) +
                                    " expected: " +
                                    std::to_string(conf.BLOCK_SIZE));
    }
}

size_t ByteOperations::calculateFieldSize() const
{
    return (conf.BLOCK_SIZE - conf.BLOCK_STATUS_POSITION) / 2;
}

std::unordered_map<size_t, Block> ByteOperations::blocksToDictionary(const std::vector<Block> &balls) const
{
    std::unordered_map<size_t, Block> dictionary;
    for (const auto &ball : balls)
    {
        size_t key = {ball.key};
        dictionary[key] = ball;
    }
    return dictionary;
}

Block ByteOperations::constructCapacityThresholdBlock(size_t capacity, int threshold) const
{
    Block new_block = Block(capacity, threshold);
    return new_block;
}

std::pair<size_t, int> ByteOperations::deconstructCapacityThresholdBall(const Block &block) const
{
    return std::pair<size_t, int>(block.key, block.data);
}