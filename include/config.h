//
// Created by 21885 on 2025/3/31.
//

#ifndef FUTORAMCPP_CONFIG_H
#define FUTORAMCPP_CONFIG_H

#include <cmath>
#include <string>
#include <random>
#include <vector>
#include <format>

class Config {
public:
    // 基础配置参数
    size_t N = 1 << 20;

    // 固定常量
    static constexpr size_t KEY_SIZE = 16;
    static constexpr size_t BLOCK_DATA_SIZE = 16; // 必须为偶数
    static constexpr size_t LOG_LAMBDA = 10;
    static constexpr size_t Z = 131220;
    static constexpr size_t RAND_CYCLIC_SHIFT_ITERATION = 7;
    static constexpr float CUCKOO_HASH_FILLAGE = 1.1f;

    // 计算参数
    size_t MU = Z / 2;
    size_t NUMBER_OF_BINS;
    size_t BIN_SIZE;
    size_t BIN_SIZE_IN_BYTES;
    float EPSILON = 1.0f / LOG_LAMBDA;
    size_t STASH_SIZE = 2 * LOG_LAMBDA;

    // 存储路径
    std::string DATA_LOCATION;
    std::string BINS_LOCATION;
    std::string OVERFLOW_LOCATION;
    std::string OVERFLOW_SECOND_LOCATION;
    std::string MIXED_STRIPE_LOCATION;
    std::string RAND_CYCLIC_SHIFT_LOCATION;

    // 加密密钥
    std::vector<uint8_t> MAIN_KEY;
    std::vector<uint8_t> CUCKOO_HASH_KEY_1;
    std::vector<uint8_t> CUCKOO_HASH_KEY_2;

    // 状态标识
    static constexpr uint8_t DUMMY_STATUS = 0x00;
    static constexpr uint8_t DATA_STATUS = 0x01;
    static constexpr uint8_t SECOND_DUMMY_STATUS = 0x02;
    
    static constexpr size_t BLOCK_STATUS_POSITION = BLOCK_DATA_SIZE;
    static constexpr size_t BLOCK_SIZE = BLOCK_DATA_SIZE + 1 + KEY_SIZE;
    static constexpr size_t DATA_SIZE = N * BLOCK_SIZE;
    size_t OVERFLOW_SIZE = static_cast<size_t>(std::ceil(DATA_SIZE * EPSILON));
    size_t NUMBER_OF_BINS_IN_OVERFLOW =
            static_cast<size_t>(std::pow(2, std::ceil(std::log2(std::ceil(EPSILON * N / MU)))));

    bool FINAL = false;

    explicit Config(size_t n = (1 << 20)) : N(n) {
        reset();
        generate_keys();
    }

    void reset() {
        MU = Z / 2;
        NUMBER_OF_BINS = static_cast<size_t>(std::ceil(static_cast<float>(N) / MU));
        BIN_SIZE = 2 * MU;
        BIN_SIZE_IN_BYTES = BIN_SIZE * BLOCK_SIZE;

        update_paths();
    }

private:
    // 密钥生成
    void generate_keys() {
        MAIN_KEY = generate_random_bytes(16);
        CUCKOO_HASH_KEY_1 = generate_random_bytes(16);
        CUCKOO_HASH_KEY_2 = generate_random_bytes(16);
    }

    // 路径生成
    void update_paths() {
        DATA_LOCATION = std::format("{}/data", NUMBER_OF_BINS);
        BINS_LOCATION = std::format("{}/bins", NUMBER_OF_BINS);
        OVERFLOW_LOCATION = std::format("{}/overflow", NUMBER_OF_BINS);
        OVERFLOW_SECOND_LOCATION = std::format("{}/second_overflow", NUMBER_OF_BINS);
        MIXED_STRIPE_LOCATION = std::format("{}/mixed_stripe", NUMBER_OF_BINS);
    }

    // 随机字节生成
    static std::vector<uint8_t> generate_random_bytes(size_t length) {
        std::vector<uint8_t> bytes(length);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);

        for (auto& byte : bytes) {
            byte = static_cast<uint8_t>(distrib(gen));
        }
        return bytes;
    }


};


#endif //FUTORAMCPP_CONFIG_H
