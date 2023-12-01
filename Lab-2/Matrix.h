#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

template <typename T>
class Matrix {
  std::vector<T> Buf;
  size_t ColNum;
  size_t RowNum;

  struct Proxy {
    size_t Offset;
    std::vector<T> &Buf;

    Proxy(std::vector<T> &Buf, size_t Offset) : Offset{Offset}, 
                                                Buf{Buf} {}

    T &operator[] (size_t J) {
      return Buf[Offset + J];
    }
  };

public:
  struct Pos {
    size_t X;
    size_t Y;
  };

  Matrix(size_t RowNum, size_t ColNum) : ColNum{ColNum}, RowNum{RowNum} {
    auto Size = ColNum * RowNum;
    assert(Size);
    std::mt19937 Rng; 
    std::generate_n(std::back_inserter(Buf), Size, Rng);
  }

  T &operator[] (Pos Position) {
    return Buf[Position.Y * ColNum + Position.X];
  }
 
  Proxy operator[] (size_t I) {
    return Proxy{Buf, I * ColNum};
  }

  size_t w() const {
    return ColNum;
  }

  size_t h() const {
    return RowNum;
  }

  bool isValid(Pos AccessPos) {
    return AccessPos.X < ColNum && AccessPos.Y < RowNum;
  }
};