#pragma once
#include "system_manager.hpp"

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
    return STATUS_CODE::SUCCESS;
}
STATUS_CODE SystemManager::CLOSE()
{
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
    if(!_lastOpened) return STATUS_CODE::NO_FILE_OPEN;
    if(_fileMode != 'I' || _fileMode != 'U' || !_lastOpened) return STATUS_CODE::BAD_FILE_MODE;

    unsigned int numBlocks = _diskManager.countNumBlocks(_lastOpened->LINK);
    unsigned int totalBytes = numBlocks * 504;
    unsigned int lastByte = totalBytes - (504 - _lastOpened->SIZE);
    unsigned int startByte = _filePointer;
    if(base == -1) startByte = 0;
    else if(base == 1) startByte = lastByte;
    int seekByte = startByte + offset;

    if(seekByte < 0 || seekByte >= totalBytes) return STATUS_CODE::ILLEGAL_ACCESS;
    _filePointer = seekByte;
    return STATUS_CODE::SUCCESS;

}