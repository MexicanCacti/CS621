#pragma once
#include "block.hpp"
#include "disk_manager.hpp"
#include "directory_block.hpp"
#include "../utils/status_codes.hpp"
#include "file_block.hpp"
#include "root_block.hpp"
#include <deque>

#include <string>

class SystemManager{
    protected:
        DiskManager& _diskManager;
        Entry* _lastOpened = nullptr;
        char _fileMode = 'I';
        unsigned int _filePointer = 0;
        RootBlock* _rootBlock = nullptr;

        std::deque<std::string> tokenizeString(const std::string& str, const char& delim);
        Entry* findFile(std::deque<std::string> nameBuffer);
        SystemManager() = delete;
    public:
        SystemManager(DiskManager& diskManager, const std::string& rootName);
        STATUS_CODE CREATE(const char& type, const std::string nameBuffer);
        STATUS_CODE OPEN(const char& mode, const std::string nameBuffer);
        STATUS_CODE CLOSE();
        STATUS_CODE DELETE(const std::string nameBuffer);
        std::pair<STATUS_CODE, std::string> READ(const int& numBytes);
        STATUS_CODE WRITE(const int& numBytes, const std::string writeBuffer);
        STATUS_CODE SEEK(const int& base, const int& offset);
};