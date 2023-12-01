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