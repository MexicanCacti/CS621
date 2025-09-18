#include "disk_manager.hpp"

/*
    Helper Function to create a chained linked-list of free blocks
    Also initializes the queue pool for free blocks
*/
void DiskManager::initBlocks()
{
    _blockMap[0] = new Block(0, 0);
    
    for(int i = 1 ; i <= _numBlocks; ++i){
        _blockMap[i] = new Block(i-1, 0);
        _blockMap[i-1]->setNextBlock(i);
    }

}

DiskManager::DiskManager(const int& numBlocks, 
                const int& blockSize, 
                const int& userDataSize)
{
    _numBlocks = numBlocks;
    _blockSize = blockSize;
    _userDataSize = userDataSize;
    initBlocks();
}

Block* const DiskManager::getBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return nullptr;

    return _blockMap[blockNumber];
}

unsigned int const DiskManager::getNextFreeBlock()
{ 
    Block* rootBlock = getBlock(0);
    if(!rootBlock) return 0;

    unsigned int freeBlockNumber = rootBlock->getNextBlock();

    if(freeBlockNumber == 0) return 0;

    Block* freeBlock = getBlock(freeBlockNumber);
    unsigned int nextFreeBlock = freeBlock->getNextBlock();
    rootBlock->setNextBlock(nextFreeBlock);
    
    if(nextFreeBlock != 0){
        getBlock(nextFreeBlock)->setPrevBlock(0);
    }

    return freeBlockNumber;
}

STATUS_CODE DiskManager::allocateBlock(const unsigned int& blockNumber, const char& type)
{
    if(!inBounds(blockNumber)) return ILLEGAL_ACCESS;

    if(type == 'U'){
        delete _blockMap[blockNumber];
        _blockMap[blockNumber] = new FileBlock(0, 0);
    }
    else if(type == 'D'){
        delete _blockMap[blockNumber];
        _blockMap[blockNumber] = new DirectoryBlock(0, 0);
    }
    else return BAD_COMMAND;

    return SUCCESS;    
}

/*
    freeBlock() does not delete the block entry. It is only written over when
    the free'd block number is the first in the queue and a directory or user
    file needs to be allocated
*/
void DiskManager::freeBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return;

    unsigned int currentBlockNumber = blockNumber;

    Block* rootBlock = getBlock(0);
    while(currentBlockNumber != 0){
        Block* freedBlock = getBlock(currentBlockNumber);
        unsigned int nextFreeBlock = getNextFreeBlock();
        Block* nextFree = _blockMap[nextFreeBlock];
        if(nextFreeBlock != 0){
            
        }
        else{
            nextFree->setPrevBlock(currentBlockNumber);
            rootBlock->setNextBlock(currentBlockNumber);
            freedBlock->setPrevBlock(0);
        }
    }
}

DiskManager::~DiskManager()
{
    for(auto& pair : _blockMap){
        delete pair.second;
    }
}