#include "../headers/system_manager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>

std::deque<std::string> SystemManager::tokenizeString(const std::string& str, const char& delim)
{
    std::deque<std::string> nameBufferQueue;
    size_t startPos = 0;
    size_t splitPos = 0;
    while(splitPos != std::string::npos){
        splitPos = str.find(delim, startPos);
        nameBufferQueue.push_back(str.substr(startPos, splitPos - startPos));
        startPos = splitPos + 1;
    }

    return nameBufferQueue;
}

DirectoryResults SystemManager::getDirectories()
{
    std::queue<unsigned int> dirOrder;
    std::queue<std::string> dirNames;
    std::queue<std::pair<unsigned int, std::string>> dirQueue;
    dirQueue.push({0, "ROOT"});

    while(!dirQueue.empty()){
        unsigned int queueSize = dirQueue.size();
        for(unsigned int i = 0 ; i < queueSize; ++i)
        {
            auto[dirBlockNumber, dirName] = dirQueue.front();
            dirQueue.pop();
            dirOrder.push(dirBlockNumber);
            dirNames.push(dirName);
            DirectoryBlock* dirBlock = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlockNumber));
            if(!dirBlock) return {dirOrder, dirNames, CASTING_ERROR};

            for(dirBlock; 
                dirBlock != nullptr; 
                dirBlock = (dirBlock->getNextBlock() != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlock->getNextBlock())) : nullptr)
            {
                auto dir = dirBlock->getDir();
                for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i) {
                    Entry& e = dir[i];
                    if(e.TYPE == 'D')
                    {
                        dirQueue.push({e.LINK, e.NAME});
                    }
                }
            }
        }
    }
    return {dirOrder, dirNames, SUCCESS};
}

void SystemManager::outputFileSystem(std::vector<std::string>& dirNames, std::vector<std::vector<entryPair>>& directoryEntries)
{
    const int dirWidth = MAX_NAME_LENGTH + 2;
    const int entryWidth = MAX_NAME_LENGTH + 2;
    const int typeWidth = 4;
    std::cout << std::left << "Note: |x| indicates no entries in directory\n";
    std::cout << std::setw(dirWidth) << "Directory" << std::setw(entryWidth) << "Entry" << std::setw(typeWidth) << "Type" << std::endl;
    std::cout << std::string(dirWidth + entryWidth + typeWidth, '-') << std::endl;
    unsigned int numDirs = directoryEntries.size();
    for (unsigned int i = 0; i < numDirs; ++i)
    {
        std::string& dirName = dirNames[i];
        std::vector<entryPair>& entries = directoryEntries[i];

        if (entries.empty())
        {
            std::cout << std::setw(dirWidth) << dirName << std::setw(entryWidth) << "|x|" << std::setw(typeWidth) << "|x|" << std::endl;
        }
        else
        {
            for (unsigned int j = 0; j < entries.size(); ++j)
            {
                if (j == 0) std::cout << std::setw(dirWidth) << dirName << std::setw(entryWidth) << entries[j].first << std::setw(typeWidth) << entries[j].second << std::endl;
                else std::cout << std::setw(dirWidth) << "" << std::setw(entryWidth) << entries[j].first << std::setw(typeWidth) << entries[j].second << std::endl;
            }
        }
    }
    std::cout << std::string(dirWidth + entryWidth + typeWidth, '-') << std::endl;
}


SystemManager::SystemManager(DiskManager& diskManager) : _diskManager(diskManager)
{
    
}

STATUS_CODE SystemManager::CREATE(const char& type, const std::string& nameBuffer)
{
    if(type != 'U' && type != 'D') return BAD_TYPE;
    std::deque<std::string> fullPath = tokenizeString(nameBuffer, PATH_DELIMITER);
    std::deque<std::string> fullPathCopy = fullPath; // To keep full path intact since findFile modifies the deque
    std::string fileName = fullPath.back();
    SearchResult searchResult = _diskManager.findFile(fullPath);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    WriteResult writeResult;
    // Found file with same name in last directory of given path
    if(status == SUCCESS)
    {
        writeResult = _diskManager.DWRITE(parentDir, entryIndex, fileName.c_str(), type);
        if(writeResult.status != SUCCESS) return writeResult.status;
        _lastOpened = writeResult.entry;
        _fileMode = 'O';
        return writeResult.status;
    }
    else if(status == NO_FILE_FOUND)
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
            if(searchResult.statusCode != SUCCESS) 
            {
                needToCreate.push_back(component);
                auto it = std::find(fullPathCopy.begin(), fullPathCopy.end(), component);
                for(++it; it != fullPathCopy.end(); ++it) needToCreate.push_back(*it);
                existingPathBufferQueue.pop_back();
                break;
            }
            else{
                if(searchResult.directory->getDir()[searchResult.entryIndex].TYPE != 'D') return ILLEGAL_ACCESS;
            }
        }
        
        int numNeededFreeBlocks = needToCreate.size();
        if(numNeededFreeBlocks > _diskManager.getNumFreeBlocks()) return OUT_OF_MEMORY;
        writeResult = _diskManager.DWRITE(existingPathBufferQueue, needToCreate, type);

        if(writeResult.status != SUCCESS) return writeResult.status;
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
    if(!parentDir) return NO_FILE_FOUND;
    if(parentDir->getDir()[entryIndex].TYPE != 'U') return NO_FILE_FOUND;
    _lastOpened = &parentDir->getDir()[entryIndex];
    _fileMode = mode;
    if(mode == 'I' || mode == 'U') _filePointer = 0;
    else{
        //_lastOpened SIZE should have Bytes used in last block!
        _filePointer = _diskManager.countNumBlocks(_lastOpened->LINK) * 504 - (504 - _lastOpened->SIZE);
    }

    return SUCCESS;
}

