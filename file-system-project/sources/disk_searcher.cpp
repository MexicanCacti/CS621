#include "../headers/disk_searcher.hpp"
#include "../headers/disk_manager.hpp"

SearchResult const DiskSearcher::findPath(const std::string& pathBuffer, const std::string& fileName) 
{
    if(pathBuffer.empty()) return {BAD_COMMAND, nullptr, 0};

    unsigned int foundBlock = 0;
    unsigned int parentBlock = 0;
    if(_diskManager._pathMap.find(pathBuffer) != _diskManager._pathMap.end())
    {
        foundBlock = _diskManager._pathMap.at(pathBuffer);
        parentBlock = _diskManager._parentMap.at(foundBlock);
    }

    if(foundBlock == 0) return {NO_FILE_FOUND, nullptr, 0};

    DirectoryBlock* currentDir = dynamic_cast<DirectoryBlock*>(_diskManager._blockMap.at(parentBlock));
    if(!currentDir) return {ILLEGAL_ACCESS, nullptr, 0};
    for(currentDir ; currentDir != nullptr ; currentDir = (currentDir->getNextBlock() != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager._blockMap.at(currentDir->getNextBlock())) : nullptr)
    {
        if(!currentDir) return {UNKNOWN_ERROR, nullptr, 0};
        Entry* dir = currentDir->getDir();

        for(unsigned int i = 0 ; i < MAX_DIRECTORY_ENTRIES; ++i)
        {
            Entry& e = dir[i];
            if(e.TYPE != 'F' && strncmp(fileName.c_str(), e.NAME, MAX_NAME_LENGTH - 1) == 0)
            {
                return {SUCCESS, currentDir, i};
            }
        }
    }

    return {NO_FILE_FOUND, nullptr, 0};

}

PathResult DiskSearcher::findMissingPath(std::string& pathBuffer)
{
    std::stack<std::string> missingPath;
    while(!pathBuffer.empty())
    {
        if(_diskManager._pathMap.find(pathBuffer) != _diskManager._pathMap.end())
        {
            auto blockNumber = _diskManager._pathMap.at(pathBuffer);
            if(dynamic_cast<DirectoryBlock*>(_diskManager._blockMap.at(blockNumber)) == nullptr)
            {
                return {ILLEGAL_ACCESS, pathBuffer, missingPath};
            }
            else break;
        }
        auto lastSlash = pathBuffer.rfind(PATH_DELIMITER);
        missingPath.push( (lastSlash == std::string::npos) ? pathBuffer : pathBuffer.substr(lastSlash + 1));
        pathBuffer = (lastSlash == std::string::npos) ? "" : pathBuffer.substr(0, lastSlash);
    }

    
    return {SUCCESS, pathBuffer, missingPath};
}