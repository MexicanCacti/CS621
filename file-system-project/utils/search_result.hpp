#pragma once
#include "status_codes.hpp"
#include "directory_block.hpp"
// Searching gives: { STATUS_CODE of the search, { DirectoryBlock pointer, index in DIR array } }
struct SearchResult {
    STATUS_CODE statusCode;
    DirectoryBlock* directory;
    unsigned int entryIndex;
};