#include "test_system_manager.cpp"
#include <iostream>
#include <assert.h>

int main(){
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    TestSystemManager testSystem(diskManager, "testRoot");
    Entry testFile;
    char testName [10] = {'t','e','s','t'};
    testFile.LINK = 1;
    testFile.NAME = testName;
    testFile.TYPE = 'U';
    testFile.SIZE = 20;
    testSystem.setFilePointer(0);

    try{
        assert(testSystem.SEEK(0,1) == STATUS_CODE::NO_FILE_OPEN);
        std::cout << "No File Open Test Passed\n";
    }
    catch(...){
        std::cout << "No File Open Test Failed\n";
    }

    testSystem.setEntry(&testFile);

    diskManager.allocateBlock(1, 'U');

    try{
        testSystem.setFileMode('O');
        assert(testSystem.SEEK(0, 1) == STATUS_CODE::BAD_FILE_MODE);
        testSystem.setFileMode('F');
        assert(testSystem.SEEK(0, 1) == STATUS_CODE::BAD_FILE_MODE);
        std::cout << "Bad File Mode Tests Passed\n";
    }
    catch(...){
        std::cout << "Bad File Mode Tests Failed\n";
    }
    
    testSystem.setFileMode('I');

    try{
        testSystem.setFileMode('I');
        assert(testSystem.SEEK(0,0) == STATUS_CODE::SUCCESS);
        testSystem.setFileMode('U');
        assert(testSystem.SEEK(0,0) == STATUS_CODE::SUCCESS);
        std::cout << "Valid File Mode Tests Passed\n";
    }
    catch(...){
        std::cout << "Valid File Mode Tests Failed\n";
    }

    try{
        assert(testSystem.SEEK(0, 5) == STATUS_CODE::SUCCESS);
        assert(testSystem.getFilePointer() == 5);
        assert(testSystem.SEEK(-1, 5) == STATUS_CODE::SUCCESS);
        assert(testSystem.getFilePointer() == 5);
        assert(testSystem.SEEK(1, 0) == STATUS_CODE::SUCCESS);
        assert(testSystem.getFilePointer() == 20);
        std::cout << "Seek Operation Tests Passed\n";
    }
    catch(...){
        std::cout << "Seek Operation Tests Failed\n";
    }

    std::cout << "All Tests Ran!\n";
    
    
    return 0;
}