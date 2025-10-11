#pragma once
#include "../headers/block.hpp"
#include "../headers/directory_block.hpp"
#include "../headers/user_data_block.hpp"
#include <unordered_map>
#include <string>
#include "../utils/status_codes.hpp"
#include "../utils/search_result.hpp"
#include "../utils/write_result.hpp"
#include "../utils/path_result.hpp"

class DiskSearcher;
class DiskWriter;

class DiskManager{
    private:
        DiskSearcher* _diskSearcher;
        DiskWriter* _diskWriter;
        DiskManager() = delete;
        void initBlocks();
    protected:
        friend class DiskSearcher;
        friend class DiskWriter;
        int _numBlocks = 0;
        int _blockSize = 0;
        int _userDataSize = 0;
        std::unordered_map<unsigned int, Block*> _blockMap;
        std::unordered_map<std::string, unsigned int> _pathMap;
        std::unordered_map<unsigned int, unsigned int> _parentMap;
        unsigned int _numFreeBlocks = 0;

        bool const inBounds(const int& blockNumber) 
            {return blockNumber >= 0 && blockNumber <= _numBlocks;}
        int findFreeEntry(DirectoryBlock* const directory);
        int const getBlockCount() { return _numBlocks;}
        int const getBlockSize() { return _blockSize;}
        unsigned int const getLastBlock(const unsigned int& blockNumber);
        unsigned int const getNextFreeBlock() {return dynamic_cast<DirectoryBlock*>(_blockMap[0])->getFreeBlock();}
        void const setNextFreeBlock(const unsigned int& blockNum) {dynamic_cast<DirectoryBlock*>(_blockMap[0])->setFreeBlock(blockNum);}
    public:
        DiskManager(const int& numBlocks, 
            const int& blockSize, 
            const int& userDataSize);
        std::pair<STATUS_CODE, std::string> DREAD(const unsigned int& blockNumber, const int& bytes);
        STATUS_CODE DWRITE(unsigned int blockNum, Block* blockPtr);        // Write any block to disk
        WriteResult DWRITE(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type, PathResult& pathResult); // Add/update entry
        STATUS_CODE DWRITE(UserDataBlock* dataBlock, const char* buffer, size_t nBytes); // Write user data
        WriteResult DWRITE(PathResult& pathResult, const char& type); // Create needs to create path to created file/dir
        SearchResult findPath(const std::string& pathBuffer, const std::string& fileName);
        PathResult findMissingPath(std::string& originalPath);
        unsigned int const getNumFreeBlocks() {return _numFreeBlocks;}
        unsigned int countNumBlocks(const unsigned int& blockNumber);
        Block* const getBlock(const unsigned int& blockNumber);
        std::pair<STATUS_CODE, unsigned int> allocateBlock(const char& type);
        void freeBlock(const unsigned int& blockNumber);
        STATUS_CODE freeEntry(const std::string& fullPath);
        ~DiskManager();
};