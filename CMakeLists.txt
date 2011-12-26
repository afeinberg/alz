cmake_minimum_required(VERSION 2.6)
set(PROJECT_NAME alz)
project(${PROJECT_NAME})

add_definitions(-Wall -Wextra -pthread -O3 -funroll-loops -DALZ_DEBUG_)
set(CMAKE_CXX_FLAGS "-std=c++0x")

enable_testing()
add_subdirectory(src)
