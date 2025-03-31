#include "../RAM/local_RAM.h"
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/aes.h> 
#include "config.h"  

class ByteOperations {    
    private:
        config conf;
        int key_length;
        std::string cipher_key;
        std::string empty_value;
    
        std::vector<uint8_t> padKey(const std::vector<uint8_t>& key) const {
            int padding = key_length - (key.size() % key_length);
            if (padding == key_length) { 
                return key;
            }
            std::vector<uint8_t> padded_key = key; 
            padded_key.insert(padded_key.end(), padding, '\x00'); 
            return padded_key;
        }
    
        std::vector<uint8_t> aesEncrypt(const std::vector<uint8_t>& data) const {
            AES_KEY aes_key;
            AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(cipher_key.c_str()), key_length * 8, &aes_key);
            std::vector<uint8_t> encrypted(data.size());
            AES_encrypt(reinterpret_cast<const unsigned char*>(data.data()), reinterpret_cast<unsigned char*>(encrypted.data()), &aes_key);
            return encrypted;
        }
    
        uint64_t bytesToInt(const std::vector<uint8_t>& bytes) const {
            return *reinterpret_cast<const uint64_t*>(bytes.data());
        }
    
        std::vector<uint8_t> intToBytes(int value, int length) const {
            std::vector<uint8_t> result(length);
            std::memcpy(result.data(), &value, length);
            return result;
        }
    public:
        ByteOperations(const std::string& key, const config& conf)
            : conf(conf), key_length(key.size()), cipher_key(key) {
            if (key_length != 16 && key_length != 24 && key_length != 32) {
                throw std::invalid_argument("AES key must be 16, 24, or 32 bytes long");
            }
            empty_value = std::string(conf.BALL_STATUS_POSITION, '\x00');
        }
    
        bool isBitOn(uint64_t number, int bit_num) const {
            return (number & (1ULL << bit_num)) > 0;
        }
    
        int getCapacity(const std::vector<uint8_t>& capacity_ball) const {
            if (std::memcmp(capacity_ball.data(), empty_value.data(), conf.BALL_STATUS_POSITION) != 0 ||
                capacity_ball[conf.BALL_STATUS_POSITION] != conf.DUMMY_STATUS) {
                return 0;
            }
            return static_cast<int>(bytesToInt(capacity_ball));
        }
    
        std::vector<uint8_t> constructCapacityThresholdBall(int capacity, int threshold) const {
            int length = (conf.BALL_SIZE - conf.BALL_STATUS_POSITION) / 2;
            std::vector<uint8_t> capacity_bytes = intToBytes(capacity, length);
            std::vector<uint8_t> threshold_bytes = intToBytes(threshold, length);
            std::vector<uint8_t> result(conf.BALL_SIZE, conf.DUMMY_STATUS);
            std::copy(capacity_bytes.begin(), capacity_bytes.end(), result.begin());
            std::copy(threshold_bytes.begin(), threshold_bytes.end(), result.begin() + length);
            return result;
        }
    
        std::pair<int, int> deconstructCapacityThresholdBall(const std::vector<uint8_t>& ball) const {
            int length = (conf.BALL_SIZE - conf.BALL_STATUS_POSITION) / 2;
            int capacity = bytesToInt(ball);
            int threshold = bytesToInt(std::vector<uint8_t>(ball.begin() + length, ball.end()));
            return {capacity, threshold};
        }
    
        int ballToPseudoRandomNumber(const std::vector<uint8_t>& ball, int limit = -1) const {
            std::vector<uint8_t> ball_key(ball.begin() + conf.BALL_STATUS_POSITION + 1, ball.end());
            return keyToPseudoRandomNumber(ball_key, limit);
        }
    
        int keyToPseudoRandomNumber(const std::vector<uint8_t>& key, int limit = -1) const {
            std::vector<uint8_t> padded_key = padKey(key);
            std::vector<uint8_t> encrypted = aesEncrypt(padded_key);
            uint64_t result = bytesToInt(encrypted);
            return limit == -1 ? result : result % limit;
        }
    
        void writeTransposed(local_RAM& ram, const std::vector<std::vector<uint8_t>>& balls, int offset, int start) const {
            std::vector<std::pair<int, int>> chunks;
            for (size_t i = 0; i < balls.size(); ++i) {
                chunks.push_back({start + i * offset * conf.BALL_SIZE, start + i * offset * conf.BALL_SIZE + conf.BALL_SIZE});
            }
            ram.writeChunks(chunks, balls);
        }
    
        std::vector<std::vector<uint8_t>> readTransposed(local_RAM& ram, int offset, int start, int readLength) const {
            std::vector<std::pair<int, int>> chunks;
            for (int i = 0; i < readLength; ++i) {
                chunks.push_back({start + i * offset * conf.BALL_SIZE, start + i * offset * conf.BALL_SIZE + conf.BALL_SIZE});
            }
            return ram.readChunks(chunks);
        }
    
        std::vector<std::vector<uint8_t>> changeBallsStatus(const std::vector<std::vector<uint8_t>>& balls, uint8_t status) const {
            std::vector<std::vector<uint8_t>> result;
            for (const auto& ball : balls) {
                result.push_back(changeBallStatus(ball, status));
            }
            return result;
        }
    
        std::vector<uint8_t> changeBallStatus(const std::vector<uint8_t>& ball, uint8_t status) const {
            std::vector<uint8_t> result = ball;
            result[conf.BALL_STATUS_POSITION] = status;
            return result;
        }
    
        std::unordered_map<std::string, std::vector<uint8_t>> ballsToDictionary(const std::vector<std::vector<uint8_t>>& balls) const {
            std::unordered_map<std::string, std::vector<uint8_t>> result;
            for (const auto& ball : balls) {
                std::string key(ball.begin() + conf.BALL_STATUS_POSITION + 1, ball.end());
                result[key] = ball;
            }
            return result;
        }
    

    };