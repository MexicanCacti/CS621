#pragma once
#include <deque>
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"
#include "../utils/path_result.hpp"

class DiskManager;

class DiskWriter{
    private:
        DiskManager& _diskManager;
        STATUS_CODE const writeToBlock(const unsigned int& blockNumber, const char* data, const int& bytes);
        std::pair<STATUS_CODE, DirectoryBlock*> const chainDirectoryBlock(DirectoryBlock* const directory);
        WriteResult const addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum, std::string& existingPath);
    public:
        DiskWriter() = delete;
        DiskWriter(DiskManager& diskManager) : _diskManager(diskManager) {}
        WriteResult const createToFile(PathResult& pathResult, const char& type);
        WriteResult const createFile(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type, PathResult& pathResult);

};