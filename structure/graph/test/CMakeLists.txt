cmake_minimum_required(VERSION 3.14)
project(gtl)

set(CMAKE_CXX_STANDARD 11)

include_directories(../include)

add_executable(gtl-test-directed ./src/directed.cc)
add_executable(gtl-test-undirected ./src/undirected.cc)