#pragma once
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

struct SaveType
{
    char* _NAME = nullptr;
    char _TYPE;
    unsigned int _blockNumber;
    unsigned int _prevBlockNumber;
    unsigned int _nextBlockNumber;
    unsigned int _SIZE;
    char* _DATA = nullptr;
    unsigned int _dataSize;
    SaveType(char* n, char t, unsigned int bn, unsigned int pn, unsigned int nb) :
        _NAME(n), _TYPE(t), _blockNumber(bn), _prevBlockNumber(pn), _nextBlockNumber(nb)
    {
        if(n) _NAME = strdup(n);
    }
    SaveType(char* n, char t, unsigned int bn, unsigned int pn, unsigned int nb, unsigned int s, char* d, unsigned int ds) :
        _NAME(n), _TYPE(t), _blockNumber(bn), _prevBlockNumber(pn), _nextBlockNumber(nb),
        _SIZE(s), _DATA(d), _dataSize(ds)
    {
        if(n) _NAME = strdup(n);
        if(d && ds > 0)
        {
            _DATA = (char*)malloc(ds);
            memcpy(_DATA, d, ds);
        }
    }
    SaveType(std::string& fileLine)
    {
        std::vector<std::string> tokens;
        size_t startPos = 0;
        size_t splitPos = 0;
        char delim = '\t';
        while(splitPos != std::string::npos)
        {
            splitPos = fileLine.find(delim, startPos);
            tokens.push_back(fileLine.substr(startPos, splitPos - startPos));
            startPos = splitPos + 1;
        }
        // Directory Block
        if(tokens.size() == 5)
        {
            _NAME = strdup(tokens[0].c_str());
            _TYPE = tokens[1][0];
            _blockNumber = std::stoul(tokens[2]);
            _prevBlockNumber = std::stoul(tokens[3]);
            _nextBlockNumber = std::stoul(tokens[4]);
        }
        // User Data Block
        if(tokens.size() == 8)
        {
            _SIZE = std::stoul(tokens[5]);
            _dataSize = std::stoul(tokens[7]);
            _DATA = (char*) malloc(_dataSize);
            memcpy(_DATA, tokens[6].data(), _dataSize);
        }

    }
    void save(unsigned int tabAmount, std::ofstream& out)
    {
        for(unsigned int i = 0 ; i < tabAmount; ++i)
        {
            out << "\t";
        }
        out << _TYPE << "\t" << _blockNumber << "\t" << (_NAME ? _NAME : "Chained Block") << "\t" << _prevBlockNumber << "\t" << _nextBlockNumber;
        if(_TYPE == 'U')
        {
            out << "\t" << _SIZE << "\t";
            out.write(_DATA, _dataSize);
            out << "\0\t" << _dataSize;
        }
        out << "\n";
    }

    ~SaveType()
    {
        if(_NAME) free(_NAME);
        if(_DATA) free(_DATA);
    }
};