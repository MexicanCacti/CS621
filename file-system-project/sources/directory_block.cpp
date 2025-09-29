#include "../headers/directory_block.hpp"
#include <cstring>

Entry* const DirectoryBlock::findFile(std::deque<std::string>& nameBuffer, DiskManager& diskManager){
    if(nameBuffer.empty()) return nullptr;
    std::string currentName = nameBuffer.front();
    nameBuffer.pop_front();

    for(unsigned int i = 0; i < 31; ++i){
        if(DIR[i].TYPE == 'F') continue;

        if(strncmp(DIR[i].NAME, currentName.c_str(), 9) == 0){
            if(nameBuffer.empty()) return &DIR[i];
            if(DIR[i].TYPE != 'D') continue;
            Block* directoryBlock = diskManager.getBlock(DIR[i].LINK);
            if(!directoryBlock) return nullptr;
            DirectoryBlock* searchDirectory = dynamic_cast<DirectoryBlock*>(directoryBlock);
            if(!searchDirectory) return nullptr;
            return searchDirectory->findFile(nameBuffer, diskManager);
        }
    }

    return nullptr;
}