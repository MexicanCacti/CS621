#include "../headers/directory_block.hpp"
#include <cstring>

STATUS_CODE DirectoryBlock::addEntry(const char* name, const char& type, const unsigned int& entryIndex, const unsigned int& blockNumber)
{
    if(entryIndex >= MAX_DIRECTORY_ENTRIES) return STATUS_CODE::BAD_COMMAND;
    if(DIR[entryIndex].TYPE != 'F') return STATUS_CODE::BAD_COMMAND;
    if(strlen(name) > MAX_NAME_LENGTH) return STATUS_CODE::INVALID_NAME;
    DIR[entryIndex].TYPE = type;
    DIR[entryIndex].SIZE = 0;
    DIR[entryIndex].LINK = blockNumber;
    strncpy(DIR[entryIndex].NAME, name, MAX_NAME_LENGTH);
    DIR[entryIndex].NAME[MAX_NAME_LENGTH] = '\0';
    return STATUS_CODE::SUCCESS;
}