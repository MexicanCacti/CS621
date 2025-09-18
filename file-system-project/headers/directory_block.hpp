#include "block.hpp"

struct Entry {
    char TYPE;
    char NAME [10];
    unsigned int LINK;
    unsigned int SIZE;
};

class DirectoryBlock : public Block {
    private:
        Entry DIR [31];

    public:
        DirectoryBlock(unsigned int next, unsigned int prev) : Block(next, prev) {};
        ~DirectoryBlock();
};