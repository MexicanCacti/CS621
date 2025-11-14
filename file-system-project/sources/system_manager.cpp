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

void SystemManager::outputFileSystem(std::vector<std::string>& dirNames, std::vector<std::vector<EntryInfo>>& directoryEntries)
{
    const int dirWidth = MAX_NAME_LENGTH + 2;
    const int fileLengthWidth = 20;
    const int entryWidth = MAX_NAME_LENGTH + fileLengthWidth;
    const int typeWidth = 6;
    const int freeBlockWidth = 25;
    const int dirCountWidth = 25;
    const int userCountWidth = 25;
    unsigned int numDirs = directoryEntries.size();
    unsigned int freeBlocks = NUM_BLOCKS - numDirs;
    unsigned int numUserBlocks = 0;
    std::cout << std::left << "Note: |x| indicates no entries in directory\n";
    std::cout << std::setw(dirWidth) << "Directory" << std::setw(entryWidth) << "Entry" << std::setw(typeWidth) << "Type";
    std::cout << std::setw(fileLengthWidth) << "Length (Bytes)" << std::endl;
    std::cout << std::string(dirWidth + entryWidth + typeWidth + fileLengthWidth, '-') << std::endl;
    for (unsigned int i = 0; i < numDirs; ++i)
    {
        std::string& dirName = dirNames[i];
        std::vector<EntryInfo>& entries = directoryEntries[i];
        if (entries.empty())
        {
            std::cout << std::setw(dirWidth) << dirName << std::setw(entryWidth) << "|x|" << std::setw(typeWidth) << "|x|" << std::endl;
        }
        else
        {
            for (unsigned int j = 0; j < entries.size(); ++j)
            {
                EntryInfo entry = entries[j];
                if(entry._entryType == 'U')
                {
                    numUserBlocks += entry._numBlocks;
                    freeBlocks -= entry._numBlocks;
                }
                std::cout << std::setw(dirWidth) << (j == 0 ? dirName : "");
                std::cout << std::setw(entryWidth) << entry._entryName;
                std::cout << std::setw(typeWidth) << entry._entryType;
                std::cout << std::setw(fileLengthWidth) << (entry._entryType == 'U' ? std::to_string(entry._fileLength) : "") << std::endl;
            }
        }
    }
    std::cout << std::string(dirWidth + entryWidth + typeWidth + fileLengthWidth, '-') << std::endl;
    std::cout << std::setw(freeBlockWidth) << "Free Block Count";
    std::cout << std::setw(dirCountWidth) << "Directory Block Count";
    std::cout << std::setw(userCountWidth) << "User Data Block Count" << std::endl;
    std::cout << std::setw(freeBlockWidth) << std::to_string(freeBlocks);
    std::cout << std::setw(dirCountWidth) << std::to_string(numDirs);
    std::cout << std::setw(userCountWidth) << std::to_string(numUserBlocks) << std::endl;
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
        std::cout << "File or Directory with same name already exists! Replacing File/Dir." << std::endl;
        DELETE(nameBuffer);
    }

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

        if(writeResult.entry->TYPE == 'U')
        {
            _lastOpened = writeResult.entry;
            _filePointer = 0;
            _fileMode = 'O';            
        }

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
    bool isOpenFile = false;
    if(_lastOpened && parentDir)
    {
        Entry* toDelete = &parentDir->getDir()[entryIndex];
        if(std::strncmp(_lastOpened->NAME, toDelete->NAME, MAX_NAME_LENGTH) == 0)
        {
            isOpenFile = true;
        }
    }
    if(!parentDir) return NO_FILE_FOUND;
    Entry* toDelete = &parentDir->getDir()[entryIndex];
    // Search to see if open file is descendent of directory being deleted
    if(toDelete->TYPE == 'D' && _lastOpened)
    {
        std::queue<unsigned int> dirQueue;
        dirQueue.push(toDelete->LINK);
        while(!dirQueue.empty())
        {
            unsigned int dirBlockNum = dirQueue.front();
            dirQueue.pop();
            DirectoryBlock* dirBlock = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlockNum));
            if(!dirBlock) return CASTING_ERROR;
            for(dirBlock;
                dirBlock != nullptr;
                dirBlock = (dirBlock->getNextBlock() != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlock->getNextBlock())) : nullptr)
            {
                auto dir = dirBlock->getDir();
                for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i) {
                    Entry& e = dir[i];
                    if(e.TYPE == 'D')
                    {
                        dirQueue.push(e.LINK);
                    }
                    else if(e.TYPE == 'U')
                    {
                        if(_lastOpened && std::strncmp(_lastOpened->NAME, e.NAME, MAX_NAME_LENGTH) == 0)
                        {
                            _lastOpened = nullptr;
                            _fileMode = 'I';
                            _filePointer = 0;
                        }
                    }
                }
            }
        }
        
    }
    _diskManager.freeBlock(toDelete->LINK);
    if(isOpenFile)
    {
        _lastOpened = nullptr;
        _fileMode = 'I';
        _filePointer = 0;
    }
    toDelete->TYPE = 'F';
    return SUCCESS;
}

