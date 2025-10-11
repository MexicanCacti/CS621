#include "../headers/system_manager.hpp"
#include <iostream>

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}

STATUS_CODE SystemManager::CREATE(const char& type, const std::string pathBuffer)
{
    if(type != 'U' && type != 'D') return STATUS_CODE::INVALID_TYPE;
    auto lastSlash = pathBuffer.rfind(PATH_DELIMITER);
    auto fileName = (lastSlash == std::string::npos) ? pathBuffer : pathBuffer.substr(lastSlash + 1);
    
    std::string originalPath = pathBuffer;
    if(fileName.length() > MAX_NAME_LENGTH) return STATUS_CODE::INVALID_NAME;

    SearchResult searchResult = _diskManager.findPath(pathBuffer, fileName);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;
    PathResult pathResult;
    WriteResult writeResult;
    // Found file with same name in last directory of given path
    if(status == STATUS_CODE::SUCCESS)
    {
        pathResult = _diskManager.findMissingPath(originalPath);
        writeResult = _diskManager.DWRITE(parentDir, entryIndex, fileName.c_str(), type, pathResult);
        if(writeResult.status != STATUS_CODE::SUCCESS) return writeResult.status;
        _lastOpened = writeResult.entry;
        _fileMode = 'O';
        return writeResult.status;
    }
    else if(status == STATUS_CODE::NO_FILE_FOUND)
    {
        // No file exists with same name
        // Check how many directories of given path don't exist. Will need to create that many directories
        auto pathResult = _diskManager.findMissingPath(originalPath);
        if(pathResult.status != SUCCESS) return pathResult.status;
        
        auto existingPath = pathResult.existingPath;
        auto missingPath = pathResult.missingPath;
        int numNeededFreeBlocks = missingPath.size();
        if(numNeededFreeBlocks > _diskManager.getNumFreeBlocks()) return STATUS_CODE::OUT_OF_MEMORY;
        writeResult = _diskManager.DWRITE(pathResult, type);

        if(writeResult.status != STATUS_CODE::SUCCESS) return writeResult.status;
        _lastOpened = writeResult.entry;
        _fileMode = 'O';
        return writeResult.status;
    }

    return status;
    
}

STATUS_CODE SystemManager::OPEN(const char& mode, const std::string pathBuffer)
{
    auto lastSlash = pathBuffer.rfind(PATH_DELIMITER);
    auto fileName = (lastSlash == std::string::npos) ? pathBuffer : pathBuffer.substr(lastSlash + 1);
    SearchResult searchResult = _diskManager.findPath(pathBuffer, fileName);
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

STATUS_CODE SystemManager::DELETE(const std::string pathBuffer)
{
    auto lastSlash = pathBuffer.rfind(PATH_DELIMITER);
    auto fileName = (lastSlash == std::string::npos) ? pathBuffer : pathBuffer.substr(lastSlash + 1);
    SearchResult searchResult = _diskManager.findPath(pathBuffer, fileName);
    STATUS_CODE status = searchResult.statusCode;
    DirectoryBlock* parentDir = searchResult.directory;
    unsigned int entryIndex = searchResult.entryIndex;
    if(!parentDir) return STATUS_CODE::NO_FILE_FOUND;
    std::string originalPath = pathBuffer;
    PathResult pathResult = _diskManager.findMissingPath(originalPath);
    return _diskManager.freeEntry(pathResult.existingPath);
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