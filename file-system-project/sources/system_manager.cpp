#include "../headers/system_manager.hpp"
#include <iostream>

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

STATUS_CODE SystemManager::CREATE(const char& type, const std::string nameBuffer)
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

    // No file exists with same name
    // Check how many directories of given path don't exist. Will need to create that many directories
    std::deque<std::string> existingPathBufferQueue;
    std::deque<std::string> needToCreate;

    for(auto& component : fullPathCopy) 
    {
        existingPathBufferQueue.push_back(component);
        SearchResult searchResult = _diskManager.findFile(existingPathBufferQueue);
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