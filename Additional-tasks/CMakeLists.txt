cmake_minimum_required(VERSION 3.16)

project(Additional-tasks)

find_package(OpenMP)
link_libraries(OpenMP::OpenMP_CXX)
include_directories(../include)

add_executable(Nested Nested.cpp)
add_executable(CopyIn CopyIn.cpp)
add_executable(CopyPrivate CopyPrivate.cpp)