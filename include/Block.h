//
//
//

#ifndef PORAM_BLOCK_H
#define PORAM_BLOCK_H

#include <algorithm>
using namespace std;

class Block {
public:
    int key;
    int data;
    uint8_t state;

    Block();
    Block(int key, int data);
    Block(int key, int data, uint8_t state);
    void printBlock();
    virtual ~Block();
};
#endif //PORAM_BLOCK_H
