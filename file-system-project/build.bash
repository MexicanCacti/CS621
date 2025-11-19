#!/bin/bash

CXX="g++"
FLAGS="-std=c++20 -g -static-libstdc++ -static-libgcc"
INCLUDES="-I./headers -I./sources -I./utils"

SRC_COMMON="./sources/disk_manager.cpp \
            ./sources/system_manager.cpp \
            ./sources/directory_block.cpp \
            ./sources/disk_searcher.cpp \
            ./sources/disk_writer.cpp"

TEST_COMMON="./tests/test_system_manager.hpp ./tests/test_system_manager.cpp"

# Build Tests

echo "Building SEEK test..."
$CXX $FLAGS ./tests/seek.cpp $TEST_COMMON $SRC_COMMON $INCLUDES -o seek.exe

echo "Building CREATE test..."
$CXX $FLAGS ./tests/create.cpp $TEST_COMMON $SRC_COMMON $INCLUDES -o create.exe

echo "Building DELETE test..."
$CXX $FLAGS ./tests/delete.cpp $TEST_COMMON $SRC_COMMON $INCLUDES -o delete.exe

echo "Building READ-WRITE test..."
$CXX $FLAGS ./tests/read-write.cpp $TEST_COMMON $SRC_COMMON $INCLUDES -o read-write.exe

# Build Main Program


echo "Building filesystem.exe..."
$CXX $FLAGS ./sources/front_end.cpp $SRC_COMMON $INCLUDES -o filesystem.exe

echo "Done!"
