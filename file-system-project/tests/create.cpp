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
    
    struct testType {
        char type;
        std::string path;
        std::string name;
        unsigned int expectedBlock;
        STATUS_CODE expectedStatus;
    };
    // CREATE operations
    // {type, name, expectedBlock, expectedStatus}
    // TODO: 
    // Add test to check that won't create chain if not enough room for chain + directory
    std::vector<testType> createTests = {
        {'Z', "file1", "file1", 1, STATUS_CODE::BAD_TYPE},
        {'D', "file1", "file1", 1, STATUS_CODE::SUCCESS},
        {'U', "file1", "file1", 1, STATUS_CODE::SUCCESS},
        {'D', "dirA/dirB", "dirB", 3, STATUS_CODE::SUCCESS},
        {'D', "dirA/dirB/dirC", "dirC", 4, STATUS_CODE::SUCCESS},
        {'U', "dirA", "dirA", 2, STATUS_CODE::SUCCESS},
        {'D', "dirA/dirB/dirC/dirD", "dirD", 5, STATUS_CODE::ILLEGAL_ACCESS},
        {'U', "dirA/dirB", "dirB", 5, STATUS_CODE::ILLEGAL_ACCESS},
        {'D', "dirA", "dirA", 2, STATUS_CODE::SUCCESS},
        {'D', "12345678910", "12345678910", 3 , STATUS_CODE::BAD_NAME_LENGTH},
        {'D', "dirA/12345678910", "12345678910", 3, STATUS_CODE::BAD_NAME_LENGTH},
        {'D', "dirB", "dirB", 3, STATUS_CODE::SUCCESS},
        {'D', "dirC", "dirC", 4, STATUS_CODE::SUCCESS},
        {'D', "dirD", "dirD", 5, STATUS_CODE::SUCCESS},
        {'D', "dirE", "dirE", 6, STATUS_CODE::SUCCESS},
        {'D', "dirF", "dirF", 7, STATUS_CODE::SUCCESS},
        {'D', "dirG", "dirG", 8, STATUS_CODE::SUCCESS},
        {'D', "dirH", "dirH", 9, STATUS_CODE::SUCCESS},
        {'D', "dirI", "dirI", 10, STATUS_CODE::SUCCESS},
        {'D', "dirJ", "dirJ", 11, STATUS_CODE::SUCCESS},
        {'D', "dirK", "dirK", 12, STATUS_CODE::SUCCESS},
        {'D', "dirL", "dirL", 13, STATUS_CODE::SUCCESS},
        {'D', "dirM", "dirM", 14, STATUS_CODE::SUCCESS},
        {'D', "dirN", "dirN", 15, STATUS_CODE::SUCCESS},
        {'D', "dirO", "dirO", 16, STATUS_CODE::SUCCESS},
        {'D', "dirP", "dirP", 17, STATUS_CODE::SUCCESS},
        {'D', "dirQ", "dirQ", 18, STATUS_CODE::SUCCESS},
        {'D', "dirR", "dirR", 19, STATUS_CODE::SUCCESS},
        {'D', "dirS", "dirS", 20, STATUS_CODE::SUCCESS},
        {'D', "dirT", "dirT", 21, STATUS_CODE::SUCCESS},
        {'D', "dirU", "dirU", 22, STATUS_CODE::SUCCESS},
        {'D', "dirV", "dirV", 23, STATUS_CODE::SUCCESS},
        {'D', "dirW", "dirW", 24, STATUS_CODE::SUCCESS},
        {'D', "dirX", "dirX", 25, STATUS_CODE::SUCCESS},
        {'D', "dirY", "dirY", 26, STATUS_CODE::SUCCESS},
        {'D', "dirZ", "dirZ", 27, STATUS_CODE::SUCCESS},
        {'D', "dirAZ", "dirAZ", 28, STATUS_CODE::SUCCESS},
        {'D', "dirBZ", "dirBZ", 29, STATUS_CODE::SUCCESS},
        {'D', "dirCZ", "dirCZ", 30, STATUS_CODE::SUCCESS},
        {'D', "dirDZ", "dirDZ", 31, STATUS_CODE::SUCCESS},
        {'D', "dirEZ", "dirEZ", 33, STATUS_CODE::SUCCESS} // Root has max entries, but should be able to create by chaining a dir to root

    };


    for(auto& test : createTests) {
        auto startTime = std::chrono::steady_clock::now();
        STATUS_CODE result = testSystem.CREATE(test.type, test.path.c_str());
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        std::cout << "Test| Type: " << test.type << "\tPath: " << test.path << std::endl;
        std::cout << "Time Taken: " << timeTaken << " Microseconds" << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        if(test.expectedStatus == STATUS_CODE::SUCCESS){
            Entry* entry = testSystem.getEntry();
            checkEqual("NAME CHECK", std::string(entry->NAME), test.name);
            checkEqual("TYPE CHECK", entry->TYPE, test.type);
            checkEqual("ALLOCATED BLOCK CHECK", entry->LINK, test.expectedBlock);
            checkEqual("MODE CHECK", testSystem.getFileMode(), 'O');
            std::cout << "nextFreeBlock:" << testSystem.getNextFreeBlock() << std::endl;
        }
        testSystem.setFileMode('I');

    }

    printSummary();
    testSystem.displayFileSystem();

    return (testsFailed == 0) ? 0 : 1;
}
