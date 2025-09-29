#include "../headers/directory_block.hpp"
#include <cstring>

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