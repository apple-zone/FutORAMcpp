//
//
//

#ifndef PORAM_BLOCK_H
#define PORAM_BLOCK_H

#include <algorithm>

#include <cstdint>
using namespace std;

class Block {
public:
    size_t key;
    int data;
    uint8_t state;

    Block();
    Block(size_t key, int data);
    Block(size_t key, int data, uint8_t state);
    void printBlock();
    virtual ~Block();
};
#endif //PORAM_BLOCK_H
