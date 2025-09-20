#include "../headers/disk_manager.hpp"

void DiskManager::initBlocks()
{
    if(_numBlocks < 1) return;

    _blockMap[0] = new Block(0, 1);
    
    for(int i = 1 ; i < _numBlocks; ++i){
        _blockMap[i] = new Block(i - 1, i + 1);
    }

    _blockMap[_numBlocks] = new Block(_numBlocks - 1, 0);
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
    if(!_blockMap[0]) return 0;
    return _blockMap[0]->getNextBlock();
}

STATUS_CODE DiskManager::allocateBlock(const unsigned int& blockNumber, const char& type)
{
    if(!inBounds(blockNumber)) return ILLEGAL_ACCESS;
    // Guarantee passed in blockNumber is the next available block
    if(getNextFreeBlock() != blockNumber) return ILLEGAL_ACCESS;

    unsigned int nextFreeBlock = _blockMap[blockNumber]->getNextBlock();

    if(_blockMap[blockNumber]) delete _blockMap[blockNumber];

    if(type == 'U'){
        _blockMap[blockNumber] = new FileBlock(0, 0);
    }
    else if(type == 'D'){
        _blockMap[blockNumber] = new DirectoryBlock(0, 0);
    }
    else return BAD_COMMAND;

    _blockMap[0]->setNextBlock(nextFreeBlock);
    
    if(nextFreeBlock != 0){
        getBlock(nextFreeBlock)->setPrevBlock(0);
    }

    return SUCCESS;    
}

/*
    freeBlock() does not delete the block entry. It is only written over when
    the next allocation occurs & the block is the next free block
    Free'd blocks are placed at front of linked-list of free blocks
*/
void DiskManager::freeBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return;

    unsigned int currentBlockNumber = blockNumber;

    Block* rootBlock = getBlock(0);
    while(currentBlockNumber != 0){
        Block* currentBlock = getBlock(currentBlockNumber);
        unsigned int nextFreeBlock = getNextFreeBlock();
        Block* nextFree = _blockMap[nextFreeBlock];
        if(nextFreeBlock != 0){
            nextFree->setPrevBlock(currentBlockNumber);
        }
        nextFree->setPrevBlock(currentBlockNumber);
        rootBlock->setNextBlock(currentBlockNumber);
        currentBlock->setPrevBlock(0);
        currentBlockNumber = currentBlock->getNextBlock();
    }
}

unsigned int DiskManager::countNumBlocks(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return 0;
    unsigned int count = 0;
    unsigned int currentBlockNumber = blockNumber;
    while(currentBlockNumber != 0){
        ++count;
        Block* currentBlock = getBlock(currentBlockNumber);
        currentBlockNumber = currentBlock->getNextBlock();
    }
    return count;
}


std::pair<STATUS_CODE, std::string> DiskManager::DREAD(const unsigned int& blockNumber, const int& bytes)
{
    return {STATUS_CODE::SUCCESS, ""};
}
STATUS_CODE DiskManager::DWRITE(const unsigned int& blockNumber, std::string writeBuffer)
{
    return STATUS_CODE::SUCCESS;
}

DiskManager::~DiskManager()
{
    for(auto& pair : _blockMap){
        delete pair.second;
    }
}