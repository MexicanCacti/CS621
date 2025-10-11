#pragma once
#include <deque>
#include "../utils/search_result.hpp"
#include "../utils/path_result.hpp"

class DiskManager;

class DiskSearcher{
    private:
        DiskManager& _diskManager;

    public:
        DiskSearcher() = delete;
        DiskSearcher(DiskManager& diskManager) : _diskManager(diskManager) {}
        SearchResult const findPath(const std::string& pathBuffer, const std::string& fileName);
        PathResult findMissingPath(std::string& pathBuffer);
};