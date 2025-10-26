#include "test_system_manager.hpp"
#include <cmath>

SearchResult TestSystemManager::findCreatedFile(const std::string& filePath)
{
    std::deque<std::string>nameBuffer = tokenizeString(filePath, PATH_DELIMITER);
    return _diskManager.findFile(nameBuffer);
    
}

unsigned int TestSystemManager::getNextFreeBlock()
{
    return dynamic_cast<DirectoryBlock*>(_diskManager.DREAD(0))->getFreeBlock();
}

// Uncomment below to enable debug WRITE & READ
/*
STATUS_CODE TestSystemManager::WRITE(const int& numBytes, const std::string& writeBuffer)
{
    std::cout << "----------------------------In TestSystemManager WRITE----------------------------\n";
    // In DWRITE, we should write starting from the file pointer.
    if(_fileMode != 'O' && _fileMode != 'U') return STATUS_CODE::BAD_FILE_MODE;
    if(!_lastOpened) return STATUS_CODE::NO_FILE_OPEN;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    
    unsigned int writeBlock = _lastOpened->LINK;
    unsigned int writeStart = pointerOffset;
    UserDataBlock* currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(writeBlock));
    if(!currentBlock) return STATUS_CODE::ILLEGAL_ACCESS;

    for(unsigned int i = 0 ; i < pointerBlock; ++i)
    {
        writeBlock = currentBlock->getNextBlock();
        if(writeBlock == 0) return STATUS_CODE::ILLEGAL_ACCESS;
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(writeBlock));
        if(!currentBlock) return STATUS_CODE::ILLEGAL_ACCESS;
    }

    std::string writeData = writeBuffer;
    if(writeData.size() < numBytes)
    {
        std::cout << "WRITE BUFFER TOO SMALL, PADDING WITH SPACES\n";
        std::cout << "WRITE DATA SIZE: " << writeData.size() << " NUM BYTES: " << numBytes << std::endl;
        unsigned int excessBytes = numBytes - writeData.size();
        writeData.append(excessBytes, ' ');
    }

    unsigned int bytesUpToPointer = pointerBlock * USER_DATA_SIZE + pointerOffset;
    unsigned int totalBytesAllocated = _diskManager.countNumBlocks(_lastOpened->LINK) * USER_DATA_SIZE;
    unsigned int freeBytes = totalBytesAllocated - bytesUpToPointer;
    unsigned int bytesToWrite = numBytes;
    unsigned int bufferStart = 0;

    std::cout << "FREEBYTES: " << freeBytes << "\tBYTES TO WRITE: " << bytesToWrite << std::endl;
    if(bytesToWrite <= freeBytes)
    {
        while(currentBlock && bytesToWrite > 0)
        {
            std::cout << "WRITING TO BLOCK: " << writeBlock << " AT OFFSET: " << writeStart << std::endl;
            unsigned int writeAmount = std::min(USER_DATA_SIZE, bytesToWrite);
            STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
            if(status != STATUS_CODE::SUCCESS) return status;
            std::cout << "WROTE: " << writeData.substr(bufferStart, writeAmount) << std::endl;
            bytesToWrite = bytesToWrite - writeAmount;
            bufferStart += writeAmount;
            if(currentBlock->getNextBlock() == 0)
            {
                _lastOpened->SIZE = std::max(_lastOpened->SIZE, writeAmount);
                break;
            }
            currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(currentBlock->getNextBlock()));
            writeStart = 0;
        }
        return STATUS_CODE::SUCCESS;
    }

    unsigned int blocksNeeded = ceil((bytesToWrite - freeBytes) / USER_DATA_SIZE);
    unsigned int numFreeBlocks = _diskManager.getNumFreeBlocks();
    if(numFreeBlocks == 0) return STATUS_CODE::OUT_OF_MEMORY;

    while(blocksNeeded > numFreeBlocks){
        blocksNeeded--;
        bytesToWrite -= USER_DATA_SIZE;
    }

    // Fill Last Block
    std::cout << "WRITING TO BLOCK: " << writeBlock << " AT OFFSET: " << writeStart << std::endl;
    unsigned int writeAmount = freeBytes;
    STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
    if(status != STATUS_CODE::SUCCESS) return status;
    std::cout << "WROTE: " << writeData.substr(bufferStart, writeAmount) << std::endl;
    bytesToWrite -= writeAmount;
    bufferStart += writeAmount;
    writeStart = 0;

    // Now Allocate & fill
    while(currentBlock && bytesToWrite > 0)
    {   
        // Note: Create function that will auto chain the new block? DWRITE overload?
        auto [allocStatus, newBlock] = _diskManager.allocateBlock('U');
        if(allocStatus != STATUS_CODE::SUCCESS) return allocStatus;
        currentBlock->setNextBlock(newBlock);
        currentBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(newBlock));
        if(!currentBlock) return STATUS_CODE::UNKNOWN_ERROR;
        currentBlock->setPrevBlock(writeBlock);
        writeBlock = newBlock;

        writeAmount = std::min(USER_DATA_SIZE, bytesToWrite);
        std::cout << "WRITING TO BLOCK: " << writeBlock << " AT OFFSET: " << writeStart << std::endl;
        STATUS_CODE status = _diskManager.DWRITE(currentBlock, writeData.c_str(), writeAmount, writeStart, bufferStart);
        std::cout << "WROTE: " << writeData.substr(bufferStart, writeAmount) << std::endl;
        if(status != STATUS_CODE::SUCCESS) return status;
        bytesToWrite = bytesToWrite - writeAmount;
        bufferStart += writeAmount;
    }
    std::cout << "----------------------------WRITE COMPLETE----------------------------\n\n";
    return STATUS_CODE::SUCCESS;
}

std::pair<STATUS_CODE, std::string> TestSystemManager::READ(const unsigned int& numBytes)
{
    std::cout << "----------------------------In TestSystemManager READ----------------------------\n";
    if(_fileMode != 'U') return {STATUS_CODE::BAD_FILE_MODE, "BADFILEMODE"};
    if(!_lastOpened) return {STATUS_CODE::NO_FILE_OPEN, "NOFILEOPEN"};

    std::string readData = "";
    UserDataBlock* dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(_lastOpened->LINK));
    if(!dataBlock) return {STATUS_CODE::ILLEGAL_ACCESS, "NOLINKTODATABLOCK"};

    unsigned int readBytes = numBytes;
    std::cout << "[TOTAL BYTES TO READ]: " << readBytes << std::endl;
    std::cout << "Number of blocks in file: " << _diskManager.countNumBlocks(_lastOpened->LINK) << std::endl;
    unsigned int pointerBlock = _filePointer / USER_DATA_SIZE + 1;
    unsigned int pointerOffset = _filePointer % USER_DATA_SIZE;
    std::cout << "FILE POINTER AT: " << _filePointer << std::endl;
    std::cout << "Pointer Block: " << pointerBlock << " Pointer Offset: " << pointerOffset << std::endl;
    unsigned int readBlock = _lastOpened->LINK;

    for(int i = 0 ; i < pointerBlock - 1; ++i)
    {
        readBlock = dataBlock->getNextBlock();
        if(readBlock == 0) return {STATUS_CODE::ILLEGAL_ACCESS, "POINTEROUTOFBOUNDS"};
        Block* nextBlock = _diskManager.DREAD(dataBlock->getNextBlock());
        if(!nextBlock) return {STATUS_CODE::ILLEGAL_ACCESS, "NEXTDATABLOCKNULL"};
        dataBlock = dynamic_cast<UserDataBlock*>(nextBlock);
    }
    unsigned int readStart = pointerOffset;
    while(dataBlock && readBytes > 0)
    {
        unsigned int bytesInBlock = dataBlock->getUserDataSize();
        std::cout << "[BYTES IN BLOCK]: " << bytesInBlock << std::endl;
        unsigned int bytesToRead = std::min(readBytes, bytesInBlock - readStart);
        std::cout << "READING FROM BLOCK: " << readBlock << " FROM BYTE: " << readStart << std::endl;
        std::cout << "[READING]: " << bytesToRead << std::endl;
        auto[status, readBuffer] = _diskManager.DREAD(readBlock, bytesToRead, readStart);
        if(status != STATUS_CODE::SUCCESS) return {status, readBuffer};
        std::cout << "Read Buffer:" << readBuffer << std::endl;
        readData.append(readBuffer);
        readBytes -= bytesToRead;
        readBlock = dataBlock->getNextBlock();
        std::cout << "[BYTES READ]: " << bytesToRead << "\t[BYTES REMAINING]: " << readBytes << std::endl;
        if(readBytes <= 0 || readBlock == 0) break;
        (readBlock == 0) ? dataBlock = nullptr : dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(readBlock));
        readStart = 0;
    }
    std::cout << "----------------------------READ COMPLETE-----------------------\n\n";
    return {STATUS_CODE::SUCCESS, readData};
}
*/

