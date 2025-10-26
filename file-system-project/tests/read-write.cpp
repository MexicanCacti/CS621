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

void checkEqual(const std::string& testName, std::string actualString, std::string expectedString){
    if (strcmp(actualString.c_str(), expectedString.c_str()) == 0) {
        std::cout << "[PASS]: " << testName << " got " << actualString << "\n";
        testsPassed++;
    } else {
        std::cout << "[FAIL]: " << testName << " expected\n" << expectedString;
        std::cout << "\nbut got\n" << actualString << "\n";
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
        unsigned int numBytes;
        std::string writeBuffer;
        STATUS_CODE expectedStatus;
    };
    // CREATE operations
    // {type, name, expectedBlock, expectedStatus}
    // TODO: 
    // Add test to check that won't create chain if not enough room for chain + directory
    // UPDATE: READS WILL NOW OUTPUT MESSAGE WHEN EOF IS REACHED
    testSystem.CREATE('U', "file1");
    testSystem.CREATE('U', "dir1/file2");

    testSystem.OPEN('U', "file1");

    std::vector<testType> writeTests = {
        {
            108,
            "This file contains exactly one hundred bytes of readable text used for a basic write test sample entry here.",
            STATUS_CODE::SUCCESS
        },
        {
            496,
            "This text block contains six hundred bytes of content used to verify multi-block write operations in the virtual disk system. "
            "It repeats sentences to ensure precise byte length. Each sentence adds predictable characters. The goal is to simulate a long "
            "user file stored across several chained blocks. Consistency matters more than meaning in this test, but the text remains human "
            "readable and grammatically coherent for debugging purposes. Additional filler ensures we hit the exact byte mark now.",
            STATUS_CODE::SUCCESS
        },
        {
            58,
            "Fifty bytes of short data used for quick write tests only.",
            STATUS_CODE::SUCCESS
        }
    };

    std::vector<testType> readTests = {
        {
            108, 
            "This file contains exactly one hundred bytes of readable text used for a basic write test sample entry here.", 
            STATUS_CODE::SUCCESS,
        },
        {
            496,   
            "This text block contains six hundred bytes of content used to verify multi-block write operations in the virtual disk system. "
            "It repeats sentences to ensure precise byte length. Each sentence adds predictable characters. The goal is to simulate a long "
            "user file stored across several chained blocks. Consistency matters more than meaning in this test, but the text remains human "
            "readable and grammatically coherent for debugging purposes. Additional filler ensures we hit the exact byte mark now.", 
            STATUS_CODE::SUCCESS
        },
        {
            58, 
            "Fifty bytes of short data used for quick write tests only.",
            STATUS_CODE::SUCCESS
        }
    };

    for(auto& test : writeTests) {
        std::cout << "Test| NumBytes: " << test.writeBuffer.size() << "\tWriteBuffer: " << test.writeBuffer << std::endl;
        auto startTime = std::chrono::steady_clock::now();
        STATUS_CODE result = testSystem.WRITE(test.writeBuffer.size(), test.writeBuffer);
        auto endTime = std::chrono::steady_clock::now();
        auto timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        std::cout << "Time Taken: " << timeTaken << " Microseconds" << std::endl;
        checkEqual("STATUS CHECK", result, test.expectedStatus);
        testSystem.SEEK(0, test.numBytes);
    }
    printSummary();

    testSystem.SEEK(-1,0);
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
        testSystem.SEEK(0, test.numBytes);
    }

    std::string entireFile =
        "This file contains exactly one hundred bytes of readable text used for a basic write test sample entry here."
        "This text block contains six hundred bytes of content used to verify multi-block write operations in the virtual disk system. "
        "It repeats sentences to ensure precise byte length. Each sentence adds predictable characters. The goal is to simulate a long "
        "user file stored across several chained blocks. Consistency matters more than meaning in this test, but the text remains human "
        "readable and grammatically coherent for debugging purposes. Additional filler ensures we hit the exact byte mark now."
        "Fifty bytes of short data used for quick write tests only.";

    std::cout << "\nENTIRE FILE CHECK" << std::endl;
    auto [readAllResult, readAllBuffer] = testSystem.READALL();
    checkEqual("STATUS CHECK", readAllResult, STATUS_CODE::SUCCESS);
    if(readAllResult == STATUS_CODE::SUCCESS){
        checkEqual("READ BUFFER CHECK", readAllBuffer, entireFile);
    }
    printSummary();
}
