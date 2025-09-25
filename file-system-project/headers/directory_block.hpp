#pragma once
#include "block.hpp"
#include "disk_manager.hpp"
#include <queue>
#include <string>

struct Entry {
    char TYPE = 'F'; // F for file, D for Directory, U for user data file
    char NAME[MAX_NAME_LENGTH + 1] = {0};
    unsigned int LINK = 0;
    unsigned int SIZE = 0;
};

class DirectoryBlock : public Block {
    private:
        Entry DIR [MAX_DIRECTORY_ENTRIES] = {Entry()};

    public:
        DirectoryBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
        ~DirectoryBlock();
        Entry* const findFile(std::deque<std::string>& nameBuffer, DiskManager& diskManager);
};