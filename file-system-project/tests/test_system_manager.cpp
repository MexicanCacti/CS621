#include "test_system_manager.hpp"

SearchResult TestSystemManager::findCreatedFile(const std::string& filePath)
{
    std::deque<std::string>nameBuffer = tokenizeString(filePath, PATH_DELIMITER);
    return _diskManager.findFile(nameBuffer);
    
}