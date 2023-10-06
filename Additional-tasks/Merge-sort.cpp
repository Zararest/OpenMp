#include "omp.h"
#include "Utils.h"

#include <chrono>
#include <optional>
#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

#define LINEAR_THRESHOLD 2

template <typename T>
class ValWrapper {
  std::optional<T> Val;

public:
  ValWrapper(std::optional<T> NewVal = std::nullopt) : Val{NewVal} {}

  auto &get() {
    assert(Val);
    return *Val;
  }

  operator bool() const {
    return static_cast<bool>(Val);
  }
  
  bool operator <(const ValWrapper &Rhs) const {
    // таким образом все отсутствующие элементы будут в конце
    if (!Val)
      return false;
    if (!Rhs.Val)
      return true;
    return *Val < *Rhs.Val;
  }

  bool operator ==(const ValWrapper &Rhs) const {
    if (!Val && !Rhs.Val)
      return true;
    if (!Val || !Rhs.Val)
      return false;
    return Val == Rhs.Val;
  }
};

namespace sort {

template <typename It, typename InsertIt>
void processEdgeCase(It DataBeg, It DataEnd, InsertIt Inserter) {
  using T = typename std::iterator_traits<It>::value_type;

  auto Buf = std::vector<T>{DataBeg, DataEnd};
  std::sort(Buf.begin(), Buf.end());
  std::copy(Buf.begin(), Buf.end(), Inserter);
}

// Глубина рекурсии - размер 
template <typename It, typename InsertIt>
void task(It DataBeg, It DataEnd, size_t LinearThreshold, InsertIt Inserter) {
  using T = typename std::iterator_traits<It>::value_type;

  auto Size = std::distance(DataBeg, DataEnd);
  if (Size <= LinearThreshold) {
    processEdgeCase(DataBeg, DataEnd, Inserter);
    return;
  }

  assert(utils::isPowOf2(Size));
  auto LeftBufEnd = DataBeg;
  std::advance(LeftBufEnd, Size / 2);
  auto LeftBuf = std::vector<T>{};
  auto RightBuf = std::vector<T>{};

  #pragma omp task shared(LeftBuf) untied
    task(DataBeg, LeftBufEnd, LinearThreshold, std::back_inserter(LeftBuf));

  #pragma omp task shared(RightBuf) untied
    task(LeftBufEnd, DataEnd, LinearThreshold, std::back_inserter(RightBuf));

  #pragma omp taskwait
    std::merge(LeftBuf.begin(), LeftBuf.end(),
              RightBuf.begin(), RightBuf.end(),
              Inserter);
}

}// namespace sort

struct Config {
  size_t Size = 1000000; 
  size_t LinearThreshold = 1;
  bool CheckResults = false;
  bool Dump = false;
  bool SkipInit = false;
  size_t NumberOfThreads = 8;
};

void bootTasks(Config Cfg) {
  omp_set_nested(true);
  omp_set_num_threads(Cfg.NumberOfThreads);

  auto PowOf2Arr = 
    std::vector<ValWrapper<size_t>>(utils::getGreaterPowOf2(Cfg.Size));
  if (!Cfg.SkipInit) {
    PowOf2Arr.clear();
    auto IntArray = std::vector<size_t>(Cfg.Size);
    utils::fillwithRandom(0, Cfg.Size, IntArray.size(), 
                          std::inserter(IntArray, IntArray.begin()));
    auto OverheadSize = utils::getGreaterPowOf2(IntArray.size()) - IntArray.size();
    std::transform(IntArray.begin(), IntArray.end(), std::back_inserter(PowOf2Arr),
                  [](size_t Val) {
                    return ValWrapper<size_t>{Val};
                  });
    PowOf2Arr.insert(PowOf2Arr.end(), OverheadSize, ValWrapper<size_t>{});
  }
  auto SortedArr = std::vector<ValWrapper<size_t>>{};
  auto Start = std::chrono::steady_clock::now();

  #pragma omp parallel
    #pragma omp single
      sort::task(PowOf2Arr.begin(), PowOf2Arr.end(), 
                Cfg.LinearThreshold, std::back_inserter(SortedArr));
  #pragma omp taskwait
  auto End = std::chrono::steady_clock::now();
  auto Duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(End - Start);
  std::cout << "Time: " << Duration.count() << "ms" << std::endl;

  if (Cfg.CheckResults) {
    std::sort(PowOf2Arr.begin(), PowOf2Arr.end());
    assert(PowOf2Arr.size() == SortedArr.size());
    std::cout << "Correct sorting: " << std::boolalpha 
      << std::equal(PowOf2Arr.begin(), PowOf2Arr.end(), SortedArr.begin()) << std::endl;
  }

  if (Cfg.Dump) {
    auto Counter = 0;
    std::cerr << "Sorted array dump:" << std::endl;
    for (auto Elem : SortedArr) {
      if (Counter % 10 == 0)
        std::cerr << "\n\t";

      if (Elem) {
        std::cerr << " " << Elem.get();
      } else {
        std::cerr << " NAN";
      }
      Counter++;
    }
  }
}


int main(int Argc, char **Argv) {
  Argv++;
  Argc--;
  auto Cfg = Config{};
  while (Argc > 0) {
    auto Option = std::string{Argv[0]};
    Argv++;
    Argc--;
    if (Option == "--check") {
      Cfg.CheckResults = true;
      std::cout << "Running with answer checker" << std::endl;
      continue;
    }

    if (Option == "--size") {
      assert(Argc >= 1 && "Too few arguments");
      Cfg.Size = std::stoi(Argv[0]);
      Argv++;
      Argc--;
      continue;
    }

    if (Option == "--threads-num") {
      assert(Argc >= 1 && "Too few arguments");
      Cfg.NumberOfThreads = std::stoi(Argv[0]);
      Argv++;
      Argc--;
      continue;
    }

    if (Option == "--dump") {
      Cfg.Dump = true;
      std::cout << "Running with answer dump" << std::endl;
      continue;
    }

    if (Option == "--linear-threashold") {
      assert(Argc >= 1 && "Too few arguments");
      Cfg.LinearThreshold = std::stoi(Argv[0]);
      Argv++;
      Argc--;
      continue;
    }

    if (Option == "--skip-init") {
      Cfg.SkipInit = true;
      std::cout << "Running with trash values" << std::endl;
      continue;
    }
 
    std::cout << "Unknown argument: " << Option << std::endl;
    assert(false);
  }

  bootTasks(Cfg);
}