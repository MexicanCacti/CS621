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
    _diskSearcher = DiskSearcher(_blockMap);
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
    if(_blockMap[0]->getNextBlock() == 0) return STATUS_CODE::OUT_OF_MEMORY;

    // Take free block out of list
    unsigned int nextFreeBlock = _blockMap[0]->getNextBlock();
    _blockMap[0]->setNextBlock(_blockMap[nextFreeBlock]->getNextBlock());
    _blockMap[_blockMap[0]->getNextBlock()]->setPrevBlock(0);

    if(_blockMap[nextFreeBlock]) delete _blockMap[nextFreeBlock];

    if(type == 'U'){
        _blockMap[nextFreeBlock] = new UserDataBlock(0, 0);
    }
    else if(type == 'D'){
        _blockMap[nextFreeBlock] = new DirectoryBlock(0, 0);
    }
    else{
        // Put freeBlock back into list
        _blockMap[nextFreeBlock]->setNextBlock(_blockMap[0]->getNextBlock());
        _blockMap[_blockMap[0]->getNextBlock()]->setPrevBlock(nextFreeBlock);
        _blockMap[0]->setNextBlock(nextFreeBlock);
        _blockMap[nextFreeBlock]->setPrevBlock(0);
        return STATUS_CODE::BAD_COMMAND;
    }
    dynamic_cast<DirectoryBlock*>(_blockMap[0])->setFreeBlock(_blockMap[0]->getNextBlock());
    return STATUS_CODE::SUCCESS;
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