//
//
//
#include "Block.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

Block::Block(){//dummy index
    this->key = 0;
    this->data = 0;
    this->state = 0;
}

Block::Block(int key, int data) : key(key), data(data),state(1){};
Block::Block(int key, int data, uint8_t state) : key(key), data(data), state(state){};

Block::~Block()
{
    //dtor
}
