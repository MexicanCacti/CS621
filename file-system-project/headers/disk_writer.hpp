#pragma once
#include <deque>
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"

class DiskManager;

class DiskWriter{
    private:
        DiskManager& _diskManager;
    public:
        DiskWriter() = delete;
        DiskWriter(DiskManager& diskManager) : _diskManager(diskManager) {}
        STATUS_CODE const writeToBlock(const unsigned int& blockNumber, const char* data, const int& bytes);
        std::pair<STATUS_CODE, DirectoryBlock*> const chainDirectoryBlock(DirectoryBlock* const directory);
        WriteResult const addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum);
        WriteResult const createToFile(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type);

};