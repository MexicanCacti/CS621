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
    : _numBlocks(numBlocks),
      _blockSize(blockSize),
      _userDataSize(userDataSize),
      _diskSearcher(*this),
      _diskWriter(*this)
{
    initBlocks();
}

// NOTE: MAYBE NOT NEEDED
Block* const DiskManager::getBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber)) return nullptr;

    return _blockMap[blockNumber];
}

int findFreeEntry(DirectoryBlock* const directory)
{
    Entry* entries = directory->getDir();
    for(unsigned int i = 0 ; i < MAX_DIRECTORY_ENTRIES; ++i){
        if(entries[i].TYPE == 'F') return i;
    }

    return -1;
}

// Note, whenever allocate a block number, always free
// Maybe have a public function for deleting a block?
std::pair<STATUS_CODE, unsigned int> DiskManager::allocateBlock(const char& type)
{
    DirectoryBlock* rootBlock = dynamic_cast<DirectoryBlock*>(_blockMap[0]);
    if(!rootBlock) return {STATUS_CODE::UNKNOWN_ERROR, 0};
    if(type != 'U' && type != 'D') return {STATUS_CODE::INVALID_TYPE, 0};

    unsigned int freeBlockNumber = rootBlock->getFreeBlock();
    if(freeBlockNumber == 0) return {STATUS_CODE::OUT_OF_MEMORY, 0};

    Block* freeBlock = _blockMap[freeBlockNumber];

    unsigned int nextFreeBlockNumber = freeBlock->getNextBlock();
    rootBlock->setFreeBlock(nextFreeBlockNumber);
    if(nextFreeBlockNumber != 0) _blockMap[nextFreeBlockNumber]->setPrevBlock(0);

    freeBlock->setNextBlock(0);
    freeBlock->setPrevBlock(0);

    if(type == 'U'){
        _blockMap[freeBlockNumber] = new UserDataBlock(0, 0);
    }
    else if(type == 'D'){
        _blockMap[freeBlockNumber] = new DirectoryBlock(0, 0);
    }
    
    return {STATUS_CODE::SUCCESS, freeBlockNumber};
}

/*
    freeBlock() does not delete the block entry. It is only written over when
    the next allocation occurs & the block is the next free block
    Free'd blocks are placed at front of linked-list of free blocks
*/
void DiskManager::freeBlock(const unsigned int& blockNumber)
{
    if(!inBounds(blockNumber) || blockNumber == 0) return;

    unsigned int currentBlockNumber = blockNumber;
    DirectoryBlock* rootBlock = dynamic_cast<DirectoryBlock*>(_blockMap[0]);
    if(!rootBlock) return;

    while(currentBlockNumber != 0){
        Block* currentBlock = _blockMap[blockNumber];
        unsigned int chainedBlockNumber = currentBlock->getNextBlock();

        unsigned int oldFreeNumber = rootBlock->getFreeBlock();
        currentBlock->setNextBlock(oldFreeNumber);
        currentBlock->setPrevBlock(0);

        if(oldFreeNumber != 0){
            Block* oldFreeBlock = _blockMap[oldFreeNumber];
            oldFreeBlock->setPrevBlock(currentBlockNumber);
        }

        rootBlock->setFreeBlock(currentBlockNumber);
        currentBlockNumber = chainedBlockNumber;
    }
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

// Write any block to disk
STATUS_CODE DWRITE(unsigned int blockNum, Block* blockPtr)
{
    
}

// Add/update entry
STATUS_CODE DWRITE(DirectoryBlock* directory, unsigned int entryIndex, const char* name, char type)
{

}

// Write user data
STATUS_CODE DWRITE(UserDataBlock* dataBlock, const char* buffer, size_t nBytes)
{

}


DiskManager::~DiskManager()
{
    for(auto& pair : _blockMap){
        delete pair.second;
    }
}