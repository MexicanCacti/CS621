class Block{
    protected:
        unsigned int BACK;
        unsigned int FRWD;
        unsigned int FREE;
        char _filler [4];
    public:
        Block(unsigned int next, unsigned int prev) :
            FRWD(next), BACK(prev) {}
        unsigned int getPrevBlock() const {return BACK;}
        unsigned int getNextBlock() const {return FRWD;}
        void setNextBlock(unsigned int next) {FRWD = next;}
        void setPrevBlock(unsigned int prev) {BACK = prev;}
};