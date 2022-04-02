cmake_minimum_required(VERSION 3.21)
project(MemoryTools)

set(CMAKE_CXX_STANDARD 14)


file(GLOB_RECURSE threadPool ThreadPool/*.cpp)
file(GLOB_RECURSE memoryTools MemoryTools/*.cpp)
file(GLOB_RECURSE http http/*.cpp)

add_library(threadPool SHARED ${threadPool})
add_library(memoryTools SHARED ${memoryTools})
add_library(http SHARED ${http})

#include_directories(/home/wyatt/Tools/libhv/include)

link_directories(/usr/local/lib)

add_executable(MemoryTools main.cpp)

target_link_libraries(MemoryTools threadPool memoryTools)

add_executable(HttpServer HttpServer.cpp)

target_link_libraries(HttpServer threadPool memoryTools http hv)