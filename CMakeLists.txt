cmake_minimum_required(VERSION 3.30)
project(Chip8Interpreter)

set(CMAKE_CXX_STANDARD 23)

add_executable(Chip8Interpreter main.cpp
        main.h)

target_link_libraries(Chip8Interpreter -lncurses)
