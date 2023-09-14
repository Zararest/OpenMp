#include "omp.h"
#include <iostream>

void helloWorld() {
  std::cout << "Hello world from section with " << omp_get_num_threads() 
    << " threads {" << omp_get_thread_num() << "}" << std::endl;
}

int main() {
  #pragma omp parallel
  {
    #pragma omp critical (cout)
    helloWorld();
  }
}
