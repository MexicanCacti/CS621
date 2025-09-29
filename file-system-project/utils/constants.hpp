#pragma once
#include "../utils/status_codes.hpp"

const unsigned int NUM_BLOCKS = 100;
const unsigned int BLOCK_SIZE = 512;
const unsigned int USER_DATA_SIZE = 504;
const unsigned int MAX_DIRECTORY_ENTRIES = 31;
const unsigned int FILLER_AMOUNT = 4;
const unsigned int MAX_NAME_LENGTH = 9;
const char PATH_DELIMITER = '/';

// Searching gives: { STATUS_CODE of the search, { DirectoryBlock pointer, index in DIR array } }
using SearchResult = std::pair<STATUS_CODE, std::pair<DirectoryBlock*, unsigned int>>;