#include "tests.h"  // 包含自己的头文件

// 进度条实现
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

// ORAM 测试实现
void _real_oram_test(int oram_size) {
    ORAM oram(oram_size, "test_data.txt");
    oram.cleanWriteMemory();
    LocalRAM::reset_counters();
    oram.initialBuild("test_data.txt");
    
    for (int i = 0; i < oram_size; ++i) {
        oram.access("write", Block(size_t(i), size_t(i+3)));
        if (i % 10000 == 0) {
            progress_bar(oram_size, i);
        }
    }
}

// 主测试函数实现
void real_oram_test() {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<size_t> dist(0, 1ULL << 8);
    
    int number_of_MB ; // 随机生成 1 到 1000 MB
    std::cout << "Enter the number of MB (1-1000): ";
    std::cin >> number_of_MB;
    int number_of_blocks = (number_of_MB * (1 << 20)) / sizeof(Block);

    std::cout << "Executing " << number_of_blocks << " accesses...\n";
    
    if (number_of_MB > 50) {
        std::cout << "Due to the initial build it might take several minutes before accesses begin.\n";
    }
    if (number_of_MB > 1000) {
        std::cout << "Due to the initial build it might take several hours before accesses begin.\n";
    }
    // system("pause"); // 暂停程序，等待用户输入
    _real_oram_test(number_of_blocks);
    system("pause"); // 暂停程序，等待用户输入

    std::cout << "\naccesses: " << number_of_blocks << ":\n";
    int Blocks_read = LocalRAM::BLOCK_READ + LocalRAM::BLOCK_WRITE;
    std::cout << "Blocks-read: " << Blocks_read << "\n";
    std::cout << "Average blocks read per-access: " << (Blocks_read / number_of_blocks) << " Blocks\n";
    std::cout << "Average KB per-access: " 
              << (static_cast<int>((10 * 32 * Blocks_read / number_of_blocks) / 1024.0) / 10.0) 
              << " KB\n";
    std::cout << "Average round-trips per-access: " 
              << ((LocalRAM::RT_READ + LocalRAM::RT_WRITE) / (2 * number_of_blocks)) 
              << "\n";
    system("pause"); // 暂停程序，等待用户输入
}