#pragma once
#include "../headers/block.hpp"
#include "../headers/directory_block.hpp"
#include "../headers/user_data_block.hpp"
#include "../utils/constants.hpp"
#include <unordered_map>
#include <string>
#include <deque>
#include "../utils/status_codes.hpp"
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"

class DiskSearcher;
class DiskWriter;

class DiskManager{
    private:
        int _numBlocks = 0;
        int _blockSize = 0;
        int _userDataSize = 0;
        std::unordered_map<unsigned int, Block*> _blockMap;
        DiskSearcher* _diskSearcher;
        DiskWriter* _diskWriter;
        unsigned int _numFreeBlocks = 0;


        DiskManager() = delete;
        void initBlocks();
        bool const inBounds(const int& blockNumber) 
            {return blockNumber >= 0 && blockNumber <= _numBlocks;}
        int findFreeEntry(DirectoryBlock* const directory);
    public:
        DiskManager(const int& numBlocks, 
            const int& blockSize, 
            const int& userDataSize);
        int const getBlockCount() { return _numBlocks;}
        int const getBlockSize() { return _blockSize;}
        std::pair<STATUS_CODE, unsigned int> allocateBlock(const char& type);
        void freeBlock(const unsigned int& blockNumber);
        unsigned int countNumBlocks(const unsigned int& blockNumber);
        unsigned int const getLastBlock(const unsigned int& blockNumber);
        unsigned int const getNextFreeBlock() {return dynamic_cast<DirectoryBlock*>(_blockMap[0])->getFreeBlock();}
        unsigned int const getNumFreeBlocks() {return _numFreeBlocks;}
        void const setNextFreeBlock(const unsigned int& blockNum) {dynamic_cast<DirectoryBlock*>(_blockMap[0])->setFreeBlock(blockNum);}
        Block* DREAD(const unsigned int& blockNumber);
        std::pair<STATUS_CODE, std::string> DREAD(const unsigned int& blockNumber, const int& bytes, const int& startByte);
        std::pair<STATUS_CODE, std::string> DREAD(const unsigned int& blockNumber, const int& bytes);
        STATUS_CODE DWRITE(unsigned int blockNum, Block* blockPtr);        // Write any block to disk
        WriteResult DWRITE(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type); // Add/update entry
        STATUS_CODE DWRITE(UserDataBlock* dataBlock, const char* buffer, unsigned int nBytes, unsigned int startByte, unsigned int bufferStart); // Write user data
        WriteResult DWRITE(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type); // Create needs to create path to created file/dir


        SearchResult findFile(std::deque<std::string>& nameBuffer);
        ~DiskManager();
    friend class DiskSearcher;
    friend class DiskWriter;
};