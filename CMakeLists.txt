cmake_minimum_required(VERSION 3.14)

project(FutORAMcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 查找 OpenSSL
find_package(OpenSSL REQUIRED)

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# 手动列出所有源文件，避免引入无关或重复的 .cpp（建议）
set(SOURCES
    src/main.cpp
    src/Block.cpp
    src/ByteOperations.cpp
    src/Hashtable.cpp
    src/ORAM.cpp
    src/LocalRAM.cpp
    src/tests.cpp
    src/Config.cpp
    # 不加 tests.cpp
)

# 添加可执行程序
add_executable(FutORAMcpp ${SOURCES})
# 无论何种构建类型，统一输出到 build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
# 链接 OpenSSL 库
target_link_libraries(FutORAMcpp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
