#include "../headers/disk_writer.hpp"
#include "../headers/disk_manager.hpp"

STATUS_CODE const DiskWriter::writeToBlock(const unsigned int& blockNumber, const char* data, const int& bytes)
{
    return STATUS_CODE::SUCCESS;
}

std::pair<STATUS_CODE, DirectoryBlock*> const DiskWriter::chainDirectoryBlock(DirectoryBlock* const directory)
{
    auto [status, freeBlock] = _diskManager.allocateBlock('D');
    if(status != STATUS_CODE::SUCCESS) return {status, nullptr};

    directory->setNextBlock(freeBlock);

    DirectoryBlock* newChain = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(freeBlock));
    if(!newChain){
        _diskManager.freeBlock(freeBlock);
        directory->setNextBlock(0);
        return {STATUS_CODE::UNKNOWN_ERROR, nullptr};
    }

    return {STATUS_CODE::SUCCESS, newChain};

}

STATUS_CODE const DiskWriter::addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum)
{
    // Check if the directory & entry is free. If not look for next free entry. If no free entry, then try to allocate & chain a new directory.
    if(entryIndex > MAX_DIRECTORY_ENTRIES) return STATUS_CODE::BAD_COMMAND;

    if(directory->getDir()[entryIndex].TYPE != 'F'){
        
    }
    else{
        directory->addEntry(name, type, entryIndex, blockNum);
    }
    return STATUS_CODE::SUCCESS;
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
STATUS_CODE const DiskWriter::createToFile(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type)
{
    // Find last valid directory
    SearchResult findStartPoint = _diskManager.findFile(existingPath);
    if(findStartPoint.statusCode != STATUS_CODE::SUCCESS) return STATUS_CODE::NO_FILE_FOUND;

    Entry startPoint = findStartPoint.directory->getDir()[findStartPoint.entryIndex];
    if(startPoint.TYPE != 'D') return STATUS_CODE::UNKNOWN_ERROR;

    DirectoryBlock* directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(startPoint.LINK));
    if(!directory) return STATUS_CODE::UNKNOWN_ERROR;

    // Now do a check if it has a free entry, if it doesn't need to chain... but ensure that we have enough space for a chain!
    unsigned int nextFreeEntry = directory->findFreeEntry();
    if(nextFreeEntry == MAX_DIRECTORY_ENTRIES){
        if(nameBufferQueue.size() + 1 > _diskManager.getNumFreeBlocks()) return STATUS_CODE::OUT_OF_MEMORY;
        auto [chainStatus, chain] = chainDirectoryBlock(directory);
        if(chainStatus != STATUS_CODE::SUCCESS) return chainStatus;
        directory = chain;
        nextFreeEntry = directory->findFreeEntry();
    }
    
    while(nameBufferQueue.size() > 1){
        std::string bufferString = nameBufferQueue.front();
        auto [status, freeBlock] = _diskManager.allocateBlock('D');
        if(status != STATUS_CODE::SUCCESS) return status;
        addEntryToDirectory(directory, nextFreeEntry, bufferString.c_str(), 'D', freeBlock);
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(freeBlock));
        nameBufferQueue.pop_front();
        nextFreeEntry = directory->findFreeEntry();
    }

    auto [status, freeBlock] = _diskManager.allocateBlock(type);
    if(!status != STATUS_CODE::SUCCESS) return status;
    addEntryToDirectory(directory, nextFreeEntry, nameBufferQueue.front().c_str(), type, freeBlock);
    return STATUS_CODE::SUCCESS;

}