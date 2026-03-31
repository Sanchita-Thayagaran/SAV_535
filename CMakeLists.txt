cmake_minimum_required(VERSION 3.16)
project(CS535Demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CS535Demo
    src/main.cpp
    src/Memory.cpp
    src/Cache.cpp
    src/Simulator.cpp
    src/Parser.cpp
    src/Instruction.cpp
    src/UI.cpp
)

target_include_directories(CS535Demo PRIVATE src)