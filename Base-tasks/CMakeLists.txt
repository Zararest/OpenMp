cmake_minimum_required(VERSION 3.16)

project(Base-tasks)

find_package(OpenMP)
link_libraries(OpenMP::OpenMP_CXX)
include_directories(../include)

add_executable(Task1 Task1.cpp)
add_executable(Task2 Task2.cpp)
