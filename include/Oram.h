#ifndef ORAM_H
#define ORAM_H

#include "Config.h"
#include "localRam.h"
#include "Hashtable.h"
#include "CuckooHash.h"
#include "Block.h"
#include "ByteOperations.h"
#include <vector>
#include <unordered_map>
#include <random>

class ORAM {
public:
    ORAM(int number_of_blocks,string data_location);
    void cleanWriteMemory();
    void initialBuild(const std::string& data_location);
    Block access(const std::string& op, const Block& block);
    void rebuild();

private:
    void extractLevelOne();
    void tightCompactionLevelOne();
    void intersperseStashAndLevelOne();
    void rebuildLevelOne();

    size_t not_found;
    Config conf;
    CuckooHash cuckoo;
    std::unordered_map<size_t, Block> local_stash; // 关键修改点
    size_t stash_reals_count;
    size_t read_count;
    std::vector<HashTable> tables;
    LocalRAM original_data_ram;
    ByteOperations byte_ops;
    std::mt19937_64 rng; // 使用64位随机引擎
};
#endif // ORAM_H