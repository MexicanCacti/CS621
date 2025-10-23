#include "../headers/disk_writer.hpp"
#include "../headers/disk_manager.hpp"
STATUS_CODE const DiskWriter::writeToBlock(UserDataBlock* dataBlock, const char* data, const int& bytes, const int& startByte, const unsigned int& bufferStart)
{
    if(!dataBlock || !data) return BAD_ARG;
    if(startByte >= _diskManager.getBlockSize()) return BOUNDS_ERROR;
    char* userData = dataBlock->getUserData();
    char tempData [USER_DATA_SIZE + 1];

    for(int i = 0 ; i < startByte; ++i)
    {
        tempData[i] = userData[i];
    }

    for(int i = 0 ; i < bytes; ++i)
    {
        tempData[i + startByte] = data[i + bufferStart];
    }

    tempData[USER_DATA_SIZE] = '\0';

    dataBlock->setUserData(tempData);

    return SUCCESS;
}

std::pair<STATUS_CODE, DirectoryBlock*> const DiskWriter::chainDirectoryBlock(DirectoryBlock* const directory)
{
    auto [status, freeBlock] = _diskManager.allocateBlock('D');
    if(status != SUCCESS) return {status, nullptr};

    directory->setNextBlock(freeBlock);

    DirectoryBlock* newChain = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(freeBlock));
    if(!newChain){
        _diskManager.freeBlock(freeBlock);
        directory->setNextBlock(0);
        return {CASTING_ERROR, nullptr};
    }

    return {SUCCESS, newChain};

}

WriteResult const DiskWriter::addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum)
{
    if(entryIndex > MAX_DIRECTORY_ENTRIES) return {BAD_ARG, nullptr, type};
    
    directory->getDir()[entryIndex] = {name, type, blockNum, 0};

    return {SUCCESS, &directory->getDir()[entryIndex], type};
}

/*
    NOTE: already guaranteed to have enough space to allocate enough blocks in nameBufferQueue
    Steps
    1. Find the last directory of the existing path. Guaranteed that last directory is descendent of root if called, otherwise addEntryToDirectory would have been used
    2. Check if the last directory has a free entry
        a. If not, check there is enough memory to chain an additional directory block & have enough blocks for the nameBufferQueue
    3. Now take from the nameBufferQueue to create directories until the end file is reached
    4. Place the end file as an entry in the last parent directory
*/
WriteResult const DiskWriter::createToFile(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type)
{
    DirectoryBlock* directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(0));
    // Find last valid directory
    if(!existingPath.empty())
    {
        SearchResult findStartPoint = _diskManager.findFile(existingPath);
        if(findStartPoint.statusCode != SUCCESS) return {findStartPoint.statusCode, nullptr, type};
        Entry startPoint = findStartPoint.directory->getDir()[findStartPoint.entryIndex];
        if(startPoint.TYPE != 'D') return {BAD_TYPE, nullptr, type};
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(startPoint.LINK));
    }

    if(!directory) return {CASTING_ERROR, nullptr, type};

    // Now do a check if it has a free entry, if it doesn't -> need to chain... but ensure that we have enough space for a chain!
    unsigned int nextFreeEntry = directory->findFreeEntry();
    if(nextFreeEntry == MAX_DIRECTORY_ENTRIES){
        if(nameBufferQueue.size() + 1 > _diskManager.getNumFreeBlocks()) return {OUT_OF_MEMORY, nullptr, type};
        auto [chainStatus, chain] = chainDirectoryBlock(directory);
        if(chainStatus != SUCCESS) return {chainStatus, nullptr, type};
        directory = chain;
        nextFreeEntry = directory->findFreeEntry();
    }
    
    while(nameBufferQueue.size() > 1){
        std::string bufferString = nameBufferQueue.front();
        auto [status, freeBlock] = _diskManager.allocateBlock('D');
        if(status != SUCCESS) return {status, nullptr, type};
        addEntryToDirectory(directory, nextFreeEntry, bufferString.c_str(), 'D', freeBlock);
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(freeBlock));
        nameBufferQueue.pop_front();
        nextFreeEntry = directory->findFreeEntry();
    }

    auto [status, freeBlock] = _diskManager.allocateBlock(type);
    if(status != SUCCESS) return {status, nullptr, type};
    return addEntryToDirectory(directory, nextFreeEntry, nameBufferQueue.back().c_str(), type, freeBlock);

}