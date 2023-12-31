#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include <iostream>

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
      assert(J >= 0 && Offset >= 0 && Offset + J < Buf.size());
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
    Buf = std::vector<T>(Size);
    std::iota(Buf.begin(), Buf.end(), 0);
    //std::generate_n(std::back_inserter(Buf), Size, Rng);
  }

  T &operator[] (Pos Position) {
    assert(Position.X >= 0 && Position.Y >= 0 && 
           Position.Y < RowNum && Position.X < ColNum);
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
                        auto e = std::abs(Lhs) * 0.01;
                        return std::abs(Lhs - Rhs) <= e;
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