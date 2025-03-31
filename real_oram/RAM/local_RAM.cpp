#include "local_RAM.h"
#include <cmath>
#include <cstring>
#include <iostream>

// 全局计数器的定义
int BALL_READ = 0;
int BALL_WRITE = 0;
int RT_READ = 0;
int RT_WRITE = 0;

local_RAM::local_RAM(const std::string& file_path, const config& conf)
    : conf(conf), file_path(file_path) {
    generate_random_memory(conf.DATA_SIZE);
}

void local_RAM::readBall(int location, std::vector<uint8_t>& output) {
    BALL_READ++;
    int ball_index = location / conf.BALL_SIZE;
    output = memory[ball_index];
}

void local_RAM::writeBall(int location, const std::vector<uint8_t>& ball) {
    BALL_WRITE++;
    int ball_index = location / conf.BALL_SIZE;
    memory[ball_index] = ball;
}

void local_RAM::readChunk(int start, int end, std::vector<std::vector<uint8_t>>& output) {
    int balls_num = (end - start) / conf.BALL_SIZE;
    BALL_READ += balls_num;
    int start_index = start / conf.BALL_SIZE;
    for (int i = 0; i < balls_num; i++) {
        output.push_back(memory[start_index + i]);
    }
}

void local_RAM::writeChunk(int start, int end, const std::vector<std::vector<uint8_t>>& balls) {
    int balls_num = (end - start) / conf.BALL_SIZE;
    BALL_WRITE += balls_num;
    int start_index = start / conf.BALL_SIZE;
    for (int i = 0; i < balls_num; i++) {
        memory[start_index + i] = balls[i];
    }
}

void local_RAM::readChunks(const std::vector<std::pair<int, int>>& chunks) {
    RT_READ++;
    std::vector<std::vector<uint8_t>> balls;
    for (const auto& chunk : chunks) {
        std::vector<std::vector<uint8_t>> chunk_balls;
        readChunk(chunk.first, chunk.second, chunk_balls);
        balls.insert(balls.end(), chunk_balls.begin(), chunk_balls.end());
    }
}

void local_RAM::writeChunks(const std::vector<std::pair<int, int>>& chunks, const std::vector<std::vector<uint8_t>>& balls) {
    RT_WRITE++;
    int i = 0;
    for (const auto& chunk : chunks) {
        int start = chunk.first;
        int end = chunk.second;
        int balls_num = std::ceil((end - start) / double(conf.BALL_SIZE));
        writeChunk(start, end, std::vector<std::vector<uint8_t>>(balls.begin() + i, balls.begin() + i + balls_num));
        i += balls_num;
    }
}

void local_RAM::readBalls(const std::vector<int>& locations, std::vector<std::vector<uint8_t>>& output) {
    RT_READ++;
    for (int location : locations) {
        std::vector<uint8_t> ball;
        readBall(location, ball);
        output.push_back(ball);
    }
}

void local_RAM::writeBalls(const std::vector<int>& locations, const std::vector<std::vector<uint8_t>>& balls) {
    RT_WRITE++;
    for (size_t i = 0; i < locations.size(); i++) {
        writeBall(locations[i], balls[i]);
    }
}

int local_RAM::getSize() const {
    return conf.FINAL ? 2 * conf.DATA_SIZE : conf.DATA_SIZE;
}

std::vector<uint8_t> local_RAM::generate_empty_ball_with_key(int key) {
    std::vector<uint8_t> ball(conf.BALL_DATA_SIZE, conf.DUMMY_STATUS);
    ball.push_back(conf.DATA_STATUS);
    ball.insert(ball.end(), reinterpret_cast<uint8_t*>(&key), reinterpret_cast<uint8_t*>(&key) + conf.KEY_SIZE);
    return ball;
}

void local_RAM::generate_random_memory(int number_of_balls) {
    memory.resize(number_of_balls);
    for (int i = 0; i < number_of_balls; i++) {
        memory[i] = generate_empty_ball_with_key(i);
    }
}

void reset_counters() {
    BALL_READ = 0;
    BALL_WRITE = 0;
    RT_READ = 0;
    RT_WRITE = 0;
}