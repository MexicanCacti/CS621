#pragma once
#include "../headers/block.hpp"
#include "../utils/status_codes.hpp"
#include "../headers/directory_block.hpp"

struct WriteResult{
    STATUS_CODE status;
    Entry* entry;
    char type;
};