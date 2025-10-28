#pragma once
#include <string>

struct EntryInfo
{
    std::string _entryName;
    char _entryType;
    unsigned int _numBlocks;
    unsigned int _fileLength;
    EntryInfo();
    EntryInfo(std::string en, char et) : 
        _entryName(en), _entryType(et) {};
    EntryInfo(std::string en, char et, unsigned int nb) :
        _entryName(en), _entryType(et), _numBlocks(nb) {};
    EntryInfo(std::string en, char et, unsigned int nb, unsigned int fl) :
        _entryName(en), _entryType(et), _numBlocks(nb), _fileLength(fl) {};
};