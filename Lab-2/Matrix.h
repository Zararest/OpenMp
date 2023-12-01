#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

template <typename T>
class Matrix {
  std::vector<T> Buf;
  long ColNum;
  long RowNum;

  struct Proxy {
    long Offset;
    std::vector<T> &Buf;

    Proxy(std::vector<T> &Buf, long Offset) : Offset{Offset}, 
                                                Buf{Buf} {}

    T &operator[] (long J) {
      return Buf[Offset + J];
    }
  };

public:
  struct Pos {
    long X;
    long Y;
  };

  Matrix(long RowNum, long ColNum) : ColNum{ColNum}, RowNum{RowNum} {
    auto Size = ColNum * RowNum;
    assert(Size);
    std::mt19937 Rng; 
    std::generate_n(std::back_inserter(Buf), Size, Rng);
  }

  T &operator[] (Pos Position) {
    return Buf[Position.Y * ColNum + Position.X];
  }
 
  Proxy operator[] (long I) {
    return Proxy{Buf, I * ColNum};
  }

  long w() const {
    return ColNum;
  }

  long h() const {
    return RowNum;
  }

  bool isValid(Pos AccessPos) {
    return AccessPos.X >= 0 && AccessPos.Y >= 0 &&
           AccessPos.X < ColNum && AccessPos.Y < RowNum;
  }

  bool operator== (const Matrix &Rhs) {
    return Rhs.Buf.size() == Buf.size() && 
           std::equal(Buf.begin(), Buf.end(), Rhs.Buf.begin(),
                      [](auto Lhs, auto Rhs) {
                        auto e = Lhs * 0.01;
                        return Rhs > Lhs - e && Rhs < Lhs + e;
                      });
  }

  bool operator!= (const Matrix &Rhs) {
    return !this->operator==(Rhs);
  }

  void dump() {
    for (int y = 0; y < RowNum; ++y) {
      for (int x = 0; x < ColNum; ++x)
        std::cout << operator[](y)[x] << " ";
      std::cout << std::endl;
    }
  }
};