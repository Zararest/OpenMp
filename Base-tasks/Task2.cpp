#include "omp.h"
#include "Utils.h"

#include <iostream>
#include <cassert>
#include <iomanip>


double calcPart(size_t From, size_t To, size_t N) {
  auto Res = 0.0L;
  for (size_t n = From; n < To && n <= N; ++n)
    Res += utils::floatDiv(1, n);
  return Res;
}

double realRes(size_t N) {
  auto Res = 0.0L;
  for (size_t n = 1; n <= N; ++n)
    Res += utils::floatDiv(1, n);
  return Res;
}

int main(int Argc, char **Argv) {
  assert(Argc == 2);
  auto N = std::atol(Argv[1]);
  auto IterPerThread = 0lu;
  auto Res = 0.0L;

  #pragma omp parallel shared(IterPerThread) reduction (+: Res)
  {
    auto NumOfThreads = omp_get_num_threads();
    #pragma omp single
    {
      IterPerThread = utils::ceilDiv(N, NumOfThreads);
    } 
    auto From = IterPerThread * omp_get_thread_num() + 1;
    Res = calcPart(From, From + IterPerThread, N);
  } 
  auto RealRes = realRes(N);
  if (!utils::cmpFloats(Res, RealRes))
    std::cout << "Wrong result!!!\n" << "\tdifferent: " 
      << std::setprecision (15) << Res - RealRes << std::endl; 
  std::cout << "Result for N = " << N << ": " << Res << "\n";
}