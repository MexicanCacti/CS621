#pragma once
#include "block.hpp"
#include <queue>
#include <string>
#include <cstring>
#include "../headers/disk_manager.hpp"
#include "../headers/root_block.hpp"
#include <cstring>

struct Entry {
    char TYPE = 'F'; // F for file, D for Directory, U for user data file
    char NAME[MAX_NAME_LENGTH + 1] = {0};
    unsigned int LINK = 0;
    unsigned int SIZE = 0;
    Entry(const char& type, const char* name, unsigned int link) : TYPE(type), LINK(link) {
        strncpy(NAME, name, MAX_NAME_LENGTH);
        NAME[MAX_NAME_LENGTH] = '\0';
    };
    Entry() : TYPE('F'), NAME{0}, LINK(0), SIZE(0) {};
};

class DirectoryBlock : public Block {
    private:
        Entry DIR [MAX_DIRECTORY_ENTRIES] = {Entry()};

    public:
        DirectoryBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
        ~DirectoryBlock();
        Entry* const getEntry(unsigned int& index);
        std::pair<DirectoryBlock*, unsigned int> const findFile(std::deque<std::string>& nameBuffer, DiskManager& diskManager, const char& type);
        STATUS_CODE addEntry(const char* name, RootBlock* rootDirectory, const char& type, DiskManager& diskManager);
};