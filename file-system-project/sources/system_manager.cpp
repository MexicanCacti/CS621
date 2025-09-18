#include "system_manager.hpp"

FileBlock* SystemManager::findFile(std::queue<std::string> nameBuffer)
{
    
}

SystemManager::SystemManager(DiskManager& diskManager, const std::string& rootName) :
                _diskManager(diskManager)
{
    
}
STATUS_CODE SystemManager::CREATE(const char& type, const std::string nameBuffer)
{

}
STATUS_CODE SystemManager::OPEN(const char& mode, const std::string nameBuffer)
{

}
STATUS_CODE SystemManager::CLOSE()
{

}
STATUS_CODE SystemManager::DELETE(const std::string nameBuffer)
{

}
std::pair<STATUS_CODE, std::string> SystemManager::READ(const int& numBytes)
{

}
STATUS_CODE SystemManager::WRITE(const int& numBytes, const std::string writeBuffer)
{

}
STATUS_CODE SystemManager::SEEK(const int& base, const int& offset)
{

}