//
// Created by 21885 on 2025/3/30.
//

#ifndef FUTORAMCPP_CONFIG_H
#define FUTORAMCPP_CONFIG_H

#include <cmath>

std::string get_random_string(size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(rng)];
    }
    return result;
}

class Config {
public:
    static const int N = 2<<20;
    static const int KEY_SIZE = 16; // 密钥大小
    static const int BLOCK_DATA_SIZE = 16; // 每个块的数据大小
    static const int BLOCK_SIZE = BLOCK_DATA_SIZE + 1 + KEY_SIZE; // 每个块的大小
    static const int LOG_LAMADA = 2**20; // 块的数量
    static const int Z = 131200;
    static constexpr int MU = Z / 2;

    static int NUMBER_OF_BINS = ceil(N/MU); // 哈希表的桶数量
    static const int BIN_SIZE = 2*MU; // 每个桶的大小
    static const int BIN_SIZE_IN_BYTES = BIN_SIZE * BLOCK_SIZE; // 每个桶的字节大小
    static const float EPSILON = 1.0 / LOG_LAMBDA; // 误差范围
    static const int STASH_SIZE = 2 * LOG_LAMBDA; // 暂存区大小

    static constexpr bool FINAL = false;

    static int DATA_SIZE = N * BLOCK_SIZE;
    static const int OVERFLOW_SIZE = std::ceil(DATA_SIZE*EPSILON);
    static const int LOCAL_MEMORY_SIZE = BIN_SIZE_IN_BYTES;
    static const int NUMBER_OF_BINS_IN_OVERFLOW = 2**std::ceil(std::log(std::ceil(EPSILON*N/MU),2));
    static const int RAND_CYCLIC_SHIFT_ITERATION = 7;
    static const float CUCKOO_HASH_FILLAGE = 1.1;

    std::string DATA_LOCATION = "data";
    std::string BINS_LOCATION = "bins";
    std::string OVERFLOW_LOCATION = "overflow";
    std::string OVERFLOW_SECOND_LOCATION = "second_overflow";
    std::string MIXED_STRIPE_LOCATION = "mixed_stripe";
    std::string RAND_CYCLIC_SHIFT_LOCATION = "rand_cyclic_shift";

    std::string MAIN_KEY = "Sixteen byte key";
    std::string CUCKOO_HASH_KEY_1 = "Cuckoo hash key1";
    std::string CUCKOO_HASH_KEY_2 = "Cuckoo hash key2";

    static const int MAIN_BINS = 1024; // 主存储桶数量
    static const int OVERFLOW_BINS = 256; // 溢出桶数量
    static const int BUCKET_CAPACITY = 4; // 每个桶的容量
    static const int MAX_STASH_SIZE = 100; // 最大暂存区大小

    static constexpr uint8_t DUMMY_STATUS = 0x00;
    static constexpr uint8_t DATA_STATUS = 0x01;
    static constexpr uint8_t SECOND_DUMMY_STATUS = 0x02;

    Config(int N = -1) {
        if (N != -1) {
            this->N = N;
        }
        reset();
    }

    void reset() {
        NUMBER_OF_BINS = std::ceil(static_cast<double>(N) / MU);
        DATA_SIZE = N * BLOCK_SIZE;
        OVERFLOW_SIZE = std::ceil(DATA_SIZE * EPSILON);
        NUMBER_OF_BINS_IN_OVERFLOW = 1 << static_cast<int>(std::ceil(std::log2(std::ceil(EPSILON * N / MU))));
        if (NUMBER_OF_BINS_IN_OVERFLOW * MU <= CUCKOO_HASH_FILLAGE * EPSILON * N) {
            NUMBER_OF_BINS_IN_OVERFLOW *= 2;
        }

        MAIN_KEY = get_random_string(16);
        CUCKOO_HASH_KEY_1 = get_random_string(16);
        CUCKOO_HASH_KEY_2 = get_random_string(16);

        std::ostringstream oss;
        oss << NUMBER_OF_BINS;
        DATA_LOCATION = oss.str() + "/data";
        BINS_LOCATION = oss.str() + "/bins";
        OVERFLOW_LOCATION = oss.str() + "/overflow";
        OVERFLOW_SECOND_LOCATION = oss.str() + "/second_overflow";
        MIXED_STRIPE_LOCATION = oss.str() + "/mixed_stripe";
    }
};


#endif //FUTORAMCPP_CONFIG_H
