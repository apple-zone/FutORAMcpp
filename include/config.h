#ifndef FUTORAMCPP_CONFIG_H
#define FUTORAMCPP_CONFIG_H

#include <cmath>
#include <string>
#include <random>
#include <vector>

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
    size_t MU;
    size_t NUMBER_OF_BINS;
    size_t BIN_SIZE;
    size_t BIN_SIZE_IN_BYTES;
    float EPSILON;
    size_t STASH_SIZE;

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

    size_t OVERFLOW_SIZE;
    size_t NUMBER_OF_BINS_IN_OVERFLOW;

    bool FINAL;

    explicit Config(size_t N); // 构造函数声明
    void reset();      // 重置函数声明
    void generate_keys(); // 密钥生成函数声明
    void update_paths();  // 路径更新函数声明

private:
    static std::vector<uint8_t> generate_random_bytes(size_t length); // 随机字节生成函数声明
};

#endif //FUTORAMCPP_CONFIG_H