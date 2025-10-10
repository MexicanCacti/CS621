#include "test_system_manager.hpp"

SearchResult TestSystemManager::findCreatedFile(const std::string& filePath)
{
    std::deque<std::string>nameBuffer = tokenizeString(filePath, PATH_DELIMITER);
    return _diskManager.findFile(nameBuffer);
    
}

unsigned int TestSystemManager::getNextFreeBlock()
{
    return dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(0))->getFreeBlock();
}