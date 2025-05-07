#include "Config.h"

// 配置类构造函数实现
Config::Config(size_t N) {
    if(N != 0) {
        this->N = N;
    }
    reset();
    generate_keys();
}

// 重置函数实现
void Config::reset() {
    MU = Z / 2;
    NUMBER_OF_BINS = static_cast<size_t>(std::ceil(static_cast<float>(N) / MU));
    BIN_SIZE = 2 * MU;
    BIN_SIZE_IN_BYTES = BIN_SIZE * BLOCK_SIZE;
    EPSILON = 1.0f / LOG_LAMBDA;
    STASH_SIZE = 2 * LOG_LAMBDA;
    OVERFLOW_SIZE = static_cast<size_t>(std::ceil(N * EPSILON));
    NUMBER_OF_BINS_IN_OVERFLOW = static_cast<size_t>(std::pow(2, std::ceil(std::log2(std::ceil(EPSILON * N / MU)))));
    FINAL = false;

    update_paths();
}

// 密钥生成函数实现
void Config::generate_keys() {
    MAIN_KEY = generate_random_bytes(16);
    CUCKOO_HASH_KEY_1 = generate_random_bytes(16);
    CUCKOO_HASH_KEY_2 = generate_random_bytes(16);
}

// 路径更新函数实现
void Config::update_paths() {
    DATA_LOCATION = std::to_string(NUMBER_OF_BINS) + "/data";
    BINS_LOCATION = std::to_string(NUMBER_OF_BINS) + "/bins";
    OVERFLOW_LOCATION = std::to_string(NUMBER_OF_BINS) + "/overflow";
    OVERFLOW_SECOND_LOCATION = std::to_string(NUMBER_OF_BINS) + "/second_overflow";
    MIXED_STRIPE_LOCATION = std::to_string(NUMBER_OF_BINS) + "/mixed_stripe";
}

// 随机字节生成函数实现
std::vector<uint8_t> Config::generate_random_bytes(size_t length) {
    std::vector<uint8_t> bytes(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    for (auto& byte : bytes) {
        byte = static_cast<uint8_t>(distrib(gen));
    }
    return bytes;
}