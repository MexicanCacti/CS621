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
// Maybe have a public function for deleting a block?
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

    while(currentBlockNumber != 0)
    {
        Block* currentBlock = _blockMap[currentBlockNumber];
        unsigned int chainedBlockNumber = currentBlock->getNextBlock();
        // Check if directory -> will have to free its entries as well!
        if(auto* dirBlock = dynamic_cast<DirectoryBlock*>(currentBlock)){
            Entry* entries = dirBlock->getDir();
            for(unsigned int i = 0 ; i < MAX_DIRECTORY_ENTRIES; ++i){
                if(entries[i].TYPE == 'F') continue;
                if(entries[i].LINK != 0){
                    freeBlock(entries[i].LINK);
                    entries[i].TYPE = 'F';
                }
            }
        }
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

SearchResult DiskManager::findFile(std::deque<std::string>& nameBuffer) 
{
    return _diskSearcher->findFile(nameBuffer);
}

// Write any block to disk
STATUS_CODE DiskManager::DWRITE(unsigned int blockNum, Block* blockPtr)
{
    return STATUS_CODE::SUCCESS;
}

// Add/update entry
WriteResult DiskManager::DWRITE(DirectoryBlock* directory, const unsigned int& entryIndex, const char* name, char type)
{
    // NOTE: Need a way to reverse the free if allocation fails!
    freeBlock(directory->getDir()[entryIndex].LINK);
    auto [status, allocatedBlock] = allocateBlock(type);
    if(status != STATUS_CODE::SUCCESS) return {status, nullptr, type};
    return _diskWriter->addEntryToDirectory(directory, entryIndex, name, type, allocatedBlock);
}

// Write user data
STATUS_CODE DiskManager::DWRITE(UserDataBlock* dataBlock, const char* buffer, size_t nBytes)
{
    return STATUS_CODE::SUCCESS;
}

WriteResult DiskManager::DWRITE(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type)
{
    return _diskWriter->createToFile(existingPath, nameBufferQueue, type);
}

DiskManager::~DiskManager()
{
    delete _diskWriter;
    delete _diskSearcher;
    for(auto& pair : _blockMap){
        delete pair.second;
    }
}