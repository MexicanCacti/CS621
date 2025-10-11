#pragma once
#include "status_codes.hpp"
#include "string"
#include "stack"

struct PathResult{
    STATUS_CODE status;
    std::string existingPath;
    std::stack<std::string> missingPath;
};