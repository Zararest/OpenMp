cmake_minimum_required(VERSION 3.13.4)
project(Lab2)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(1-G 1-g.cpp)
target_link_libraries(1-G  ${MPI_C_LIBRARIES})
target_compile_features(1-G PRIVATE cxx_std_20)
target_link_libraries(1-G pthread)
target_link_libraries(1-G mpi_cxx)
# mpirun -np 1 ./1-G 20 20

find_package(OpenMP)
add_executable(2-G 2-g.cpp)
target_link_libraries(2-G PRIVATE OpenMP::OpenMP_CXX)

add_executable(3-G 3-g.cpp)
target_link_libraries(3-G PRIVATE OpenMP::OpenMP_CXX)