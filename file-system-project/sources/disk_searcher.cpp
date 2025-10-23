#include "../headers/disk_searcher.hpp"
#include "../headers/disk_manager.hpp"

SearchResult const DiskSearcher::findFile(std::deque<std::string>& nameBuffer) {
    if(nameBuffer.empty()) return {BAD_ARG, nullptr, 0};
    
    DirectoryBlock* currentDir = dynamic_cast<DirectoryBlock*>(_diskManager._blockMap.at(0));
    if(!currentDir) return {CASTING_ERROR, nullptr, 0};

    while(!nameBuffer.empty()) {
        std::string currentName = nameBuffer.front();
        if(currentName.length() > MAX_NAME_LENGTH) return {BAD_NAME_LENGTH, nullptr, 0};
        bool found = false;

        for(DirectoryBlock* dir = currentDir; 
            dir != nullptr; 
            dir = (dir->getNextBlock() != 0) ? dynamic_cast<DirectoryBlock*>(_diskManager._blockMap.at(dir->getNextBlock())) : nullptr)
        {
            for(unsigned int i = 0; i < MAX_DIRECTORY_ENTRIES; ++i) {
                Entry& e = dir->getDir()[i];
                if(e.TYPE != 'F' && strncmp(e.NAME, currentName.c_str(), MAX_NAME_LENGTH - 1) == 0) {
                    nameBuffer.pop_front();
                    if(nameBuffer.empty()) return {SUCCESS, dir, i};
                    if(e.TYPE == 'D') {
                        currentDir = dynamic_cast<DirectoryBlock*>(_diskManager._blockMap[e.LINK]);
                        if(!currentDir) return {CASTING_ERROR, nullptr, 0};
                        found = true;
                        break;
                    }
                }
            }
            if(found) break;
        }

        
        if(!found) return {NO_FILE_FOUND, nullptr, 0};
    }

    return {NO_FILE_FOUND, nullptr, 0};
}
