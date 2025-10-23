#include "../headers/system_manager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

std::deque<std::string> SystemManager::tokenizeString(const std::string& str, const char& delim)
{
    std::deque<std::string> nameBufferQueue;
    std::string bufferCopy = str;
    size_t startPos = 0;
    size_t splitPos = 0;
    while(splitPos != std::string::npos){
        splitPos = bufferCopy.find(delim, startPos);
        nameBufferQueue.push_back(bufferCopy.substr(startPos, splitPos - startPos));
        startPos = splitPos + 1;
    }

    return nameBufferQueue;
}

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}

STATUS_CODE SystemManager::CREATE(const char& type, const std::string& nameBuffer)
{
    if(type != 'U' && type != 'D') return STATUS_CODE::INVALID_TYPE;
    std::deque<std::string> fullPath = tokenizeString(nameBuffer, PATH_DELIMITER);
    std::deque<std::string> fullPathCopy = fullPath; // To keep full path intact since findFile modifies the deque
    std::string fileName = fullPath.back();
    SearchResult searchResult = _diskManager.findFile(fullPath);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    WriteResult writeResult;
    // Found file with same name in last directory of given path
    if(status == STATUS_CODE::SUCCESS)
    {
        writeResult = _diskManager.DWRITE(parentDir, entryIndex, fileName.c_str(), type);
        if(writeResult.status != STATUS_CODE::SUCCESS) return writeResult.status;
        _lastOpened = writeResult.entry;
        _fileMode = 'O';
        return writeResult.status;
    }
    else if(status == STATUS_CODE::NO_FILE_FOUND)
    {
        // No file exists with same name
        // Check how many directories of given path don't exist. Will need to create that many directories
        std::deque<std::string> existingPathBufferQueue;
        std::deque<std::string> needToCreate;

        for(auto& component : fullPathCopy) 
        {
            existingPathBufferQueue.push_back(component);
            auto probePath = existingPathBufferQueue;
            SearchResult searchResult = _diskManager.findFile(probePath);
            if(searchResult.statusCode != STATUS_CODE::SUCCESS) 
            {
                needToCreate.push_back(component);
                auto it = std::find(fullPathCopy.begin(), fullPathCopy.end(), component);
                for(++it; it != fullPathCopy.end(); ++it) needToCreate.push_back(*it);
                existingPathBufferQueue.pop_back();
                break;
            }
            else{
                if(searchResult.directory->getDir()[searchResult.entryIndex].TYPE != 'D') return STATUS_CODE::ILLEGAL_ACCESS;
            }
        }
        
        int numNeededFreeBlocks = needToCreate.size();
        if(numNeededFreeBlocks > _diskManager.getNumFreeBlocks()) return STATUS_CODE::OUT_OF_MEMORY;
        writeResult = _diskManager.DWRITE(existingPathBufferQueue, needToCreate, type);

        if(writeResult.status != STATUS_CODE::SUCCESS) return writeResult.status;
        _lastOpened = writeResult.entry;
        _fileMode = 'O';
        return writeResult.status;
    }

    return status;
    
}

STATUS_CODE SystemManager::OPEN(const char& mode, const std::string& nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);

    SearchResult searchResult = _diskManager.findFile(nameBufferQueue);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;
    if(!parentDir) return STATUS_CODE::NO_FILE_FOUND;

    _lastOpened = &parentDir->getDir()[entryIndex];
    _fileMode = mode;
    if(mode == 'I' || mode == 'U') _filePointer = 0;
    else{
        //_lastOpened SIZE should have Bytes used in last block!
        _filePointer = _diskManager.countNumBlocks(_lastOpened->LINK) * 504 - (504 - _lastOpened->SIZE);
    }

    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::CLOSE()
{
    _lastOpened = nullptr;

    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::DELETE(const std::string& nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    SearchResult searchResult = _diskManager.findFile(nameBufferQueue);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    if(!parentDir) return STATUS_CODE::NO_FILE_FOUND;
    Entry* toDelete = &parentDir->getDir()[entryIndex];
    _diskManager.freeBlock(toDelete->LINK);
    toDelete->TYPE = 'F';
    if(_lastOpened->NAME == toDelete->NAME) 
    {
        _lastOpened = nullptr;
        _fileMode = 'I';
        _filePointer = 0;
    }
    return STATUS_CODE::SUCCESS;
}

std::pair<STATUS_CODE, std::string> SystemManager::READ(const unsigned int& numBytes)
{
    if(_fileMode != 'U') return {STATUS_CODE::BAD_FILE_MODE, "BADFILEMODE"};
    if(!_lastOpened) return {STATUS_CODE::NO_FILE_OPEN, "NOFILEOPEN"};

    std::string readData = "";
    UserDataBlock* dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(_lastOpened->LINK));
    if(!dataBlock) return {STATUS_CODE::ILLEGAL_ACCESS, "NOLINKTODATABLOCK"};

    unsigned int readBytes = numBytes;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE + 1;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    unsigned int readBlock = _lastOpened->LINK;

    for(int i = 0 ; i < pointerBlock - 1; ++i)
    {
        readBlock = dataBlock->getNextBlock();
        if(readBlock == 0) return {STATUS_CODE::ILLEGAL_ACCESS, "POINTEROUTOFBOUNDS"};
        Block* nextBlock = _diskManager.getBlock(dataBlock->getNextBlock());
        if(!nextBlock) return {STATUS_CODE::ILLEGAL_ACCESS, "NEXTDATABLOCKNULL"};
        dataBlock = dynamic_cast<UserDataBlock*>(nextBlock);
    }
    unsigned int readStart = pointerOffset;
    while(dataBlock && readBytes > 0)
    {
        unsigned int bytesInBlock = dataBlock->getUserDataSize();
        unsigned int bytesToRead = std::min(readBytes, bytesInBlock - readStart);
        auto[status, readBuffer] = _diskManager.DREAD(readBlock, bytesToRead, readStart);
        if(status != STATUS_CODE::SUCCESS) return {status, readBuffer};
        readData.append(readBuffer);
        readBytes -= bytesToRead;
        readBlock = dataBlock->getNextBlock();
        if(readBytes <= 0 || readBlock == 0) break;
        (readBlock == 0) ? dataBlock = nullptr : dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(readBlock));
        readStart = 0;
    }
    return {STATUS_CODE::SUCCESS, readData};
}

