#ifndef LOCAL_RAM_H
#define LOCAL_RAM_H

#include <vector>
#include <cstdint>
#include <string>
#include <utility>
#include "../real_oram/config.h"

// 全局计数器
extern int BALL_READ;
extern int BALL_WRITE;
extern int RT_READ;
extern int RT_WRITE;

class local_RAM {
public:
    local_RAM(const std::string& file_path, const config& conf);
    void readBall(int location, std::vector<uint8_t>& output);
    void writeBall(int location, const std::vector<uint8_t>& ball);
    void readChunk(int start, int end, std::vector<std::vector<uint8_t>>& output);
    void writeChunk(int start, int end, const std::vector<std::vector<uint8_t>>& balls);
    void readChunks(const std::vector<std::pair<int, int>>& chunks, std::vector<std::vector<uint8_t>>& output);
    void writeChunks(const std::vector<std::pair<int, int>>& chunks, const std::vector<std::vector<uint8_t>>& balls);
    void readBalls(const std::vector<int>& locations, std::vector<std::vector<uint8_t>>& output);
    void writeBalls(const std::vector<int>& locations, const std::vector<std::vector<uint8_t>>& balls);
    int getSize() const;

private:
    config conf;
    std::string file_path;
    std::vector<std::vector<uint8_t>> memory;

    std::vector<uint8_t> generate_empty_ball_with_key(int key);
    void generate_random_memory(int number_of_balls);
};

void reset_counters();

#endif // LOCAL_RAM_H