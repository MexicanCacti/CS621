#pragma once
#include "directory_block.hpp"

class RootBlock : public DirectoryBlock{

    public:
        unsigned int const getNextFreeBlock() {return FREE;}
        void setFreeBlock(const unsigned int& blockNum) {FREE = blockNum;}
};