cmake_minimum_required(VERSION 3.10)

project(csp_assignment_2)

set(CMAKE_CXX_FLAGS "-std=c++2a -O3")

set(SOURCES
    src/main.cpp
    src/b_plus_tree.cpp
    src/trie.cpp
)

add_executable(app ${SOURCES})
