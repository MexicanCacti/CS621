#pragma once
#include "constants.hpp"
#include "../utils/status_codes.hpp"

class Block {
    protected:
        unsigned int BACK = 0;
        unsigned int FRWD = 0;
    public:
        Block(unsigned int prev, unsigned int next) :
            BACK(prev), FRWD(next) {}
        virtual ~Block() = default;
        unsigned int getPrevBlock() const {return BACK;}
        unsigned int getNextBlock() const {return FRWD;}
        void setPrevBlock(unsigned int prev) {BACK = prev;}
        void setNextBlock(unsigned int next) {FRWD = next;}
};