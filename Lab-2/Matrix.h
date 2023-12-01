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
    return AccessPos.X < ColNum && AccessPos.Y < RowNum;
  }

  bool operator== (const Matrix &Rhs) {
    return Rhs.Buf.size() != Buf.size() && 
           std::equal(Buf.begin(), Buf.end(), Rhs.Buf.begin());
  }

  bool operator!= (const Matrix &Rhs) {
    return !this->operator==(Rhs);
  }
};