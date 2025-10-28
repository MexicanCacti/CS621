#include "../headers/disk_writer.hpp"
#include "../headers/disk_manager.hpp"
#include <iostream>
STATUS_CODE const DiskWriter::writeToBlock(UserDataBlock* dataBlock, const char* data, const int& bytes, const int& startByte, const unsigned int& bufferStart)
{
    if(!dataBlock || !data || bytes < 0 || startByte < 0) return BAD_ARG;
    
    if(startByte >= _diskManager.getBlockSize()) return BOUNDS_ERROR;

    char* userData = dataBlock->getUserData();
    unsigned int dataLength = dataBlock->getUserDataSize();
    char tempData [USER_DATA_SIZE + 1] = {0};

    unsigned int prevBytes = std::min<unsigned int>(startByte, dataLength);
    for(int i = 0 ; i < prevBytes ; ++i)
    {
        tempData[i] = userData[i];
    }

    unsigned int writeBytes = std::min<unsigned int>(bytes, USER_DATA_SIZE - startByte);
    for(int i = 0 ; i < writeBytes; ++i)
    {
        tempData[i + startByte] = data[i + bufferStart];
    }

    int lastByteIndex = startByte + writeBytes;
    if(lastByteIndex > USER_DATA_SIZE) lastByteIndex = USER_DATA_SIZE;
    tempData[lastByteIndex] = '\0';

    dataBlock->setUserData(tempData);

    return SUCCESS;
}

std::pair<STATUS_CODE, DirectoryBlock*> const DiskWriter::chainDirectoryBlock(DirectoryBlock* const directory)
{
    auto [status, freeBlock] = _diskManager.allocateBlock('D');
    if(status != SUCCESS) return {status, nullptr};

    directory->setNextBlock(freeBlock);

    DirectoryBlock* newChain = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(freeBlock));
    if(!newChain){
        _diskManager.freeBlock(freeBlock);
        directory->setNextBlock(0);
        return {CASTING_ERROR, nullptr};
    }

    return {SUCCESS, newChain};

}

WriteResult const DiskWriter::addEntryToDirectory(DirectoryBlock* const directory, const unsigned int& entryIndex, const char* name, const char& type, const unsigned int& blockNum)
{
    if(entryIndex > MAX_DIRECTORY_ENTRIES) return {BAD_ARG, nullptr, type};
    
    directory->getDir()[entryIndex] = {name, type, blockNum, 0};

    return {SUCCESS, &directory->getDir()[entryIndex], type};
}

/*
    NOTE: already guaranteed to have enough space to allocate enough blocks in nameBufferQueue
    Steps
    1. Find the last directory of the existing path. Guaranteed that last directory is descendent of root if called, otherwise addEntryToDirectory would have been used
    2. Check if the last directory has a free entry
        a. If not, check there is enough memory to chain an additional directory block & have enough blocks for the nameBufferQueue
    3. Now take from the nameBufferQueue to create directories until the end file is reached
    4. Place the end file as an entry in the last parent directory
*/
WriteResult const DiskWriter::createToFile(std::deque<std::string>& existingPath, std::deque<std::string>& nameBufferQueue, const char& type)
{
    DirectoryBlock* directory = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(0));
    // Find last valid directory
    if(!existingPath.empty())
    {
        SearchResult findStartPoint = _diskManager.findFile(existingPath);
        if(findStartPoint.statusCode != SUCCESS) return {findStartPoint.statusCode, nullptr, type};
        Entry startPoint = findStartPoint.directory->getDir()[findStartPoint.entryIndex];
        if(startPoint.TYPE != 'D') return {BAD_TYPE, nullptr, type};
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(startPoint.LINK));
    }

    if(!directory) return {CASTING_ERROR, nullptr, type};

    // Now do a check if it has a free entry, if it doesn't -> need to chain... but ensure that we have enough space for a chain!
    unsigned int nextFreeEntry = directory->findFreeEntry();
    if(nextFreeEntry == MAX_DIRECTORY_ENTRIES){
        if(nameBufferQueue.size() + 1 > _diskManager.getNumFreeBlocks()) return {OUT_OF_MEMORY, nullptr, type};
        auto [chainStatus, chain] = chainDirectoryBlock(directory);
        if(chainStatus != SUCCESS) return {chainStatus, nullptr, type};
        directory = chain;
        nextFreeEntry = directory->findFreeEntry();
    }
    
    while(nameBufferQueue.size() > 1){
        std::string bufferString = nameBufferQueue.front();
        auto [status, freeBlock] = _diskManager.allocateBlock('D');
        if(status != SUCCESS) return {status, nullptr, type};
        addEntryToDirectory(directory, nextFreeEntry, bufferString.c_str(), 'D', freeBlock);
        directory = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(freeBlock));
        if(!directory) return {CASTING_ERROR, nullptr, type};
        nameBufferQueue.pop_front();
        nextFreeEntry = directory->findFreeEntry();
    }

    auto [status, freeBlock] = _diskManager.allocateBlock(type);
    if(status != SUCCESS) return {status, nullptr, type};
    return addEntryToDirectory(directory, nextFreeEntry, nameBufferQueue.back().c_str(), type, freeBlock);
}