STATUS_CODE SystemManager::CLOSE()
{
    _lastOpened = nullptr;
    _filePointer = 0;
    _fileMode = 'I';
    return SUCCESS;
}

STATUS_CODE SystemManager::DELETE(const std::string& nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    SearchResult searchResult = _diskManager.findFile(nameBufferQueue);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    if(!parentDir) return NO_FILE_FOUND;
    Entry* toDelete = &parentDir->getDir()[entryIndex];
    _diskManager.freeBlock(toDelete->LINK);
    toDelete->TYPE = 'F';
    if(_lastOpened->NAME == toDelete->NAME) 
    {
        _lastOpened = nullptr;
        _fileMode = 'I';
        _filePointer = 0;
    }
    return SUCCESS;
}

// NOTE: FIX ME!
std::pair<STATUS_CODE, std::string> SystemManager::READ(const unsigned int& numBytes)
{
    if(_fileMode != 'U') return {BAD_FILE_MODE, "BADFILEMODE"};
    if(!_lastOpened) return {NO_FILE_OPEN, "NOFILEOPEN"};

    std::string readData = "";
    UserDataBlock* dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(_lastOpened->LINK));
    if(!dataBlock) return {CASTING_ERROR, "NOLINKTODATABLOCK"};
    // 1 2 3 | 4 5 6 | 7 8 9
    //   p             x
    // Total : 9
    // Last Block: 1

    unsigned int readBytes = numBytes;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE + 1;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    unsigned int readBlock = _lastOpened->LINK;
    unsigned int blocksAllocated = _diskManager.countNumBlocks(_lastOpened->LINK);
    unsigned int bytesUpToPointer = pointerBlock * USER_DATA_SIZE + pointerOffset;
    unsigned int totalBytesAllocated = blocksAllocated * USER_DATA_SIZE;

    for(int i = 0 ; i < pointerBlock - 1; ++i)
    {
        readBlock = dataBlock->getNextBlock();
        if(readBlock == 0) return {ILLEGAL_ACCESS, "POINTEROUTOFBOUNDS"};
        Block* nextBlock = _diskManager.DREAD(dataBlock->getNextBlock());
        if(!nextBlock) return {CASTING_ERROR, "NEXTDATABLOCKNULL"};
        dataBlock = dynamic_cast<UserDataBlock*>(nextBlock);
    }
    unsigned int readStart = pointerOffset;
    std::pair<STATUS_CODE, std::string> readBuffer;
    while(dataBlock && readBytes > 0)
    {
        unsigned int bytesInBlock = dataBlock->getUserDataSize();
        unsigned int bytesToRead = std::min(readBytes, bytesInBlock - readStart);
        readBuffer = _diskManager.DREAD(readBlock, bytesToRead, readStart);
        if(readBuffer.first != SUCCESS) break;
        readData.append(readBuffer.second);
        readBytes -= bytesToRead;
        readBlock = dataBlock->getNextBlock();
        if(readBytes <= 0 || readBlock == 0) break;
        (readBlock == 0) ? dataBlock = nullptr : dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(readBlock));
        readStart = 0;
    }
    if(totalBytesAllocated -  _lastOpened->SIZE - bytesUpToPointer <= 0) readData.append("\nEnd of File Reached");
    return {readBuffer.first, readData};
}

