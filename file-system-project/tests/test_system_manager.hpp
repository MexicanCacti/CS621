#pragma once
#include "../headers/system_manager.hpp"

const int NUM_BLOCKS = 10;
const int BLOCK_SIZE = 512;
const int USER_DATA_SIZE = 504;


class TestSystemManager : public SystemManager {

    public:
        TestSystemManager(DiskManager& diskManager, const std::string& rootName) : SystemManager(diskManager, rootName) {};
        void setEntry(Entry* entry) {
            STATUS_CODE code = _diskManager.allocateBlock(entry->LINK, 'U');
            if(code == STATUS_CODE::SUCCESS) _lastOpened = entry;
        }
        Entry* const getEntry() {return _lastOpened;}
        void setFileMode(char mode) {_fileMode = mode;}
        void setRoot(RootBlock* root) {_rootBlock = root;}
        void setFilePointer(const unsigned int& fp) {_filePointer = fp;}
        unsigned int const getFilePointer() { return _filePointer;}
};