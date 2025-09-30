#pragma once
#include "../headers/disk_manager.hpp"

class DiskSearcher{
    private:
        DiskManager& _diskManager;

    public:
        DiskSearcher(DiskManager& diskManager) : _diskManager(diskManager) {}
        SearchResult const findFile(std::deque<std::string>& nameBuffer);
};