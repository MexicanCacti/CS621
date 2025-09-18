#include "block.hpp"
#include "root_block.hpp"
#include "directory_block.hpp"
#include "file_block.hpp"
#include <unordered_map>
#include <queue>
#include "utils/status_codes.cpp"

class DiskManager{
    private:
        int _numBlocks = 0;
        int _blockSize = 0;
        int _userDataSize = 0;
        std::unordered_map<unsigned int, Block*> _blockMap;

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
        unsigned int const getNextFreeBlock();
        STATUS_CODE allocateBlock(const unsigned int& blockNumber, const char& type);
        void freeBlock(const unsigned int& blockNumber);
        ~DiskManager();
};