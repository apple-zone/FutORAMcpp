#ifndef THRESHOLDGENERATOR_H
#define THRESHOLDGENERATOR_H
#include "Config.h"
#include <random>
#include <stdexcept>


class ThresholdGenerator {
private:
    Config conf_;
    int b_;
    int nPrime_;
    std::mt19937 rng_;  // Mersenne Twister引擎

public:
    explicit ThresholdGenerator(const Config& conf) 
        : conf_(conf), rng_(std::random_device{}()) {
        reset();
    }


    void reset() {
        b_ = conf_.NUMBER_OF_BINS;
        nPrime_ = conf_.N - static_cast<int>(conf_.N * conf_.EPSILON);
        
        // 参数校验
        if (b_ <= 0 || nPrime_ < 0) {
            throw std::invalid_argument("Invalid configuration parameters");
        }
    }

    // 生成阈值
    int generate() {
        if (b_ <= 0 || nPrime_ < 0) {
            throw std::logic_error("Generator not properly initialized");
        }

        // 二项分布参数
        const double p = 1.0 / b_;
        std::binomial_distribution<int> dist(nPrime_, p);
        
        int sample = dist(rng_);
        
        // 更新状态
        --b_;
        nPrime_ -= sample;
        
        return sample;
    }

    // 重新生成阈值
    int regenerate(int prevThreshold) {
        // 恢复状态
        ++b_;
        nPrime_ += prevThreshold;
        
        // 参数校验
        if (b_ > conf_.NUMBER_OF_BINS || nPrime_ > conf_.N) {
            throw std::overflow_error("State recovery overflow");
        }
        
        return generate();
    }


    void seed(unsigned int seed_value) {
        rng_.seed(seed_value);
    }
};



#endif //THRESHOLDGENERATOR_H