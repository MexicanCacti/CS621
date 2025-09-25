#pragma once
#include "block.hpp"

class FileBlock : public Block {
    private:
        char USER_DATA [USER_DATA_SIZE] = {0};
    public:
        FileBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
};