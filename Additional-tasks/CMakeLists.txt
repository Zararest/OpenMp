cmake_minimum_required(VERSION 3.25)

add_executable()

find_package(OpenMP)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)