STATUS_CODE SystemManager::WRITE(const int& numBytes, const std::string& writeBuffer)
{
    if(_fileMode != 'O' && _fileMode != 'U') return BAD_FILE_MODE;
    if(!_lastOpened) return NO_FILE_OPEN;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    
    unsigned int writeBlock = _lastOpened->LINK;
    unsigned int writeStart = pointerOffset;
    UserDataBlock* currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(writeBlock));
    if(!currentBlock) return CASTING_ERROR;

    for(unsigned int i = 0 ; i < pointerBlock; ++i)
    {
        writeBlock = currentBlock->getNextBlock();
        if(writeBlock == 0) return ILLEGAL_ACCESS;
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(writeBlock));
        if(!currentBlock) return CASTING_ERROR;
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
            if(status != SUCCESS) return status;
            bytesToWrite = bytesToWrite - writeAmount;
            bufferStart += writeAmount;
            if(currentBlock->getNextBlock() == 0)
            {
                _lastOpened->SIZE = std::max(_lastOpened->SIZE, writeAmount);
                break;
            }
            currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(currentBlock->getNextBlock()));
            writeStart = 0;
        }
        return SUCCESS;
    }

    unsigned int blocksNeeded = ceil((bytesToWrite - freeBytes) / USER_DATA_SIZE);
    unsigned int numFreeBlocks = _diskManager.getNumFreeBlocks();
    if(numFreeBlocks == 0) return OUT_OF_MEMORY;

    while(blocksNeeded > numFreeBlocks){
        blocksNeeded--;
        bytesToWrite -= USER_DATA_SIZE;
    }

    // Fill Last Block
    unsigned int writeAmount = freeBytes;
    STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
    if(status != SUCCESS) return status;
    bytesToWrite -= writeAmount;
    bufferStart += writeAmount;
    writeStart = 0;

    // Now Allocate & fill
    while(currentBlock && bytesToWrite > 0)
    {   
        // Note: Create function that will auto chain the new block? DWRITE overload?
        auto [allocStatus, newBlock] = _diskManager.allocateBlock('U');
        if(allocStatus != SUCCESS) return allocStatus;
        currentBlock->setNextBlock(newBlock);
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(newBlock));
        if(!currentBlock) return CASTING_ERROR;
        currentBlock->setPrevBlock(writeBlock);
        writeBlock = newBlock;

        writeAmount = std::min(USER_DATA_SIZE, bytesToWrite);
        STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
        if(status != SUCCESS) return status;
        bytesToWrite = bytesToWrite - writeAmount;
        bufferStart += writeAmount;
    }
    return SUCCESS;
}

STATUS_CODE SystemManager::SEEK(const int& base, const int& offset)
{
    if(_fileMode != 'I' && _fileMode != 'U') return BAD_FILE_MODE;
    if(!_lastOpened) return NO_FILE_OPEN;
    if(base < -1 || base > 1) return BAD_ARG;
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
    return SUCCESS;
}

STATUS_CODE SystemManager::displayFileSystem()
{
    DirectoryResults directoryResults = getDirectories();
    if(directoryResults.status != SUCCESS)
    {
        std::cout << "[ERROR]: COULD NOT GET DIRECTORIES IN FILE SYSTEM!" << std::endl;
        return directoryResults.status;
    }
    std::queue<unsigned int>& dirOrder = directoryResults.directoryOrder;
    std::queue<std::string>& dirNames = directoryResults.directoryName;

    unsigned int numDirectories = dirOrder.size();

    std::vector<std::string> directoryNames(numDirectories);
    std::vector<std::vector<entryPair>> directoryEntries(numDirectories);

    unsigned int index = 0;
    while(!dirOrder.empty())
    {
        unsigned int blockNum = dirOrder.front();
        std::string dirName = dirNames.front();
        dirOrder.pop();
        dirNames.pop();
        directoryNames[index] = dirName;
        std::vector<entryPair>& entryList = directoryEntries[index];
        DirectoryBlock* dirBlock = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(blockNum));
        if(!dirBlock)
        {
            std::cout << "[ERROR]: COULD NOT GET DIRECTORY BLOCK " << blockNum << std::endl;
            return CASTING_ERROR;
        }

        for(dirBlock; 
            dirBlock != nullptr; 
            dirBlock = (dirBlock->getNextBlock() != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlock->getNextBlock())) : nullptr)
        {
            auto dir = dirBlock->getDir();
            for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i) {
                Entry& e = dir[i];
                if(e.TYPE != 'F')
                {
                    entryList.push_back({e.NAME, e.TYPE});
                }
            }
        }
        index++;
    }
    outputFileSystem(directoryNames, directoryEntries);
    return STATUS_CODE::SUCCESS;
}

char* SystemManager::getFileName()
{
    if(!_lastOpened) return nullptr;
    return _lastOpened->NAME;
}