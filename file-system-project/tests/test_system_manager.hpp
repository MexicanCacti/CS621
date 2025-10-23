#pragma once
#include "../headers/system_manager.hpp"
#include <iostream>
#include <algorithm>
class TestSystemManager : public SystemManager {

    public:
        TestSystemManager(DiskManager& diskManager, const std::string& rootName) : SystemManager(diskManager, rootName) {};
        void setEntry(Entry* entry) {
            STATUS_CODE code = _diskManager.allocateBlock('U').first;
            if(code == STATUS_CODE::SUCCESS) _lastOpened = entry;
        }
        Entry* const getEntry() {return _lastOpened;}
        void setFileMode(char mode) {_fileMode = mode;}
        char getFileMode() {return _fileMode;}
        void setRoot(DirectoryBlock* root) {_rootBlock = root;}
        void setFilePointer(const unsigned int& fp) {_filePointer = fp;}
        unsigned int const getFilePointer() { return _filePointer;}
        SearchResult findCreatedFile(const std::string& filePath);
        unsigned int getNextFreeBlock();
        //std::pair<STATUS_CODE, std::string> READ(const unsigned int& numBytes);
        std::pair<STATUS_CODE, std::string> READALL();
        //STATUS_CODE WRITE(const int& numBytes, const std::string& writeBuffer);

};