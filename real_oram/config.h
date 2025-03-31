#ifndef CONFIG_H
#define CONFIG_H
 
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <sstream>

// 随机字符串生成函数
std::string get_random_string(int length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

    std::string result(length, 0);
    for (int i = 0; i < length; ++i) {
        result[i] = charset[distribution(generator)];
    }
    return result;
}

// 基础配置类
struct baseConfig {
    static int N;
};

int baseConfig::N = 1 << 20; // 2^20

// 配置类
class config : public baseConfig {
public:
    static constexpr int KEY_SIZE = 16;
    static constexpr int BALL_DATA_SIZE = 16;
    static constexpr int BALL_STATUS_POSITION = BALL_DATA_SIZE;
    static constexpr int BALL_SIZE = BALL_DATA_SIZE + 1 + KEY_SIZE;
    static constexpr int LOG_LAMBDA = 10;
    static constexpr int Z = 131220;
    static constexpr int MU = Z / 2;
    static constexpr double EPSILON = 1.0 / LOG_LAMBDA;
    static constexpr int STASH_SIZE = 2 * LOG_LAMBDA;
    static constexpr bool FINAL = false;

    int NUMBER_OF_BINS;
    int DATA_SIZE;
    int OVERFLOW_SIZE;
    int LOCAL_MEMORY_SIZE;
    int NUMBER_OF_BINS_IN_OVERFLOW;
    int RAND_CYCLIC_SHIFT_ITERATION = 7;
    double CUCKOO_HASH_FILLAGE = 1.1;

    std::string DATA_LOCATION;
    std::string BINS_LOCATION;
    std::string OVERFLOW_LOCATION;
    std::string OVERFLOW_SECOND_LOCATION;
    std::string MIXED_STRIPE_LOCATION;
    std::string RAND_CYCLIC_SHIFT_LOCATION;

    std::string MAIN_KEY;
    std::string CUCKOO_HASH_KEY_1;
    std::string CUCKOO_HASH_KEY_2;

    static constexpr uint8_t DUMMY_STATUS = 0x00;
    static constexpr uint8_t DATA_STATUS = 0x01;
    static constexpr uint8_t SECOND_DUMMY_STATUS = 0x02;

    config(int N = -1) {
        if (N != -1) {
            this->N = N;
        }
        reset();
    }

    void reset() {
        NUMBER_OF_BINS = std::ceil(static_cast<double>(N) / MU);
        DATA_SIZE = N * BALL_SIZE;
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

#endif // CONFIG_H