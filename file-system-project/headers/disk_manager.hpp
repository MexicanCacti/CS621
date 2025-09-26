#pragma once
#include "../headers/block.hpp"
#include "../headers/file_block.hpp"
#include "../headers/root_block.hpp"
#include <unordered_map>
#include <string>
#include "../utils/status_codes.hpp"

class DiskManager{
    private:
        int _numBlocks = 0;
        int _blockSize = 0;
        int _userDataSize = 0;
        std::unordered_map<unsigned int, Block*> _blockMap;
        RootBlock* _rootBlock = nullptr;

        DiskManager() = delete;
        void initBlocks();
        bool const inBounds(const int& blockNumber) 
            {return blockNumber > 0 && blockNumber <= _numBlocks;}
    public:
        DiskManager(const int& numBlocks, 
            const int& blockSize, 
            const int& userDataSize);
        Block* const getBlock(const unsigned int& blockNumber);
        int const getBlockCount() { return _numBlocks;}
        int const getBlockSize() { return _blockSize;}
        STATUS_CODE allocateBlock(const char& type);
        void freeBlock(const unsigned int& blockNumber);
        unsigned int countNumBlocks(const unsigned int& blockNumber);
        unsigned int const getLastBlock(const unsigned int& blockNumber);
        std::pair<STATUS_CODE, std::string> DREAD(const unsigned int& blockNumber, const int& bytes);
        STATUS_CODE DWRITE(const unsigned int& blockNumber, std::string writeBuffer);

        ~DiskManager();
};