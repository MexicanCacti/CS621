#pragma once
#include <string>
#include "status_codes.hpp"
struct InputResult{
    std::string _command;
    std::string _stringArg;
    int _intArg1;
    int _intArg2;
    char _charArg;
    STATUS_CODE _status;
    std::string _errorMessage;
    InputResult() = default;
    InputResult(STATUS_CODE status, std::string em) : 
        _status(status), _errorMessage(em) {};
    InputResult(std::string c, std::string sa, int ia1, int ia2, char ca, STATUS_CODE st, std::string em) :
        _command(c), _stringArg(sa), _intArg1(ia1), _intArg2(ia2), _charArg(ca), _status(st), _errorMessage(em) {};
};