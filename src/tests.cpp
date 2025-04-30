#include "Oram.h"
#include "localRam.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

void progress_bar(int total, int current) {
    float progress = (current + 1) * 100.0f / total;
    int bar_width = 50;
    
    std::cout << "\r["; // 回车符回到行首
    int pos = bar_width * progress / 100;
    for (int i = 0; i < bar_width; ++i) {
        std::cout << (i <= pos ? "#" : " ");
    }
    std::cout << "] " << std::ceil(progress) << "%" << std::flush;
}



void _real_oram_test(int oram_size)
{
    ORAM oram(oram_size,"test_data.txt");
    oram.cleanWriteMemory();
    //原注释：分配存储不应该被计数为写操作
    LocalRAM::reset_counters();
    oram.initialBuild("test_data.txt");
    for(int i = 0; i < oram_size; ++i)
    {
        oram.access("write", Block(size_t(i), size_t(i+3)));
        if(i % 10000 == 0)
        {
            progress_bar(oram_size, i);
        }
    }
}

void real_oram_test() {
    int number_of_MB = std::stoi("How many MB of storage should the test allocate?\n");


    int number_of_blocks = (number_of_MB * (1 << 20)) / sizeof(Block);

    std::cout << "Executing " << number_of_blocks << " accesses (the size of the ORAM as every block contains 16 bytes of data)\n";
    if (number_of_MB > 50) {
        std::cout << "Due to the initial build it might take several minutes before accesses begin.\n";
    }

    if (number_of_MB > 1000) {
        std::cout << "Due to the initial build it might take several hours before accesses begin.\n";
    }

    // 调用实际的 ORAM 测试函数
    _real_oram_test(number_of_blocks);

    std::cout << "\naccesses: " << number_of_blocks << ":\n";

    int Blocks_read = LocalRAM::BLOCK_READ+LocalRAM::BLOCK_WRITE;

    std::cout << "Blocks-read: " << Blocks_read << "\n";

    std::cout << "Average blocks read per-access: " << (Blocks_read / number_of_blocks) << " Blocks\n";

    // 计算平均每次访问的 KB
    std::cout << "Average KB per-access: " 
              << (static_cast<int>((10 * 32 * Blocks_read / number_of_blocks) / 1024.0) / 10.0) 
              << " KB\n";

    // 计算平均每次访问的往返次数
    std::cout << "Average round-trips per-access: " 
              << ((LocalRAM::RT_READ + LocalRAM::RT_WRITE) / (2 * number_of_blocks)) 
              << "\n";
}



