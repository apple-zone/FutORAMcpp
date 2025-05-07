#ifndef TESTS_H
#define TESTS_H

#include "Oram.h"
#include "localRam.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>  // 添加 random 头文件

// 进度条函数声明
void progress_bar(int total, int current);

// ORAM 测试函数声明
void _real_oram_test(int oram_size);
void real_oram_test();

#endif // TESTS_H