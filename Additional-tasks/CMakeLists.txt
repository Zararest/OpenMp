cmake_minimum_required(VERSION 3.16)

project(Additional-tasks)

find_package(OpenMP)
link_libraries(OpenMP::OpenMP_CXX)
include_directories(../include)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Nested Nested.cpp)
add_executable(CopyIn CopyIn.cpp)
add_executable(CopyPrivate CopyPrivate.cpp)
add_executable(Schedule Schedule.cpp)
add_executable(Sort Merge-sort.cpp)