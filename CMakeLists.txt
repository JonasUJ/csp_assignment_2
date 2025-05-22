cmake_minimum_required(VERSION 3.10)

project(csp_assignment_2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SOURCES
    src/main.cpp
    src/b_plus_tree.cpp
    src/trie.cpp
)

add_executable(app ${SOURCES})

target_link_libraries(app PRIVATE pthread)