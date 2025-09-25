#include "../headers/system_manager.hpp"

std::deque<std::string> tokenizeString(const std::string& str, const char& delim){
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
    return nameBufferQueue
}

Entry* SystemManager::findFile(std::deque<std::string> nameBuffer)
{
    return _rootBlock->findFile(nameBuffer, _diskManager);
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
    Entry* file = findFile(nameBufferQueue);
    // NOTE IF NAMES ARE THE SAME BUT TYPES DIFFERENT SHOULD REPLACE WITH DIFFERENT TYPE? OR CREATE NEW ENTRY?
    if(file){
        Block* storageBlock = _diskManager.getBlock(file->LINK);
    }

    return STATUS_CODE::SUCCESS;
}
STATUS_CODE SystemManager::OPEN(const char& mode, const std::string nameBuffer)
{
    std::deque<std::string> nameBufferQueue = tokenizeString(nameBuffer, PATH_DELIMITER);

    Entry* file = findFile(nameBufferQueue);
    if(!file) return STATUS_CODE::NO_FILE_FOUND;

    return STATUS_CODE::SUCCESS;
}
STATUS_CODE SystemManager::CLOSE()
{
    _lastOpened = nullptr;

    return STATUS_CODE::SUCCESS;
}
STATUS_CODE SystemManager::DELETE(const std::string nameBuffer)
{
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
    if(seekByte > lastByte) seekByte = lastByte;

    _filePointer = seekByte;
    return STATUS_CODE::SUCCESS;

}