std::pair<STATUS_CODE, std::string> TestSystemManager::READALL()
{
    std::cout << "----------------------------In TestSystemManager READALL-----------------------\n";
    if(_fileMode != 'U') return {BAD_FILE_MODE, "BADFILEMODE"};
    if(!_lastOpened) return {NO_FILE_OPEN, "NOFILEOPEN"};
    std::string readData = "";
    UserDataBlock* dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(_lastOpened->LINK));
    if(!dataBlock) return {CASTING_ERROR, "NOLINKTODATABLOCK"};
    std::cout << "Number of blocks in file: " << _diskManager.countNumBlocks(_lastOpened->LINK) << std::endl;

    unsigned int readBlock = _lastOpened->LINK;
    while(dataBlock)
    {
        std::cout << "[READING FROM BLOCK]: " << readBlock << std::endl;
        unsigned int bytesInBlock = dataBlock->getUserDataSize();
        auto [status, readBuffer] = _diskManager.DREAD(readBlock, bytesInBlock, 0);
        if(status != SUCCESS) return {status, readBuffer};
        std::cout << "Read Buffer: " << readBuffer << std::endl;
        readData.append(readBuffer);
        readBlock = dataBlock->getNextBlock();
        (readBlock == 0) ? dataBlock = nullptr : dataBlock = dynamic_cast<UserDataBlock*>(_diskManager.DREAD(readBlock));
    }

    std::cout << "----------------------------READALL COMPLETE-----------------------\n\n";
    return {SUCCESS, readData};
}