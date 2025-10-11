#include "../headers/disk_writer.hpp"
#include "../headers/disk_manager.hpp"
#include <iostream>
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

WriteResult const DiskWriter::addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum, std::string& existingPath)
{
    if(entryIndex > MAX_DIRECTORY_ENTRIES) return {STATUS_CODE::BAD_COMMAND, nullptr, type};
    
    directory->getDir()[entryIndex] = {name, type, blockNum, 0};
    unsigned int parentBlock = _diskManager._pathMap[existingPath];
    existingPath += name;
    _diskManager._pathMap[existingPath] = blockNum;
    _diskManager._parentMap[blockNum] = parentBlock;
    return {STATUS_CODE::SUCCESS, &directory->getDir()[entryIndex], type};
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
// NOTE: Need a way to reverse block allocation & update maps if allocation fails at any point!
WriteResult const DiskWriter::createToFile(PathResult& pathResult, const char& type)
{
    DirectoryBlock* directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(0));
    std::string existingPath = pathResult.existingPath;
    std::stack<std::string> missingPath = pathResult.missingPath;
    auto lastSlash = existingPath.rfind(PATH_DELIMITER);
    auto fileName = (lastSlash == std::string::npos) ? existingPath : existingPath.substr(lastSlash + 1);
    // Find last valid directory
    if(!existingPath.empty())
    {
        SearchResult findStartPoint = _diskManager.findPath(existingPath, fileName);
        if(findStartPoint.statusCode != STATUS_CODE::SUCCESS) return {STATUS_CODE::NO_FILE_FOUND, nullptr, type};
        Entry startPoint = findStartPoint.directory->getDir()[findStartPoint.entryIndex];
        if(startPoint.TYPE != 'D') return {STATUS_CODE::UNKNOWN_ERROR, nullptr, type};
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(startPoint.LINK));
    }

    if(!directory) return {STATUS_CODE::UNKNOWN_ERROR, nullptr, type};

    // Now do a check if it has a free entry, if it doesn't -> need to chain... but ensure that we have enough space for a chain!
    unsigned int nextFreeEntry = directory->findFreeEntry();
    if(nextFreeEntry == MAX_DIRECTORY_ENTRIES){
        if(missingPath.size() + 1 > _diskManager.getNumFreeBlocks()) return {STATUS_CODE::OUT_OF_MEMORY, nullptr, type};
        auto [chainStatus, chain] = chainDirectoryBlock(directory);
        if(chainStatus != STATUS_CODE::SUCCESS) return {chainStatus, nullptr, type};
        directory = chain;
        nextFreeEntry = directory->findFreeEntry();
    }
    
    while(missingPath.size() > 1){
        std::string bufferString = missingPath.top();
        auto [status, freeBlock] = _diskManager.allocateBlock('D');
        if(status != STATUS_CODE::SUCCESS) return {status, nullptr, type};
        addEntryToDirectory(directory, nextFreeEntry, bufferString.c_str(), 'D', freeBlock, existingPath);
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(freeBlock));
        missingPath.pop();
        nextFreeEntry = directory->findFreeEntry();
    }

    auto [status, freeBlock] = _diskManager.allocateBlock(type);
    if(status != STATUS_CODE::SUCCESS) return {status, nullptr, type};
    return addEntryToDirectory(directory, nextFreeEntry, missingPath.top().c_str(), type, freeBlock, existingPath);
}

WriteResult const DiskWriter::createFile(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type, PathResult& pathResult)
{
    // NOTE: Need a way to reverse the free if allocation fails!
    std::string existingPath = pathResult.existingPath;
    std::stack<std::string> missingStack = pathResult.missingPath;
    std::string missingPath = "";
    _diskManager.freeEntry(existingPath);
    auto [status, allocatedBlock] = _diskManager.allocateBlock(type);
    if(status != STATUS_CODE::SUCCESS) return {status, nullptr, type};

    while(!missingStack.empty())
    {
        missingPath.insert(0, missingStack.top());
        missingStack.pop();
    }
    
    std::string fullPath = existingPath + "/" + missingPath;
    return addEntryToDirectory(directory, entryIndex, name, type, allocatedBlock, fullPath);
}