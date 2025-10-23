#pragma once
#include "../utils/constants.hpp"
#include <cstring>

struct Entry {
    char TYPE = 'F'; // F for file, D for Directory, U for user data file
    char NAME[MAX_NAME_LENGTH + 1];
    unsigned int LINK = 0;
    unsigned int SIZE = 0;
    Entry() = default;
    Entry(const char* name, const char& type, const unsigned int& link, const unsigned int& size) :
        TYPE(type), LINK(link), SIZE(size) {
            strncpy(NAME, name, MAX_NAME_LENGTH);
            NAME[MAX_NAME_LENGTH] = '\0';
        };
};