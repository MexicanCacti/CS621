To run tests, run these commands from the root folder [FILE-SYSTEM-PROJECT]

SEEK:
g++ ./tests/seek.cpp ./tests/test_system_manager.hpp ./tests/test_system_manager.cpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o seek.exe

CREATE:
 g++ ./tests/create.cpp ./tests/test_system_manager.hpp ./tests/test_system_manager.cpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o create.exe

DELETE:
 g++ ./tests/delete.cpp ./tests/test_system_manager.hpp ./tests/test_system_manager.cpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o delete.exe

READ-WRITE:
 g++ ./tests/read-write.cpp ./tests/test_system_manager.hpp ./tests/test_system_manager.cpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o read-write.exe

Front-End
g++ -std=c++20 -g -static-libstdc++ -static-libgcc ./sources/front_end.cpp ./sources/disk_manager.cpp ./sources/system_manager.cpp ./sources/directory_block.cpp ./sources/disk_searcher.cpp ./sources/disk_writer.cpp -I./headers -I./sources -I./utils -o front-end.exe

g++ ./sources/front_end.cpp -std=c++20 -g -static-libstdc++ -static-libgcc ./sources/*.cpp -I./headers -I./sources -I./utils -o front-end.exe