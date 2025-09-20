#include "../headers/system_manager.hpp"

Entry* SystemManager::findFile(std::queue<std::string> nameBuffer)
{
    return nullptr;
}

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}
STATUS_CODE SystemManager::CREATE(const char& type, const std::string nameBuffer)
{
    return STATUS_CODE::SUCCESS;
}
STATUS_CODE SystemManager::OPEN(const char& mode, const std::string nameBuffer)
{
    std::queue<std::string> nameBufferQueue;
    const char splitChar = '/';
    std::string bufferCopy = nameBuffer;
    size_t startPos = 0;
    size_t splitPos = 0;
    while(splitPos != std::string::npos){
        splitPos = bufferCopy.find(splitChar, startPos);
        nameBufferQueue.push(bufferCopy.substr(startPos, splitPos - startPos));
        startPos = splitPos + 1;
    }
    nameBufferQueue.push(bufferCopy.substr(startPos, std::string::npos));

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