#pragma once
#include "status_codes.hpp"
#include <string>

std::string statusToString(STATUS_CODE code) {
    switch (code) {
        case STATUS_CODE::SUCCESS:        return "SUCCESS";
        case STATUS_CODE::END_OF_FILE:    return "END_OF_FILE";
        case STATUS_CODE::OUT_OF_MEMORY:  return "OUT_OF_MEMORY";
        case STATUS_CODE::NO_FILE_FOUND:  return "NO_FILE_FOUND";
        case STATUS_CODE::NO_FILE_OPEN:   return "NO_FILE_OPEN";
        case STATUS_CODE::ILLEGAL_ACCESS: return "ILLEGAL_ACCESS";
        case STATUS_CODE::INVALID_TYPE:   return "INVALID_TYPE";
        case STATUS_CODE::INVALID_NAME:   return "INVALID_NAME";
        case STATUS_CODE::BAD_FILE_MODE:  return "BAD_FILE_MODE";
        case STATUS_CODE::BAD_COMMAND:    return "BAD_COMMAND";
        default:                          return "UNKNOWN_STATUS_CODE";
    }
}