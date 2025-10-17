#pragma once
#include "block.hpp"
class UserDataBlock : public Block {
    private:
        char USER_DATA [USER_DATA_SIZE] = {0};
    public:
        UserDataBlock(unsigned int prev, unsigned int next) : Block(prev, next) {};
        char* const getUserData() {return USER_DATA;}
        unsigned int const getUserDataSize() {return strlen(USER_DATA);}
        void setUserData(const char* data) {strncpy(USER_DATA, data, USER_DATA_SIZE);}
};