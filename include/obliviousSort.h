#ifndef FUTORAMCPP_OBLIVIOUSSORT_H
#define FUTORAMCPP_OBLIVIOUSSORT_H

#include "config.h"
#include "ByteOperations.h"
#include "Block.h"
#include "localRam.h"

class ObliviousSort {
    public:
        ObliviousSort(const Config& config, ByteOperations& byte_ops)
            : conf(config), byte_ops(byte_ops) {}
    
        std::pair<std::vector<Block>, std::vector<Block>> splitToBinsByBit(std::vector<Block>& blocks, size_t bit_num, size_t number_of_bins) {
            std::vector<Block> bin_0, bin_1;
            for (const auto& block : blocks) {
                if(block.state == 0) {
                    continue; // 跳过虚拟块
                }
                size_t addr = byte_ops.keyToPseudoRandomNumber(block.key, number_of_bins);
                if (byte_ops.isBitOn(block.key, bit_num)) {
                    bin_1.push_back(block);
                } else {
                    bin_0.push_back(block);
                }
            }
            // 向bin_0和bin_1补入虚拟块
            size_t bin_0_size = bin_0.size();
            size_t bin_1_size = bin_1.size();
            for(size_t i = bin_0_size; i < conf.BIN_SIZE; ++i) {
                bin_0.push_back(Block(-1, -1, false)); // 添加虚拟块
            }
            for(size_t i = bin_1_size; i < conf.BIN_SIZE; ++i) {
                bin_1.push_back(Block(-1, -1, false)); // 添加虚拟块
            }
            return {bin_0, bin_1};
        }

        std:pair<vector<Block>,vector<Block>> splitToBinsByBitExtract(vector<Block>& blocks, size_t bit_num, size_t number_of_bins,float epsilon) {
            vector<Block> bin_0, bin_1;
            for(auto block : blocks) {
                if(block.state == 0) {
                    continue; // 跳过虚拟块
                }
                size_t addr = byte_ops.blockToPseudoRandomNumber(block, number_of_bins);
                if (byte_ops.isBitOn(block.key, bit_num)) {
                    bin_1.push_back(block);
                } else {
                    bin_0.push_back(block);
                }
            }
            // 向bin_0和bin_1补入虚拟块
            size_t bin_0_size = bin_0.size();
            size_t bin_1_size = bin_1.size();
            for(size_t i = bin_0_size; i < conf.BIN_SIZE; ++i) {
                bin_0.push_back(Block(-1, -1, false)); // 添加虚拟块
            }
            for(size_t i = bin_1_size; i < conf.BIN_SIZE; ++i) {
                bin_1.push_back(Block(-1, -1, false)); // 添加虚拟块
            }
            return {bin_0, bin_1};
        }
}

#endif //FUTORAMCPP_OBLIVIOUSSORT_H
