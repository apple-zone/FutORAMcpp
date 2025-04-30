cmake_minimum_required(VERSION 3.14)

project(FutORAMcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 查找 OpenSSL
find_package(OpenSSL REQUIRED)

# 包含目录：用于让 #include "Oram.h" 等能正确找到
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# 收集所有 .cpp 源文件（在 src 目录中）
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# 添加可执行程序
add_executable(FutORAMcpp ${SOURCES})

# 链接 OpenSSL 库
target_link_libraries(FutORAMcpp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
