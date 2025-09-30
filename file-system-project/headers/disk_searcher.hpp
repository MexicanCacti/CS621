#pragma once
#include <deque>
#include "../utils/search_result.hpp"

class DiskManager;

class DiskSearcher{
    private:
        DiskManager& _diskManager;

    public:
        DiskSearcher() = delete;
        DiskSearcher(DiskManager& diskManager) : _diskManager(diskManager) {}
        SearchResult const findFile(std::deque<std::string>& nameBuffer);
};