std::pair<STATUS_CODE, std::string> SystemManager::READ(const unsigned int& numBytes)
{
    if(_fileMode != 'U' && _fileMode != 'I') return {BAD_FILE_MODE, "BADFILEMODE"};
    if(!_lastOpened) return {NO_FILE_OPEN, "NOFILEOPEN"};
    std::string readData = "\"";
    UserDataBlock* dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(_lastOpened->LINK));
    if(!dataBlock) return {CASTING_ERROR, "NOLINKTODATABLOCK"};
    if(numBytes == 0) return {SUCCESS, "\"\""};
    unsigned int readBytes = numBytes;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    unsigned int readBlock = _lastOpened->LINK;

    for(int i = 0 ; i < pointerBlock; ++i)
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
        if(dataBlock->getNextBlock() == 0 && readStart + bytesToRead >= bytesInBlock) readData.append("\"\nEnd of File Reached");
        else if(dataBlock->getNextBlock() == 0) readData.append("\"");
        else if(readBytes == 0) readData.append("\"");
        readBlock = dataBlock->getNextBlock();
        (readBlock == 0) ? dataBlock = nullptr : dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(readBlock));
        readStart = 0;
    }
    return {readBuffer.first, readData};
}

STATUS_CODE SystemManager::WRITE(const int& numBytes, const std::string& writeBuffer)
{
    if(_fileMode != 'O' && _fileMode != 'U') return BAD_FILE_MODE;
    if(!_lastOpened) return NO_FILE_OPEN;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    
    unsigned int writeBlock = _lastOpened->LINK;
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

    unsigned int bytesToWrite = numBytes;
    unsigned int bufferStart = 0;
    unsigned int writeStart = pointerOffset;
    unsigned int writeAmount = 0;
    STATUS_CODE writeStatus = SUCCESS;
    while(bytesToWrite > 0)
    {
        if(!currentBlock) return CASTING_ERROR;

        writeAmount = std::min(bytesToWrite, USER_DATA_SIZE - writeStart);
        STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
        if(status != SUCCESS) return status;

        bytesToWrite -= writeAmount;
        bufferStart += writeAmount;
        writeStart = 0;
        if(bytesToWrite > 0)
        {
            if(currentBlock->getNextBlock() == 0)
            {
                // Note: Create function that will auto chain the new block? DWRITE overload?
                auto [allocStatus, newBlock] = _diskManager.allocateBlock('U');
                // Can't allocate new blocks, fit as much of the data as can fit in final block
                if(allocStatus == OUT_OF_MEMORY)
                {
                    writeStatus = OUT_OF_MEMORY;
                    break;
                }
                if(allocStatus != SUCCESS) return allocStatus;
                currentBlock->setNextBlock(newBlock);
                currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(newBlock));
                if(!currentBlock) return CASTING_ERROR;
                currentBlock->setPrevBlock(writeBlock);
                writeBlock = newBlock;
            }
            else
            {
                writeBlock = currentBlock->getNextBlock();
                currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(writeBlock));
                if(!currentBlock) return CASTING_ERROR; 
            }
        }
    }
    if(currentBlock->getNextBlock() == 0) _lastOpened->SIZE = currentBlock->getUserDataSize();
    return writeStatus;
}

