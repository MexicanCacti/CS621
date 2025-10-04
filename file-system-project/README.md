To run tests, run these commands from the root folder [FILE-SYSTEM-PROJECT]

SEEK:
g++ ./tests/seek.cpp ./tests/test_system_manager.hpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o seek.exe

CREATE:
g++ ./tests/create.cpp ./tests/test_system_manager.hpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o create.exe
