#include "../headers/disk_writer.hpp"

STATUS_CODE const DiskWriter::writeToBlock(const unsigned int& blockNumber, const char* data, const int& bytes)
{
    return STATUS_CODE::SUCCESS;
}

STATUS_CODE const DiskWriter::chainDirectoryBlock(DirectoryBlock* const directory, const unsigned int& newBlockNumber)
{
    return STATUS_CODE::SUCCESS;
}

STATUS_CODE const DiskWriter::addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type)
{
    return STATUS_CODE::SUCCESS;
}