#pragma once

class Block{
    protected:
        unsigned int BACK = 0;
        unsigned int FRWD = 0;
        unsigned int FREE = 0;
        char _filler [4] = {0};
    public:
        Block(unsigned int prev, unsigned int next) :
            BACK(prev), FRWD(next) {}
        unsigned int getPrevBlock() const {return BACK;}
        unsigned int getNextBlock() const {return FRWD;}
        void setNextBlock(unsigned int next) {FRWD = next;}
        void setPrevBlock(unsigned int prev) {BACK = prev;}
};