#include "Matrix.h"
#include <omp.h>

#include <chrono>
#include <thread>

#define NUM_OF_THREADS 1

using Data_t = double;

struct CalcRes {
  Matrix<Data_t> Res;
  long long Time;
};

constexpr int YShift = 3;
constexpr int XShift = -2;

template <typename T>
T f(T x) {
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  return sin(0.1 * x);
}

CalcRes linear(Matrix<Data_t> a) {
  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  for (long i = 0; i < a.h() - YShift; i++)
    for (long j = -XShift; j < a.w(); j++)
      a[i][j] = f(a[i + YShift][j + XShift]);
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a),  
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Begin).count()};
}

void fillBuf(Matrix<Data_t> &a, long y, std::vector<Data_t> &Buf) {
  if (y >= a.h())
    return;
  auto LinesToCopy = omp_get_num_threads() / YShift + 1;
  auto GlobalNextLinePos = y + YShift;
  auto NewLineNum = (y / YShift) % LinesToCopy;
  if (GlobalNextLinePos >= a.h())
    return;
  
  for (long x = 0; x < a.w(); x++)
    Buf[NewLineNum * a.w() + x] = a[GlobalNextLinePos][x];
}

CalcRes parallel(Matrix<Data_t> a, int NumOfThreads) {
  omp_set_num_threads(NumOfThreads);

  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  #pragma omp parallel
  {
    auto IterNum = a.h() / omp_get_num_threads();
    auto LinesToCopy = omp_get_num_threads() / YShift + 1;
    auto Lines = std::vector<Data_t>(LinesToCopy * a.w());

    for (long x = 0; x < a.w(); x++)
      Lines[x] = a[omp_get_thread_num() + YShift][x];

    for (long It = 0; It < IterNum; It++) {
      auto i = omp_get_thread_num() + omp_get_num_threads() * It;
      fillBuf(a, i, Lines);

      #pragma omp barrier
      
      if (i < a.h() - YShift) {
        auto LinePosInBuf = (i / YShift) % LinesToCopy;
        for (long j = -XShift; j < a.w(); j++) {   
          a[i][j] = f(Lines[LinePosInBuf * a.w() + j + XShift]);
        }
      }
    }
  }
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a),  
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Begin).count()};
}

int main(int Argc, char **Argv) {
  assert(Argc == 4);
  auto NumOfThreads = std::stoul(Argv[1]);
  auto a = Matrix<Data_t>(std::stoul(Argv[2]), std::stoul(Argv[3]));

  auto LinearRes = linear(a);
  auto ParRes = parallel(a, NumOfThreads);

  std::cout << "Linear time: " << LinearRes.Time << std::endl;
  std::cout << "Parallel time: " << ParRes.Time << std::endl;

  if (LinearRes.Res != ParRes.Res)
    std::cout << "Incorrect parallel result\n" << std::endl; 
}