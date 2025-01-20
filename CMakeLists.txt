cmake_minimum_required(VERSION 3.27)
project(assignment3)

set(CMAKE_CXX_STANDARD 17)

add_executable(assignment3 main.cpp
        PageTable.cpp
        TLB.cpp)
