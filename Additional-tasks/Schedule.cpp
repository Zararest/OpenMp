#include "omp.h"
#include "Utils.h"

#include <iostream>
#include <chrono>
#include <thread>

void printInfo(size_t It) {
  #pragma omp critical
  {
    std::cout << "Iteration: " << It << " in:";
    utils::printThreadPos(std::cout);
    std::cout << std::endl;
  }
}

int main() {
  // Очевидно статик
  std::cout << "Default\n" << std::endl;
  #pragma omp parallel for
  for (size_t i = 0; i < 13; i++) {
    printInfo(i);
  }

  std::cout << "Static\n" << std::endl;
  #pragma omp parallel for schedule(static, 4)
  for (size_t i = 0; i < 14; i++) {
    printInfo(i);
  } 

  // Размер чанка 2 
  std::cout << "Dynamic\n" << std::endl;
  #pragma omp parallel for schedule(dynamic, 3)  num_threads(3)
  for (size_t i = 0; i < 23; i++) {
    if (omp_get_thread_num() % 3 != 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    printInfo(i);
  }

  // Тут гарантируется что 
  std::cout << "Guided\n" << std::endl;
  #pragma omp parallel for schedule(guided, 3)  num_threads(3)
  for (size_t i = 0; i < 23; i++) {
    if (omp_get_thread_num() % 3 != 1)
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    printInfo(i);
  }
}