STATUS_CODE SystemManager::SEEK(const int& base, const int& offset)
{
    if(_fileMode != 'I' && _fileMode != 'U') return BAD_FILE_MODE;
    if(!_lastOpened) return NO_FILE_OPEN;
    if(base < -1 || base > 1) return BAD_ARG;
    unsigned int numBlocks = _diskManager.countNumBlocks(_lastOpened->LINK);
    unsigned int totalBytes = (numBlocks - 1) * USER_DATA_SIZE;
    int lastByte = (_lastOpened->SIZE == 0 && numBlocks == 1) ? -1 : totalBytes + _lastOpened->SIZE - 1;
    unsigned int startByte = _filePointer;

    if(lastByte == -1)
    {
        _filePointer = 0u;
        return SUCCESS;
    }

    if(base == -1){
        startByte = 0;
    }
    else if(base == 1){
        startByte = lastByte + 1; // Last Byte is EOF
    }
    int seekByte = startByte + offset;

    if(seekByte < 0) seekByte = 0;
    if(static_cast<unsigned int>(seekByte) > lastByte) seekByte = lastByte + 1;

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
    std::vector<std::vector<EntryInfo>> directoryEntries(numDirectories);

    unsigned int index = 0;
    while(!dirOrder.empty())
    {
        unsigned int blockNum = dirOrder.front();
        std::string dirName = dirNames.front();
        dirOrder.pop();
        dirNames.pop();
        directoryNames[index] = dirName;
        std::vector<EntryInfo>& entryList = directoryEntries[index++];
        DirectoryBlock* dirBlock = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(blockNum));
        if(!dirBlock)
        {
            std::cout << "[ERROR]: COULD NOT GET DIRECTORY BLOCK " << blockNum << std::endl;
            continue;
        }

        auto dir = dirBlock->getDir();
        for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i) {
            Entry& e = dir[i];
            if(e.TYPE == 'D')
            {
                entryList.push_back({e.NAME, e.TYPE});
            }
            else if(e.TYPE == 'U')
            {
                unsigned int numBlocks = _diskManager.countNumBlocks(e.LINK);
                unsigned int prevBlocksBytesAllocated = (numBlocks - 1 ) * USER_DATA_SIZE;
                unsigned int lastBlockSize = e.SIZE;
                entryList.push_back({e.NAME, e.TYPE, numBlocks, prevBlocksBytesAllocated + lastBlockSize});
            }
            
        }
    }
    outputFileSystem(directoryNames, directoryEntries);
    return STATUS_CODE::SUCCESS;
}

void SystemManager::SAVE(const std::string& fileName)
{
    std::ofstream saveFile;
    saveFile.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
    if(!saveFile.is_open() || saveFile.fail())
    {
        std::cerr << "[SAVE] Could not open or write to save file: " << fileName << std::endl;
        return;
    }
    _diskManager.DSAVE(saveFile);

    saveFile.flush();
    saveFile.close();

    return;
}

STATUS_CODE SystemManager::LOAD(const std::string& fileName)
{
    std::ifstream loadFile;
    loadFile.open(fileName);
    if(!loadFile.is_open() || loadFile.fail()) return NO_FILE_OPEN;
    return _diskManager.DLOAD(loadFile);
}

char* SystemManager::getFileName()
{
    if(!_lastOpened) return nullptr;
    return _lastOpened->NAME;
}