#include "omp.h"
#include "Utils.h"

#include <iostream>

// copyprivate позволяет изменить переменную во всех потоках

int main() {
  size_t A = 0;
  #pragma omp parallel firstprivate(A) num_threads(3)
  {
    A++;
    #pragma omp critical
    {
      std::cout << "A: " << A << " ";
      utils::printThreadPos(std::cout);
      std::cout << std::endl;
    }
    
    #pragma omp barrier

    #pragma omp single copyprivate(A)
    {
      A = 129;
      std::cout << "Translated A: " << A << " "; 
      utils::printThreadPos(std::cout);
      std::cout << std::endl;
    }

    #pragma omp barrier

    #pragma omp critical
    {
      std::cout << "A: " << A << " ";
      utils::printThreadPos(std::cout);
      std::cout << std::endl;
    }

    #pragma omp barrier

    #pragma omp single
    {
      A = 549;
      std::cout << "Copied A: " << A << " "; 
      utils::printThreadPos(std::cout);
      std::cout << std::endl;
    }

    #pragma omp barrier

    #pragma omp critical
    {
      std::cout << "A: " << A << " ";
      utils::printThreadPos(std::cout);
      std::cout << std::endl;
    }
  }
}