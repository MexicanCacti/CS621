#include "test_system_manager.hpp"

SearchResult TestSystemManager::findCreatedFile(const std::string& filePath)
{
    auto lastSlash = filePath.rfind(PATH_DELIMITER);
    std::string fileName = filePath.substr(lastSlash);
    return _diskManager.findPath(filePath, fileName);
}

unsigned int TestSystemManager::getNextFreeBlock()
{
    return dynamic_cast<DirectoryBlock*>(_diskManager.getBlock(0))->getFreeBlock();
}