#include "../headers/disk_searcher.hpp"

SearchResult const DiskSearcher::findFile(std::deque<std::string>& nameBuffer)
{
    if(nameBuffer.empty()) return SearchResult(BAD_COMMAND, nullptr, 0);
    std::string currentName = nameBuffer.front();
    nameBuffer.pop_front();
    DirectoryBlock* searchDirectory = dynamic_cast<DirectoryBlock*>(_blockMap->at(0));
    Entry* entries = searchDirectory->getDir();

    for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
        if(entries[i].TYPE == 'F') continue;

        if(strncmp(entries[i].NAME, currentName.c_str(), 9) == 0){
            if(nameBuffer.empty()) return SearchResult(SUCCESS, searchDirectory, i);
            if(entries[i].TYPE != 'D') continue;
            unsigned int nextSearchDirectory = entries[i].LINK;
            searchDirectory = dynamic_cast<DirectoryBlock*>((*_blockMap)[nextSearchDirectory]);
            if(!searchDirectory) return SearchResult(ILLEGAL_ACCESS, nullptr, 0);
            return searchDirectory->findFile(nameBuffer);
        }
    }

    while(searchDirectory->getNextBlock() != 0){
        searchDirectory = dynamic_cast<DirectoryBlock*>((*_blockMap)[searchDirectory->getNextBlock()]);
        if(!searchDirectory) return SearchResult(ILLEGAL_ACCESS, nullptr, 0);
        entries = searchDirectory->getDir();
        for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i){
            if(entries[i].TYPE == 'F') continue;

            if(strncmp(entries[i].NAME, currentName.c_str(), 9) == 0){
                if(nameBuffer.empty()) return SearchResult(SUCCESS, searchDirectory, i);
                if(entries[i].TYPE != 'D') continue;
                unsigned int nextSearchDirectory = entries[i].LINK;
                searchDirectory = dynamic_cast<DirectoryBlock*>((*_blockMap)[nextSearchDirectory]);
                if(!searchDirectory) return SearchResult(ILLEGAL_ACCESS, nullptr, 0);
                return searchDirectory->findFile(nameBuffer);
            }
        }
    }

    return SearchResult(NO_FILE_FOUND, nullptr, 0);
}