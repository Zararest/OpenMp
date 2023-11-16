cmake_minimum_required(VERSION 3.0.0)
project(OpenCL-sort VERSION 0.1.0)

find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)
target_include_directories(${PROJECT_NAME} PUBLIC include/
                                           PUBLIC ../../include)
