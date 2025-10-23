#pragma once
#include <queue>
#include <string>
#include "../utils/status_codes.hpp"

struct DirectoryResults
{
    std::queue<unsigned int> directoryOrder;
    std::queue<std::string> directoryName;
    STATUS_CODE status;
};