#pragma once
#include <deque>
#include "../utils/search_result.hpp"

class DiskManager;

class DiskWriter{
    private:
        DiskManager& _diskManager;
    public:
        DiskWriter() = delete;
        DiskWriter(DiskManager& diskManager) : _diskManager(diskManager) {}
        STATUS_CODE const writeToBlock(const unsigned int& blockNumber, const char* data, const int& bytes);
        STATUS_CODE const chainDirectoryBlock(DirectoryBlock* const directory, const unsigned int& newBlockNumber);
        STATUS_CODE const addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type);

};