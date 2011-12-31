cmake_minimum_required(VERSION 2.6)
set(PROJECT_NAME alz)
project(${PROJECT_NAME})

add_definitions(-Wall -Wextra -pthread -g -O4 -funroll-loops)
set(CMAKE_CXX_FLAGS "-std=c++0x")

enable_testing()
add_subdirectory(src)
