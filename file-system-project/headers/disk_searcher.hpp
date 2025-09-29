#pragma once
#include "../headers/disk_manager.hpp"

class DiskSearcher{
    private:
        std::unordered_map<unsigned int, Block*>* _blockMap;

    public:
        DiskSearcher() = default;
        DiskSearcher(std::unordered_map<unsigned int, Block*>& blockMap) : _blockMap(&blockMap) {}
        SearchResult const findFile(std::deque<std::string>& nameBuffer);
};