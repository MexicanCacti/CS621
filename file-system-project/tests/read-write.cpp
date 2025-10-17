#include "test_system_manager.hpp"
#include <iostream>
#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include <chrono>
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

const int testBlocks = 35;

int main() {
    DiskManager diskManager(testBlocks, BLOCK_SIZE, USER_DATA_SIZE);
    TestSystemManager testSystem(diskManager, "testRoot");
    
    struct testType {
        unsigned int numBytes;
        std::string writeBuffer;
        STATUS_CODE expectedStatus;
    };
    // CREATE operations
    // {type, name, expectedBlock, expectedStatus}
    // TODO: 
    // Add test to check that won't create chain if not enough room for chain + directory
    testSystem.CREATE('U', "file1");
    testSystem.CREATE('U', "dir1/file2");

    testSystem.OPEN('U', "file1");

    std::vector<testType> writeTests = {
        {100, "Hello, World! This is a test string to write to the file.", STATUS_CODE::SUCCESS},
        {600, "This is a longer test string that exceeds the user data size of a single block. It should span multiple blocks in the file system. Let's add more text to ensure it goes over the limit. Adding even more text to make sure we have enough data to test the multi-block writing functionality properly.", STATUS_CODE::SUCCESS},
        {50, "Short write.", STATUS_CODE::SUCCESS}
    };

    std::vector<testType> readTests = {
        {100, "Hello, World! This is a test string to write to the file.", STATUS_CODE::SUCCESS},
        {700, "Hello World! This is a test string to write to the file. This is a longer test string that exceeds the user data size of a single block. It should span multiple blocks in the file system. Let's add more text to ensure it goes over the limit. Adding even more text to make sure we have enough data to test the multi-block writing functionality properly.", STATUS_CODE::SUCCESS},
        {750, "Hello World! This is a test string to write to the file. This is a longer test string that exceeds the user data size of a single block. It should span multiple blocks in the file system. Let's add more text to ensure it goes over the limit. Adding even more text to make sure we have enough data to test the multi-block writing functionality properly.Short write.", STATUS_CODE::SUCCESS}
    };

    for(auto& test : writeTests) {
        auto startTime = std::chrono::steady_clock::now();
        STATUS_CODE result = testSystem.WRITE(test.numBytes, test.writeBuffer);
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        std::cout << "Test| NumBytes: " << test.numBytes << "\tWriteBuffer: " << test.writeBuffer << std::endl;
        std::cout << "Time Taken: " << timeTaken << " Microseconds" << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
    }
    printSummary();

    std::cout << "FileType: " << testSystem.getEntry()->TYPE << std::endl;
    testsPassed = 0;
    testsFailed = 0;
    for(auto& test : readTests) {
        auto startTime = std::chrono::steady_clock::now();
        auto [result, readBuffer] = testSystem.READ(test.numBytes);
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        std::cout << "Test| NumBytes: " << test.numBytes << "\tReadBuffer: " << readBuffer << std::endl;
        std::cout << "Time Taken: " << timeTaken << " Microseconds" << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        if(result == STATUS_CODE::SUCCESS){
            checkEqual("READ BUFFER CHECK", readBuffer, test.writeBuffer);
        }
    }
    printSummary();
}
