#include "Matrix.h"
#include <omp.h>

#include <chrono>
#include <thread>

using Data_t = double;

struct CalcRes {
  Matrix<Data_t> a;
  Matrix<Data_t> b;
  long long Time;
};

constexpr int YShift = -5;
constexpr int XShift = 2;

template <typename T>
T f(T x) {
  //std::this_thread::sleep_for(std::chrono::milliseconds(1));
  return sin(0.005 * x);
}

CalcRes linear(Matrix<Data_t> a, Matrix<Data_t> b) {
  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  for (long i = 0; i < a.h(); i++)
    for (long j = 0; j < a.w(); j++)
      a[i][j] = f(a[i][j]);
  
  for (long i = std::abs(YShift); i < a.h(); i++)
    for (long j = 0; j < a.w() - XShift; j++)
      b[i][j] = a[i + YShift][j + XShift] * 1.5;
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a), std::move(b),
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Begin).count()};
}

CalcRes parallel(Matrix<Data_t> a, Matrix<Data_t> b, int NumOfThreads) {
  omp_set_num_threads(NumOfThreads);

  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  #pragma omp parallel
  {
    for (long i = omp_get_thread_num(); i < a.h(); i += omp_get_num_threads())
      for (long j = 0; j < a.w(); j++)
        a[i][j] = f(a[i][j]);

    #pragma omp barrier

    for (long i = -YShift + omp_get_thread_num(); i < a.h(); i += omp_get_num_threads())
      for (long j = 0; j < a.w() - XShift; j++)
        b[i][j] = a[i + YShift][j + XShift] * 1.5;
  }
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a), std::move(b), 
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Begin).count()};
}

int main(int Argc, char **Argv) {
  assert(Argc == 4);
  auto NumOfThreads = std::stoul(Argv[1]);
  auto a = Matrix<Data_t>(std::stoul(Argv[2]), std::stoul(Argv[3]));
  auto b = Matrix<Data_t>(std::stoul(Argv[2]), std::stoul(Argv[3]));

  auto LinearRes = linear(a, b);
  auto ParRes = parallel(a, b, NumOfThreads);

  std::cout << "Linear time: " << LinearRes.Time << std::endl;
  std::cout << "Parallel time: " << ParRes.Time << std::endl;

  if (LinearRes.a != ParRes.a || LinearRes.b != ParRes.b)
    std::cout << "Incorrect parallel result\n" << std::endl; 
}