#include "Driver.h"
#include "Utils.h"

#include <chrono>

template <typename It>
void print(It Beg, It End) {
  for (; Beg != End; ++Beg)
    std::cout << *Beg << " ";
  std::cout << "\n" << std::endl;
}

int main() {
  auto Cfg = Config{};

  auto MergeSort = Driver{Cfg};
  MergeSort.loadKernel("/home/uwu/Proga/OpenMP/Additional-tasks/OpenCL-merge-sort/Merge.cl");
  
  auto Arr = std::vector<Config::T>{};
  utils::fillwithRandom(0, Cfg.Size, Cfg.Size, 
                        std::back_inserter(Arr));
  auto GPUArr = std::vector<int>{Arr.begin(), Arr.end()};
  std::chrono::high_resolution_clock::time_point Start, End;

  Start = std::chrono::high_resolution_clock::now();
  MergeSort.sort(GPUArr.begin(), GPUArr.end()); 
  End = std::chrono::high_resolution_clock::now();

  std::cout << "GPU time: " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Start).count() << std::endl;

  Start = std::chrono::high_resolution_clock::now();
  std::sort(Arr.begin(), Arr.end());
  End = std::chrono::high_resolution_clock::now();

  std::cout << "CPU time: " <<
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Start).count() << std::endl;
  
  assert(Arr.size() == GPUArr.size());
  std::cout << "Is equal: " << std::boolalpha << 
    std::equal(GPUArr.begin(), GPUArr.end(), Arr.begin()) << std::endl;
}