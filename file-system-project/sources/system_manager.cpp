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

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}

STATUS_CODE SystemManager::CREATE(const char& type, const std::string nameBuffer)
{
    if(type != 'U' && type != 'D') return STATUS_CODE::INVALID_TYPE;
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    std::string fileName = nameBufferQueue.back();
    SearchResult searchResult = _diskManager.findFile(nameBufferQueue);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    // Found file with same name in last directory of given path
    if(status == SUCCESS){
        _diskManager.freeBlock(parentDir->getDir()[entryIndex].LINK);
        auto [status, allocatedBlock] = _diskManager.allocateBlock(type);
        if(status != STATUS_CODE::SUCCESS) return status;
        return _diskManager.DWRITE(parentDir, entryIndex, fileName.c_str(), type, allocatedBlock);
    }

    // No file exists with same name
    // Check how many directories of given path don't exist. Will need to create that many directories
    int numNeededFreeBlocks = nameBufferQueue.size();
    if(numNeededFreeBlocks > _diskManager.getNumFreeBlocks()) return STATUS_CODE::OUT_OF_MEMORY;

    std::deque<std::string> existingPathBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);
    int pathLength = existingPathBufferQueue.size();
    while(existingPathBufferQueue.size() != pathLength - nameBufferQueue.size()) existingPathBufferQueue.pop_back();
    return _diskManager.DWRITE(existingPathBufferQueue, nameBufferQueue, type);
}

STATUS_CODE SystemManager::OPEN(const char& mode, const std::string nameBuffer)
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
        // So then when lastOpened is written to @ last block, need to update SIZE for all previously linked blocks!
        _filePointer = _diskManager.countNumBlocks(_lastOpened->LINK) * 504 - (504 - _lastOpened->SIZE);
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
    SearchResult searchResult = _diskManager.findFile(nameBufferQueue);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;

    if(!parentDir) return STATUS_CODE::NO_FILE_FOUND;
    Entry* toDelete = &parentDir->getDir()[entryIndex];
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