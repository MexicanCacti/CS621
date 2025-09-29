#include "../headers/system_manager.hpp"

std::deque<std::string> SystemManager::tokenizeString(const std::string& str, const char& delim){
    std::deque<std::string> nameBufferQueue;
    std::string bufferCopy = str;
    size_t startPos = 0;
    size_t splitPos = 0;
    while(splitPos != std::string::npos){
        splitPos = bufferCopy.find(delim, startPos);
        nameBufferQueue.push_front(bufferCopy.substr(startPos, splitPos - startPos));
        startPos = splitPos + 1;
    }

    nameBufferQueue.push_front(bufferCopy.substr(startPos, std::string::npos));
    return nameBufferQueue;
}

std::pair<DirectoryBlock*, unsigned int> SystemManager::findFile(std::deque<std::string> nameBuffer, const char& type)
{
    return _rootBlock->findFile(nameBuffer, _diskManager, type);
}

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}

STATUS_CODE SystemManager::CREATE(const char& type, const std::string nameBuffer)
{
    if(type != 'U' && type != 'D') return STATUS_CODE::INVALID_TYPE;
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    std::string fileName = nameBufferQueue.back();
    std::pair<DirectoryBlock*, unsigned int> dir_and_index = findFile(nameBufferQueue, type);
    
    const unsigned int nextFreeBlock = _rootBlock->getNextFreeBlock();
    if(nextFreeBlock == 0) return STATUS_CODE::OUT_OF_MEMORY;

    if(!dir_and_index.first){
       // Search for next free entry in root directory & place 
        STATUS_CODE status = _rootBlock->addEntry(fileName.c_str(), _rootBlock, type, _diskManager);
        if(status != STATUS_CODE::SUCCESS) return status;
    }
    else{
        // Allocate new block, free old block, replace entry info
        _diskManager.allocateBlock(type);
        _diskManager.freeBlock(dir_and_index.first->getEntry(dir_and_index.second)->LINK);
        dir_and_index.first->getEntry(dir_and_index.second)->LINK = nextFreeBlock;
        dir_and_index.first->getEntry(dir_and_index.second)->SIZE = 0;
        dir_and_index.first->getEntry(dir_and_index.second)->TYPE = type;
        strncpy(dir_and_index.first->getEntry(dir_and_index.second)->NAME, fileName.c_str(), MAX_NAME_LENGTH);
        dir_and_index.first->getEntry(dir_and_index.second)->NAME[MAX_NAME_LENGTH] = '\0';
    }
    
    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::OPEN(const char& mode, const std::string nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);

    std::pair<DirectoryBlock*, unsigned int> dir_and_index = findFile(nameBufferQueue, 'U');
    if(!dir_and_index.first) return STATUS_CODE::NO_FILE_FOUND;
    _lastOpened = dir_and_index.first->getEntry(dir_and_index.second);
    _fileMode = mode;
    if(mode == 'I' || mode == 'U') _filePointer = 0;
    else{
        // pointer to last byte in file
        _filePointer = _lastOpened->SIZE - 1;
    }

    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::CLOSE()
{
    _lastOpened = nullptr;

    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::DELETE(const std::string nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    std::pair<DirectoryBlock*, unsigned int> dir_and_index = findFile(nameBufferQueue, 'U');
    if(!dir_and_index.first) return STATUS_CODE::NO_FILE_FOUND;
    Entry* toDelete = dir_and_index.first->getEntry(dir_and_index.second);
    _diskManager.freeBlock(toDelete->LINK);
    toDelete->TYPE = 'F';
    return STATUS_CODE::SUCCESS;
}

std::pair<STATUS_CODE, std::string> SystemManager::READ(const int& numBytes)
{
    return {STATUS_CODE::SUCCESS, ""};
}

STATUS_CODE SystemManager::WRITE(const int& numBytes, const std::string writeBuffer)
{
    return STATUS_CODE::SUCCESS;
}

STATUS_CODE SystemManager::SEEK(const int& base, const int& offset)
{
    if(_fileMode != 'I' && _fileMode != 'U') return STATUS_CODE::BAD_FILE_MODE;
    if(!_lastOpened) return STATUS_CODE::NO_FILE_OPEN;
    if(base < -1 || base > 1) return STATUS_CODE::BAD_COMMAND;
    unsigned int numBlocks = _diskManager.countNumBlocks(_lastOpened->LINK);
    unsigned int totalBytes = numBlocks * 504;

    unsigned int lastByte = totalBytes - (504 - _lastOpened->SIZE);
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