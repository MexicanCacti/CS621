#pragma once
#include "block.hpp"
#include <queue>
#include <string>
#include <cstring>

struct Entry {
    char TYPE = 'F'; // F for file, D for Directory, U for user data file
    char NAME[MAX_NAME_LENGTH + 1];
    unsigned int LINK = 0;
    unsigned int SIZE = 0;
    Entry() = default;
    Entry(const char* name, const char& type, const unsigned int& link, const unsigned int& size) :
        TYPE(type), LINK(link), SIZE(size) {
            strncpy(NAME, name, MAX_NAME_LENGTH);
            NAME[MAX_NAME_LENGTH] = '\0';
        };
};

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
        unsigned int const getFreeBlock() {return FREE;}
        void const setFreeBlock(const unsigned int& blockNum) {FREE = blockNum;};

};