#pragma once
#include <string>

struct EntryInfo
{
    std::string _entryName;
    char _entryType;
    unsigned int _fileLength;
    EntryInfo();
    EntryInfo(std::string en, char et) : 
        _entryName(en), _entryType(et) {};
    EntryInfo(std::string en, char et, unsigned int fl) :
        _entryName(en), _entryType(et), _fileLength(fl) {};
};