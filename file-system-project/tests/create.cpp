#include "test_system_manager.hpp"
#include <iostream>
#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include "../utils/status_codes_strings.hpp"

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

// NOTE: NEED TO TEST ENTRY OVERFLOW & OUT OF MEMORY STILL!
// NOTE: IF PATH INCLUDES A FILE... SHOULD WE NOT CREATE OR REPLACE FILE WITH DIR?
int main() {
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    TestSystemManager testSystem(diskManager, "testRoot");
    
    struct testType {
        char type;
        std::string path;
        std::string name;
        unsigned int expectedBlock;
        STATUS_CODE expectedStatus;
    };
    // CREATE operations
    // {type, name, expectedBlock, expectedStatus}
    std::vector<testType> createTests = {
        {'U', "file1", "file1", 1, STATUS_CODE::SUCCESS},
        {'D', "file1", "file1", 1, STATUS_CODE::SUCCESS},
        {'D', "dirA/dirB", "dirB", 3, STATUS_CODE::SUCCESS},
        {'U', "dirA", "dirA", 2, STATUS_CODE::SUCCESS},
        {'D', "dirA/dirB/dirC/dirD", "dirD", 5, STATUS_CODE::SUCCESS},
        {'U', "dirA/dirB", "dirB", 3, STATUS_CODE::SUCCESS}
    };

    for(auto& test : createTests) {

        STATUS_CODE result = testSystem.CREATE(test.type, test.path.c_str());
        std::cout << "Test| Type: " << test.type << "\tPath: " << test.path << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        if(test.expectedStatus == STATUS_CODE::SUCCESS){
            Entry* entry = testSystem.getEntry();
            checkEqual("NAME CHECK", std::string(entry->NAME), test.name);
            checkEqual("TYPE CHECK", entry->TYPE, test.type);
            checkEqual("ALLOCATED BLOCK CHECK", entry->LINK, test.expectedBlock);
            checkEqual("MODE CHECK", testSystem.getFileMode(), 'O');
        }
        testSystem.setFileMode('I');

    }

    printSummary();

    return (testsFailed == 0) ? 0 : 1;
}
