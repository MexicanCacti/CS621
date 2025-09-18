#include "directory_block.hpp"

class RootBlock : public DirectoryBlock{

    public:
        unsigned int const getNextFreeBlock() {return FREE;}
        int setFreeBlock(const unsigned int& blockNum) {FREE = blockNum;}
};