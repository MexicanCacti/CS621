#pragma once
#include "block.hpp"
class UserDataBlock : public Block {
    private:
        char USER_DATA [USER_DATA_SIZE] = {0};
    public:
        UserDataBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
};