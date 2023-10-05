#include "omp.h"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>
#include <cassert>

#pragma once

namespace utils {

template <typename T1, typename T2>
size_t ceilDiv(T1 Lhs, T2 Rhs) {
  auto LhsF = static_cast<float>(Lhs);
  auto RhsF = static_cast<float>(Rhs);
  return ceil(LhsF / RhsF);
}

template <typename T1, typename T2>
double floatDiv(T1 Lhs, T2 Rhs) {
  auto LhsF = static_cast<double>(Lhs);
  auto RhsF = static_cast<double>(Rhs);
  return LhsF / RhsF;
}

bool cmpFloats(double Lhs, double Rhs) {
  constexpr auto e = 0.00000001L;
  return (Lhs - Rhs) > -e && (Lhs - Rhs) < e;
}

void printThreadPos(std::ostream &S) {
  S << "{" << omp_get_level() << ", " << omp_get_thread_num() 
    << ", " << omp_get_ancestor_thread_num(omp_get_level()) << "}";
}

template <typename InsertIt>
void fillwithRandom(size_t From, size_t To, size_t Size, InsertIt Inserter) {
  std::random_device Device;
  std::mt19937 Mersenne {Device()};  // Generates random integers
  std::uniform_int_distribution<size_t> Dist{From, To};
  
  auto Gen = [&Dist, &Mersenne](){
                return Dist(Mersenne);
              };
  std::vector<size_t> Vec(Size);
  std::generate(std::begin(Vec), std::end(Vec), Gen);
  std::copy(Vec.begin(), Vec.end(), Inserter);
}

bool isPowOf2(size_t N) {
  assert(N);
  return (N & (N - 1)) == 0;
}

size_t getGreaterPowOf2(size_t N) {
  return std::pow(2, std::ceil(std::log2(N)));
}

} // namespace utils