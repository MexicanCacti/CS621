#include "../headers/disk_manager.hpp"
#include "../headers/disk_searcher.hpp"
#include "../headers/disk_writer.hpp"
#include <iostream>

void DiskManager::initBlocks()
{
    if(_numBlocks < 1) return;

    _blockMap[0] = new DirectoryBlock(0, 1);

    for(int i = 1 ; i < _numBlocks; ++i){
        _blockMap[i] = new Block(i - 1, i + 1);
    }

    _blockMap[_numBlocks] = new Block(_numBlocks - 1, 0);
    _numFreeBlocks = _numBlocks - 1;
    DirectoryBlock* root = dynamic_cast<DirectoryBlock*>(_blockMap[0]);
    root->setFreeBlock(1);

}

DiskManager::DiskManager(const int& numBlocks, 
                const int& blockSize, 
                const int& userDataSize)
    : _numBlocks(numBlocks),
      _blockSize(blockSize),
      _userDataSize(userDataSize),
      _diskSearcher(nullptr),
      _diskWriter(nullptr)
{
    initBlocks();
    _diskSearcher = new DiskSearcher(*this);
    _diskWriter = new DiskWriter(*this);
}

// NOTE: MAYBE NOT NEEDED
Block* const DiskManager::getBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return nullptr;

    return _blockMap[blockNumber];
}

int DiskManager::findFreeEntry(DirectoryBlock* const directory)
{
    Entry* entries = directory->getDir();
    for(unsigned int i = 0 ; i < MAX_DIRECTORY_ENTRIES; ++i){
        if(entries[i].TYPE == 'F') return i;
    }

    return -1;
}

// Note, whenever allocate a block number, always free

std::pair<STATUS_CODE, unsigned int> DiskManager::allocateBlock(const char& type)
{
    DirectoryBlock* rootBlock = dynamic_cast<DirectoryBlock*>(_blockMap[0]);
    if(!rootBlock) return {STATUS_CODE::UNKNOWN_ERROR, 0};
    if(type != 'U' && type != 'D') return {STATUS_CODE::INVALID_TYPE, 0};

    unsigned int freeBlockNumber = rootBlock->getFreeBlock();
    if(freeBlockNumber == 0) return {STATUS_CODE::OUT_OF_MEMORY, 0};

    Block* freeBlock = _blockMap[freeBlockNumber];

    //std::cout << "[DEBUG] allocateBlock requested type=" << type << " freeHead=" << rootBlock->getFreeBlock() << "\n";

    unsigned int nextFreeBlockNumber = freeBlock->getNextBlock();
    rootBlock->setFreeBlock(nextFreeBlockNumber);
    if(nextFreeBlockNumber != 0) _blockMap[nextFreeBlockNumber]->setPrevBlock(0);

    freeBlock->setPrevBlock(0);

    if(type == 'U'){
        _blockMap[freeBlockNumber] = new UserDataBlock(0, 0);
    }
    else if(type == 'D'){
        _blockMap[freeBlockNumber] = new DirectoryBlock(0, 0);
    }
    
    --_numFreeBlocks;
    //std::cout << "[DEBUG] allocateBlock returning block=" << freeBlockNumber << " nextFree=" << rootBlock->getFreeBlock() << "\n";
    return {STATUS_CODE::SUCCESS, freeBlockNumber};
}


STATUS_CODE DiskManager::freeEntry(const std::string& fullPath)
{
    using freeEntry = std::pair<unsigned int, std::string>;
    unsigned int blockNumber = _pathMap.at(fullPath);
    std::stack<freeEntry> toFreeList;
    std::queue<freeEntry> freeQueue;
    freeQueue.push({blockNumber, fullPath});

    while(!freeQueue.empty())
    {
        freeEntry toFree = freeQueue.front();
        unsigned int freeBlock = toFree.first;
        std::string path = toFree.second;
        freeQueue.pop();
        DirectoryBlock* currentDir = dynamic_cast<DirectoryBlock*>(_blockMap.at(freeBlock));
        UserDataBlock* currentFile = dynamic_cast<UserDataBlock*>(_blockMap.at(freeBlock));
        if(currentDir)
        {
            do
            {
                toFreeList.push({freeBlock, path});
                Entry* dir = currentDir->getDir();
                for(unsigned int i = 0 ; i < MAX_DIRECTORY_ENTRIES; ++i)
                {
                    auto entry = &dir[i];
                    if(entry->TYPE != 'F') freeQueue.push({entry->LINK, path + "/" + entry->NAME});
                }
            }
            while(currentDir->getNextBlock() != 0);
 
        }
        else if(currentFile)
        {
            toFreeList.push({freeBlock, path});
            while(currentFile->getNextBlock() != 0){
                freeBlock = currentFile->getNextBlock();
                currentFile = dynamic_cast<UserDataBlock*>(_blockMap.at(freeBlock));
                if(!currentFile) return UNKNOWN_ERROR;
                freeQueue.push({freeBlock, ""});
            }
        }
        else return UNKNOWN_ERROR;
    }

    while(!toFreeList.empty())
    {

        freeEntry toFree = toFreeList.top();
        toFreeList.pop();
        unsigned int toFreeBlock = toFree.first;
        std::string toFreePath = toFree.second;

        freeBlock(toFreeBlock);
        if(!toFreePath.empty())
        {
            _pathMap.erase(toFreePath);
            _parentMap.erase(toFreeBlock);
        }
    }

    return SUCCESS;
}

