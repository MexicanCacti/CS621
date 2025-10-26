#include "test_system_manager.hpp"
#include <iostream>
#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include <chrono>

unsigned int testsPassed = 0;
unsigned int testsFailed = 0;

void checkEqual(const std::string& testName, STATUS_CODE actual, STATUS_CODE expected) {
    if (actual == expected) {
        std::cout << "[PASS]: " << testName << " got " << statusToString(actual) << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL]: " << testName << " expected " << statusToString(expected);
        std::cout << ", but got " << statusToString(actual) << "\n";
        testsFailed++;
    }
}

void checkEqual(const std::string& testName, unsigned int actual, unsigned int expected) {
    if (actual == expected) {
        std::cout << "[PASS]: " << testName << " got " << actual << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL]: " << testName << " expected " << expected;
        std::cout << ", but got " << actual << "\n";
        testsFailed++;
    }
}

void checkEqual(const std::string& testName, std::string actualName, std::string expectedName){
    if (strcmp(actualName.c_str(), expectedName.c_str()) == 0) {
        std::cout << "[PASS]: " << testName << " got " << actualName << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL]: " << testName << " expected " << expectedName;
        std::cout << ", but got " << actualName << "\n";
        testsFailed++;
    }
}

void checkEqual(const std::string& testName, char actualMode, char expectedMode){
    if (actualMode == expectedMode) {
        std::cout << "[PASS]: " << testName << " got " << actualMode << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL]: " << testName << " expected " << expectedMode;
        std::cout << ", but got " << actualMode << "\n";
        testsFailed++;
    }
}

void printSummary() {
    std::cout << "\n TEST SUMMARY \n";
    std::cout << "Passed: " << testsPassed << "\n";
    std::cout << "Failed: " << testsFailed << "\n";
    std::cout << "Total: " << (testsPassed + testsFailed) << "\n";
}

const int testBlocks = 35;

int main() {
    DiskManager diskManager(testBlocks, BLOCK_SIZE, USER_DATA_SIZE);
    TestSystemManager testSystem(diskManager);
    
    struct createType {
        char type;
        std::string path;
        std::string name;
        unsigned int expectedBlock;
        STATUS_CODE expectedStatus;
    };

    std::vector<createType> initCreates = {
        {'U', "file1", "file1", 1, STATUS_CODE::SUCCESS},
        {'D', "dir1", "dir1", 2, STATUS_CODE::SUCCESS},
        {'D', "dir1/dir2/dir3", "dir3", 4, STATUS_CODE::SUCCESS},
        {'U', "dir1/dir2/file2", "file2", 5, STATUS_CODE::SUCCESS}
    };

    for(auto& test : initCreates) 
    {
        std::cout << "Creating: " << test.path;
        STATUS_CODE result = testSystem.CREATE(test.type, test.path.c_str());
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        std::cout << "nextFreeBlock:" << testSystem.getNextFreeBlock() << std::endl;
    }

    struct testType{
        std::string path;
        unsigned int nextFreeBlock;
        STATUS_CODE expectedStatus;
    };

    std::vector<testType> deleteTests = {
        {"notapath", 6, STATUS_CODE::NO_FILE_FOUND},
        {"dir1/dir2/file2", 5, STATUS_CODE::SUCCESS},
        {"dir1/dir2/file2", 5, STATUS_CODE::NO_FILE_FOUND},
        {"dir1/dir2", 3, STATUS_CODE::SUCCESS},
        {"dir1/dir2", 3, STATUS_CODE::NO_FILE_FOUND},
        {"dir1/dir2/dir3", 3, STATUS_CODE::NO_FILE_FOUND},
        {"dir1/dir3", 3, STATUS_CODE::NO_FILE_FOUND},
        {"file1", 1, STATUS_CODE::SUCCESS}
    };

    for(auto& test : deleteTests) 
    {
        auto startTime = std::chrono::steady_clock::now();
        STATUS_CODE result = testSystem.DELETE(test.path);
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        std::cout << "Test|Path: " << test.path << std::endl;
        std::cout << "Time Taken: " << timeTaken << " Microseconds" << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        if(test.expectedStatus == STATUS_CODE::SUCCESS){
            Entry* entry = testSystem.getEntry();
            checkEqual("Next Free Block Check", testSystem.getNextFreeBlock(), test.nextFreeBlock);
        }

    }

    printSummary();

    return (testsFailed == 0) ? 0 : 1;
}
