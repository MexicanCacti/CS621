#include "block.hpp"

class FileBlock : public Block {
    private:
        char USER_DATA [504];
    public:
        FileBlock(unsigned int next, unsigned int prev) : Block(next, prev) {};
};