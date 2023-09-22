#include "omp.h"
#include "Utils.h"

#include <iostream>

// Nested parallelism can be enabled or disabled 
//  by setting the OMP_NESTED environment variable or calling omp_set_nested().

// Можно заметить, что существует 13 = 1 + 3 + 9 записей с одинаковыми номерами тредов.
// У тредов на первом уровне вложенности номер родителя - они сами.

void printPos() {
  #pragma omp critical 
  {
    std::cout << "Thread number: " << omp_get_thread_num() << "\n"
      << "\tfrom nested level: " << omp_get_level() << "\n"
      << "\tnum of threads: " << omp_get_num_threads() << "\n"
      << "\tancestor number: " << omp_get_ancestor_thread_num(omp_get_level()) 
      << "\n" << std::endl;
  }
}

int main() {
  omp_set_nested(true);
  std::cout << "Maximum number of levels: " << omp_get_max_active_levels() << std::endl;

  #pragma omp parallel num_threads(3)
  {
    printPos();
    
    #pragma omp parallel num_threads(3)
    {
      printPos();
    
      #pragma omp parallel num_threads(3)
      {
        printPos();
      }
    }
  }
}