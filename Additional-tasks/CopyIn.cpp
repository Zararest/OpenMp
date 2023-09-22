#include "omp.h"
#include "Utils.h"

#include <iostream>

/* not working example
struct CopyableT {
  static size_t NumOfCreation;
  static size_t NumOfCopy;

  CopyableT() {
    NumOfCreation++;
    std::cout << "Creation" << std::endl;
  }

  CopyableT(const CopyableT &Old) {
    std::cout << "Copy in :";
    utils::printThreadPos(std::cout);
    std::cout << std::endl;
  }
};

size_t CopyableT::NumOfCreation = 0;
size_t CopyableT::NumOfCopy = 0;

CopyableT Obj;
#pragma omp threadprivate(Obj1)

void demo() {
  #pragma omp parallel copyin(Obj1)
  {
    
  }
  std::cout << "Num of creation: " << CopyableT::NumOfCreation << std::endl;
  std::cout << "Num of copy: " << CopyableT::NumOfCopy << std::endl;
}
*/ 

int Var = 10;
// Каждый запущенный тред(включая nested) поллучает свою копию Var
#pragma omp threadprivate(Var)

void demo() {
  Var = 0;
  #pragma omp parallel
  {
    #pragma omp single 
    {
      std::cout << "Var without copyin: " << Var << std::endl;
      Var = 3;
      #pragma omp parallel
      {
        #pragma omp single 
        {
        std::cout << "Var without copyin(nested): " << Var << std::endl;
        }
      }
    }
  }
  std::cout << "Var: " << Var << std::endl;

  Var = 1;
  #pragma omp parallel copyin(Var)
  {
    #pragma omp single
    {
      std::cout << "Var with copyin: " << Var << std::endl;
      Var++;
      #pragma omp parallel copyin(Var)
      {
        #pragma omp single 
        {
        std::cout << "Var with copyin(nested): " << Var << std::endl;
        }
      }
    }
  }
  std::cout << "Var: " << Var << std::endl;
}

int main() {
  omp_set_nested(true);
  demo();
}