void DiskWriter::saveFileSystem(std::ofstream& out)
{
    std::queue<std::pair<unsigned int, char>> blockQueue;
    std::vector<std::vector<SaveType>> blockEntries(NUM_BLOCKS);

    blockQueue.push({0, 'D'});

    while(!blockQueue.empty())
    {
        unsigned int queueSize = blockQueue.size();
        for(unsigned int i = 0 ; i < queueSize; ++i)
        {
            auto[blockNum, type] = blockQueue.front();
            blockQueue.pop();
            if(type == 'D')
            {
                DirectoryBlock* dirBlock = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(blockNum));
                if(!dirBlock) continue;
                for(dirBlock;
                    dirBlock != nullptr;
                    dirBlock = (blockNum != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(dirBlock->getNextBlock())) : nullptr)
                {
                    auto dir = dirBlock->getDir();
                    for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i)
                    {
                        Entry& e = dir[i];
                        if(e.TYPE == 'D')
                        {
                            DirectoryBlock* dEntry = dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(e.LINK));
                            blockEntries[blockNum].push_back(SaveType(e.NAME, e.TYPE, e.LINK, dEntry->getPrevBlock(), dEntry->getNextBlock()));
                            blockQueue.push({e.LINK, e.TYPE});
                        }
                        else if(e.TYPE == 'U')
                        {
                            UserDataBlock* uEntry = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(e.LINK));
                            blockEntries[blockNum].push_back(SaveType(e.NAME, e.TYPE, e.LINK, uEntry->getPrevBlock(), uEntry->getNextBlock(), e.SIZE, uEntry->getUserData(), uEntry->getUserDataSize()));
                            if(uEntry->getNextBlock() != 0) blockQueue.push({e.LINK, e.TYPE}); // Meaning this UserDataBlock has chained blocks
                        }
                    }
                    blockNum = dirBlock->getNextBlock();
                }
            }
            // Get the chained blocks of the base UserDataBlock
            else if(type == 'U')
            {
                UserDataBlock* userBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(blockNum));
                if(!userBlock) continue;
                unsigned int currentBlockNumber = userBlock->getNextBlock();
                while(currentBlockNumber != 0)
                {
                    userBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(currentBlockNumber));
                    if(!userBlock) break;
                    blockEntries[blockNum].push_back(SaveType(nullptr, 'U', currentBlockNumber, userBlock->getPrevBlock(), userBlock->getNextBlock(), 0, userBlock->getUserData(), userBlock->getUserDataSize()));
                    currentBlockNumber = userBlock->getNextBlock();
                }
            }
        }
    }

    for(unsigned int i = 0 ; i < NUM_BLOCKS; ++i)
    {
        out << i << "\n";
        for(SaveType& saveOutput : blockEntries[i])
        {
            saveOutput.save(1, out);
        }
    }
}

void DiskWriter::loadFileSystem(std::ifstream& in)
{
    
}