STATUS_CODE SystemManager::WRITE(const int& numBytes, const std::string& writeBuffer)
{
    if(_fileMode != 'O' && _fileMode != 'U') return STATUS_CODE::BAD_FILE_MODE;
    if(!_lastOpened) return STATUS_CODE::NO_FILE_OPEN;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    
    unsigned int writeBlock = _lastOpened->LINK;
    unsigned int writeStart = pointerOffset;
    UserDataBlock* currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(writeBlock));
    if(!currentBlock) return STATUS_CODE::ILLEGAL_ACCESS;

    for(unsigned int i = 0 ; i < pointerBlock; ++i)
    {
        writeBlock = currentBlock->getNextBlock();
        if(writeBlock == 0) return STATUS_CODE::ILLEGAL_ACCESS;
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(writeBlock));
        if(!currentBlock) return STATUS_CODE::ILLEGAL_ACCESS;
    }

    std::string writeData = writeBuffer;
    if(writeData.size() < numBytes)
    {
        unsigned int excessBytes = numBytes - writeData.size();
        writeData.append(excessBytes, ' ');
    }

    unsigned int bytesUpToPointer = pointerBlock * USER_DATA_SIZE + pointerOffset;
    unsigned int totalBytesAllocated = _diskManager.countNumBlocks(_lastOpened->LINK) * USER_DATA_SIZE;
    unsigned int freeBytes = totalBytesAllocated - bytesUpToPointer;
    unsigned int bytesToWrite = numBytes;
    unsigned int bufferStart = 0;

    if(bytesToWrite <= freeBytes)
    {
        while(currentBlock && bytesToWrite > 0)
        {
            unsigned int writeAmount = std::min(USER_DATA_SIZE, bytesToWrite);
            STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
            if(status != STATUS_CODE::SUCCESS) return status;
            bytesToWrite = bytesToWrite - writeAmount;
            bufferStart += writeAmount;
            if(currentBlock->getNextBlock() == 0)
            {
                _lastOpened->SIZE = std::max(_lastOpened->SIZE, writeAmount);
                break;
            }
            currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(currentBlock->getNextBlock()));
            writeStart = 0;
        }
        return STATUS_CODE::SUCCESS;
    }

    unsigned int blocksNeeded = ceil((bytesToWrite - freeBytes) / USER_DATA_SIZE);
    unsigned int numFreeBlocks = _diskManager.getNumFreeBlocks();
    if(numFreeBlocks == 0) return STATUS_CODE::OUT_OF_MEMORY;

    while(blocksNeeded > numFreeBlocks){
        blocksNeeded--;
        bytesToWrite -= USER_DATA_SIZE;
    }

    // Fill Last Block
    unsigned int writeAmount = freeBytes;
    STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
    if(status != STATUS_CODE::SUCCESS) return status;
    bytesToWrite -= writeAmount;
    bufferStart += writeAmount;
    writeStart = 0;

    // Now Allocate & fill
    while(currentBlock && bytesToWrite > 0)
    {   
        // Note: Create function that will auto chain the new block? DWRITE overload?
        auto [allocStatus, newBlock] = _diskManager.allocateBlock('U');
        if(allocStatus != STATUS_CODE::SUCCESS) return allocStatus;
        currentBlock->setNextBlock(newBlock);
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.getBlock(newBlock));
        if(!currentBlock) return STATUS_CODE::UNKNOWN_ERROR;
        currentBlock->setPrevBlock(writeBlock);
        writeBlock = newBlock;

        writeAmount = std::min(USER_DATA_SIZE, bytesToWrite);
        STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
        if(status != STATUS_CODE::SUCCESS) return status;
        bytesToWrite = bytesToWrite - writeAmount;
        bufferStart += writeAmount;
    }
    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::SEEK(const int& base, const int& offset)
{
    if(_fileMode != 'I' && _fileMode != 'U') return STATUS_CODE::BAD_FILE_MODE;
    if(!_lastOpened) return STATUS_CODE::NO_FILE_OPEN;
    if(base < -1 || base > 1) return STATUS_CODE::BAD_COMMAND;
    unsigned int numBlocks = _diskManager.countNumBlocks(_lastOpened->LINK);
    unsigned int totalBytes = numBlocks * USER_DATA_SIZE;

    unsigned int lastByte = totalBytes - (USER_DATA_SIZE - _lastOpened->SIZE);
    unsigned int startByte = _filePointer;

    if(base == -1){
        startByte = 0;
    }
    else if(base == 1){
        startByte = lastByte; // Last Byte is EOF
    }
    int seekByte = startByte + offset;

    if(seekByte < 0) seekByte = 0;
    if(static_cast<unsigned int>(seekByte) > lastByte) seekByte = lastByte;

    _filePointer = seekByte;
    return STATUS_CODE::SUCCESS;

}