#pragma once
#include <deque>
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"
#include "../headers/user_data_block.hpp"

class DiskManager;

class DiskWriter{
    private:
        DiskManager& _diskManager;
    public:
        DiskWriter() = delete;
        DiskWriter(DiskManager& diskManager) : _diskManager(diskManager) {}
        STATUS_CODE const writeToBlock(UserDataBlock* dataBlock, const char* data, const int& bytes, const int& startByte, const unsigned int& bufferStart);
        std::pair<STATUS_CODE, DirectoryBlock*> const chainDirectoryBlock(DirectoryBlock* const directory);
        WriteResult const addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum);
        WriteResult const createToFile(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type);

};