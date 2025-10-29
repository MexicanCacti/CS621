#pragma once
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
struct SaveType
{
    std::string _NAME;
    char _TYPE = 'F';
    unsigned int _blockNumber = 0;
    unsigned int _prevBlockNumber = 0;
    unsigned int _nextBlockNumber = 0;
    unsigned int _SIZE = 0;
    std::vector<char> _DATA;
    unsigned int _dataSize = 0;

    SaveType(const char* n, char t, unsigned int bn, unsigned int pn, unsigned int nb) :
        _NAME(n ? n : ""), _TYPE(t), _blockNumber(bn), _prevBlockNumber(pn), _nextBlockNumber(nb)
    {}

    SaveType(const char* n, char t, unsigned int bn, unsigned int pn, unsigned int nb, unsigned int s, const char* d, unsigned int ds) :
        _NAME(n ? n : ""), _TYPE(t), _blockNumber(bn), _prevBlockNumber(pn), _nextBlockNumber(nb), _SIZE(s), _dataSize(ds)
    {
        if (d && ds > 0) {
            _DATA.assign(d, d + ds);
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
        if(tokens.size() >= 5)
        {
            _TYPE = tokens[0][0];
            _blockNumber = std::stoul(tokens[1]);
            _NAME = tokens[2];
            _prevBlockNumber = std::stoul(tokens[3]);
            _nextBlockNumber = std::stoul(tokens[4]);
        }
        // User Data Block
        if(tokens.size() >= 8)
        {
            _SIZE = std::stoul(tokens[5]);
            _dataSize = std::stoul(tokens[7]);
            _DATA.assign(tokens[6].begin(), tokens[6].begin() + _dataSize);
        }
        /*
        std::cout << "Parsed line: [" << fileLine << "]\n";
        std::cout << "Token count: " << tokens.size() << "\n";
        for (size_t i = 0; i < tokens.size(); ++i)
            std::cout << "  Token[" << i << "]: '" << tokens[i] << "'\n";

        std::cout << "TYPE: " << _TYPE << "\n";
        std::cout << "Block#: " << _blockNumber << "\n";
        std::cout << "Name: " << _NAME << "\n";
        std::cout << "Prev: " << _prevBlockNumber << "\n";
        std::cout << "Next: " << _nextBlockNumber << "\n";
        if (tokens.size() == 8)
        {
            std::cout << "Size: " << _SIZE << "\n";
            std::cout << "DataSize: " << _dataSize << "\n";
            std::cout << "Data: '";
            for (size_t i = 0; i < _DATA.size(); ++i)
                std::cout << _DATA[i];
            }
            "\'\n";
        std::cout << "----------------------------------------\n";
        */
    }

    void save(unsigned int tabAmount, std::ofstream& out)
    {
        for(unsigned int i = 0 ; i < tabAmount; ++i)
        {
            out << "\t";
        }
        out << _TYPE << "\t" << _blockNumber << "\t" << (_NAME.empty() ? "Chained Block" : _NAME) << "\t" << _prevBlockNumber << "\t" << _nextBlockNumber;
        if(_TYPE == 'U' || _TYPE == 'C')
        {
            out << "\t" << _SIZE << "\t";
            out.write(_DATA.data(), _DATA.size());
            out << "\t" << _dataSize;
        }
        out << "\n";
    }
};