/*
    freeBlock() does not delete the block entry. It is only written over when
    the next allocation occurs & the block is the next free block
    Free'd blocks are placed at front of linked-list of free blocks
    ONLY called when deleting a chained directory block or a chained user data block
*/
void DiskManager::freeBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber) || blockNumber == 0) return;

    unsigned int currentBlockNumber = blockNumber;
    DirectoryBlock* rootBlock = dynamic_cast<DirectoryBlock*>(_blockMap[0]);
    Block* currentBlock = _blockMap[currentBlockNumber];
    if(!rootBlock) return;

    unsigned int oldFreeNumber = rootBlock->getFreeBlock();
    // std::cout << "[DEBUG] freeBlock freeing block=" << currentBlockNumber << " chainedNext=" << chainedBlockNumber << " oldFreeHead=" << oldFreeNumber << "\n";
    currentBlock->setNextBlock(oldFreeNumber);
    currentBlock->setPrevBlock(0);

    if(oldFreeNumber != 0){
        Block* oldFreeBlock = _blockMap[oldFreeNumber];
        oldFreeBlock->setPrevBlock(currentBlockNumber);
    }
    ++_numFreeBlocks;
    rootBlock->setFreeBlock(currentBlockNumber);
    // std::cout << "[DEBUG] freeBlock newFreeHead=" << rootBlock->getFreeBlock() << " numFree=" << _numFreeBlocks << "\n";
    
}

unsigned int DiskManager::countNumBlocks(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return 0;
    unsigned int count = 1;
    unsigned int lastBlockNumber = blockNumber;
    Block* currentBlock = getBlock(lastBlockNumber);
    
    while(currentBlock->getNextBlock() != 0){
        lastBlockNumber = currentBlock->getNextBlock();
        currentBlock = getBlock(lastBlockNumber);
        ++count;
    }
    return count;
}

unsigned int const DiskManager::getLastBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return 0;
    unsigned int lastBlockNumber = blockNumber;
    Block* currentBlock = getBlock(lastBlockNumber);
    
    while(currentBlock->getNextBlock() != 0){
        lastBlockNumber = currentBlock->getNextBlock();
        currentBlock = getBlock(lastBlockNumber);
    }
    return lastBlockNumber;
}

SearchResult DiskManager::findPath(const std::string& pathBuffer, const std::string& fileName) 
{
    return _diskSearcher->findPath(pathBuffer, fileName);
}

PathResult DiskManager::findMissingPath(std::string& pathBuffer)
{
    return _diskSearcher->findMissingPath(pathBuffer);
}

// Write any block to disk
STATUS_CODE DiskManager::DWRITE(unsigned int blockNum, Block* blockPtr)
{
    return STATUS_CODE::SUCCESS;
}

// Add/update entry
WriteResult DiskManager::DWRITE(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type, PathResult& pathResult)
{
    return _diskWriter->createFile(directory, entryIndex, name, type, pathResult);
}

// Write user data
STATUS_CODE DiskManager::DWRITE(UserDataBlock* dataBlock, const char* buffer, size_t nBytes)
{
    return STATUS_CODE::SUCCESS;
}

WriteResult DiskManager::DWRITE(PathResult& pathResult, const char& type)
{
    return _diskWriter->createToFile(pathResult, type);
}

DiskManager::~DiskManager()
{
    delete _diskWriter;
    delete _diskSearcher;
    for(auto& pair : _blockMap){
        delete pair.second;
    }
}