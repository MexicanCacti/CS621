#pragma once
#include "block.hpp"
#include "../utils/entry.hpp"

class DirectoryBlock : public Block {
    private:
        unsigned int FREE = 0;
        char filler [FILLER_AMOUNT];
        Entry DIR [MAX_DIRECTORY_ENTRIES];

    public:
        DirectoryBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
        ~DirectoryBlock();
        Entry* const getDir() {return DIR;};
        STATUS_CODE addEntry(const char* name, const char& type, const unsigned int& entryIndex, const unsigned int& blockNumber);
        unsigned int findFreeEntry();
        unsigned int const getFreeBlock() {return FREE;}
        void const setFreeBlock(const unsigned int& blockNum) {FREE = blockNum;};

};