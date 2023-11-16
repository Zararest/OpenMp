#include "Driver.h"
#include "Utils.h"

int main() {
  auto Cfg = Config{};

  auto MergeSort = Driver{Cfg};
  MergeSort.loadKernel("/home/uwu/Proga/OpenMP/Additional-tasks/OpenCL-merge-sort/Merge.cl");

  auto Arr = std::vector<Config::T>{};
  utils::fillwithRandom(0, Cfg.Size, Cfg.Size, 
                        std::back_inserter(Arr));
  MergeSort.sort(Arr.begin(), Arr.end());                    
}