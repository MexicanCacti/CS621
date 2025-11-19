#pragma once
#include "../headers/block.hpp"
#include "../headers/directory_block.hpp"
#include "../headers/user_data_block.hpp"
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"
#include <unordered_map>
#include <string>
#include <deque>
#include <unordered_set>

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
        bool const inBounds(const int& blockNumber) {
            return blockNumber >= 0 && blockNumber <= _numBlocks;
        }
        int findFreeEntry(DirectoryBlock* const directory);
    public:
        DiskManager(
            const int& numBlocks, 
            const int& blockSize, 
            const int& userDataSize
        );
        int const getBlockCount() { 
            return _numBlocks;
        }
        int const getBlockSize() { 
            return _blockSize;
        }
        std::pair<STATUS_CODE, unsigned int> allocateBlock(const char& type);
        void freeBlock(const unsigned int& blockNumber);
        unsigned int countNumBlocks(const unsigned int& blockNumber);
        unsigned int const getLastBlock(const unsigned int& blockNumber);
        unsigned int const getNextFreeBlock() {
            return dynamic_cast<DirectoryBlock*>(_blockMap[0])->getFreeBlock();
        }
        unsigned int const getNumFreeBlocks() {return _numFreeBlocks;}
        void DSAVE(std::ofstream& out);
        STATUS_CODE DLOAD(std::ifstream& in);
        Block* DREAD(const unsigned int& blockNumber);
        std::pair<STATUS_CODE, std::string> DREAD(
            const unsigned int& blockNumber, 
            const int& bytes, 
            const int& startByte
        );
        std::pair<STATUS_CODE, std::string> DREAD(
            const unsigned int& blockNumber, 
            const int& bytes
        );
        // Write any block to disk
        STATUS_CODE DWRITE(unsigned int blockNum, Block* blockPtr);
        // Add/update entry
        WriteResult DWRITE(
            DirectoryBlock* directory, 
            const unsigned int& entryIndex, 
            const char* name, char type
        );
        // Write user data
        STATUS_CODE DWRITE(
            UserDataBlock* dataBlock, 
            const char* buffer, 
            unsigned int nBytes, 
            unsigned int startByte, 
            unsigned int bufferStart
        );
        // Write path to file/directory
        WriteResult DWRITE(
            std::deque<std::string>& existingPath, 
            std::deque<std::string>& nameBufferQueue, 
            const char& type
        );
        SearchResult findFile(std::deque<std::string>& nameBuffer);
        ~DiskManager();
    friend class DiskSearcher;
    friend class DiskWriter;
};