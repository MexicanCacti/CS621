#include "../headers/directory_block.hpp"

Entry* const DirectoryBlock::getEntry(unsigned int& index)
{
    if(index < 0 || index >= MAX_DIRECTORY_ENTRIES) return nullptr;
    return &DIR[index];
}

std::pair<DirectoryBlock*, unsigned int> const DirectoryBlock::findFile(std::deque<std::string>& nameBuffer, DiskManager& diskManager, const char& type)
{
    if(nameBuffer.empty()) return {nullptr, 0};
    std::string currentName = nameBuffer.front();
    nameBuffer.pop_front();

    for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
        if(DIR[i].TYPE == 'F') continue;

        if(strncmp(DIR[i].NAME, currentName.c_str(), 9) == 0){
            if(nameBuffer.empty() && DIR[i].TYPE == type) return {this, i};
            if(DIR[i].TYPE != 'D') continue;
            Block* directoryBlock = diskManager.getBlock(DIR[i].LINK);
            if(!directoryBlock) return {nullptr, 0};
            DirectoryBlock* searchDirectory = dynamic_cast<DirectoryBlock*>(directoryBlock);
            if(!searchDirectory) return {nullptr, 0};
            return searchDirectory->findFile(nameBuffer, diskManager, type);
        }
    }

    // No match in this directory block, check next block in FRWD chain
    Block* currentBlock = this;
    while(currentBlock->getNextBlock() != 0){
        currentBlock = diskManager.getBlock(currentBlock->getNextBlock());
        if(!currentBlock) return {nullptr, 0};
        DirectoryBlock* nextDirBlock = dynamic_cast<DirectoryBlock*>(currentBlock);
        if(!nextDirBlock) return {nullptr, 0};
        for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
            if(nextDirBlock->DIR[i].TYPE == 'F') continue;

            if(strncmp(nextDirBlock->DIR[i].NAME, currentName.c_str(), 9) == 0){
                if(nameBuffer.empty() && nextDirBlock->DIR[i].TYPE == type) return {nextDirBlock, i};
                if(nextDirBlock->DIR[i].TYPE != 'D') continue;
                Block* directoryBlock = diskManager.getBlock(nextDirBlock->DIR[i].LINK);
                if(!directoryBlock) return {nullptr, 0};
                DirectoryBlock* searchDirectory = dynamic_cast<DirectoryBlock*>(directoryBlock);
                if(!searchDirectory) return {nullptr, 0};
                return searchDirectory->findFile(nameBuffer, diskManager, type);
            }
        }
        if(nextDirBlock->FRWD == 0) break;
    }

    return {nullptr, 0};
}

STATUS_CODE DirectoryBlock::addEntry(const char* name, RootBlock* rootDirectory, const char& type, DiskManager& diskManager)
{
    unsigned int nextFreeBlock = rootDirectory->getNextFreeBlock();
    if(nextFreeBlock == 0) return STATUS_CODE::OUT_OF_MEMORY;

    for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
        if(DIR[i].TYPE == 'F'){
            STATUS_CODE allocateStatus = diskManager.allocateBlock(type);
            if(allocateStatus != STATUS_CODE::SUCCESS) return allocateStatus;
            DIR[i] = Entry(type, name, nextFreeBlock);
            return STATUS_CODE::SUCCESS;
        }
    }
    // No room in this directory block, check next block in FRWD chain
    Block* currentBlock = this;
    while(currentBlock->getNextBlock() != 0){
        currentBlock = diskManager.getBlock(currentBlock->getNextBlock());
        if(!currentBlock) return STATUS_CODE::UNKNOWN_ERROR;
        DirectoryBlock* nextDirBlock = dynamic_cast<DirectoryBlock*>(currentBlock);
        if(!nextDirBlock) return STATUS_CODE::UNKNOWN_ERROR;
        for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
            if(nextDirBlock->DIR[i].TYPE == 'F'){
                STATUS_CODE allocateStatus = diskManager.allocateBlock(type);
                if(allocateStatus != STATUS_CODE::SUCCESS) return allocateStatus;
                nextDirBlock->DIR[i] = Entry(type, name, nextFreeBlock);
                return STATUS_CODE::SUCCESS;
            }
        }
        if(nextDirBlock->FRWD == 0) break;
        this->FRWD = nextDirBlock->FRWD;
    }

    // So now we need to allocate a new directory block and chain it to last FRWD
    return STATUS_CODE::SUCCESS;


}