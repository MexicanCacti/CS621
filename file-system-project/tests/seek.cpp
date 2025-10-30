#include "test_system_manager.hpp"
#include <iostream>
#include <cstring>
#include <cassert>
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

void printSummary() {
    std::cout << "\n TEST SUMMARY \n";
    std::cout << "Passed: " << testsPassed << "\n";
    std::cout << "Failed: " << testsFailed << "\n";
    std::cout << "Total: " << (testsPassed + testsFailed) << "\n";
}

int main() {
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    TestSystemManager testSystem(diskManager);
    Entry testFile;
    char testName[10] = {'t','e','s','t'};
    testFile.LINK = 1;
    strcpy(testFile.NAME, testName);
    testFile.TYPE = 'U';
    testFile.SIZE = 20;
    testSystem.setFilePointer(0);

    // Bad Files Modes
    testSystem.setFileMode('O');
    checkEqual("Seek with mode O", testSystem.SEEK(0, 1), STATUS_CODE::BAD_FILE_MODE);

    testSystem.setFileMode('F');
    checkEqual("Seek with mode F", testSystem.SEEK(0, 1), STATUS_CODE::BAD_FILE_MODE);

    testSystem.setFileMode('I');

    checkEqual("Seek with no file open", testSystem.SEEK(0,1), STATUS_CODE::NO_FILE_OPEN);

    testSystem.setEntry(&testFile);

    // Valid File Modes
    testSystem.setFileMode('I');
    checkEqual("Seek with mode I", testSystem.SEEK(0,0), STATUS_CODE::SUCCESS);

    testSystem.setFileMode('U');
    checkEqual("Seek with mode U", testSystem.SEEK(0,0), STATUS_CODE::SUCCESS);

    // SEEK operations
    // {Base, Offset, Expected}
    // Expected = -1 means BAD_COMMAND
    std::vector<std::vector<int>> seekTests = {
        {0, 0, 0},
        {0, 10, 10},
        {0, 10, 20},
        {0, 9, 20},
        {-1, 0, 0},
        {1, -10, 10},
        {0, -10, 0},
        {-1, 0, 0},
        {-1, 10, 10},
        {0, 999999999, 20},
        {1, -9999999, 0},
        {2, 0, -1},
        {-2, 0, -1}
    };
    std::cout << std::endl;
    for(std::vector<int>& test : seekTests) {
        int base = test[0];
        int offset = test[1];
        int expected = test[2];
        auto startTime = std::chrono::steady_clock::now();
        STATUS_CODE result = testSystem.SEEK(base, offset);
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
        std::string seekString = "Seek Operation: BASE = ";
        seekString += std::to_string(base);
        seekString += " OFFSET = ";
        seekString += std::to_string(offset);
        if(base <-1 || base > 1) {
            checkEqual(seekString, result, BAD_ARG);
            std::cout << "Time Taken: " << timeTaken << " Nanoseconds\n" << std::endl;
            continue;
        }

        if(expected == -1) {
            checkEqual(seekString, result, BAD_ARG);
        } 
        else {
            checkEqual(seekString, result, SUCCESS);
            checkEqual("File pointer after seek", testSystem.getFilePointer(), expected);
        }
        std::cout << "Time Taken: " << timeTaken << " Nanoseconds\n" << std::endl;

    }

    printSummary();

    return (testsFailed == 0) ? 0 : 1;
}
