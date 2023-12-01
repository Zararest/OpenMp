#include "Manager.h"
#include "Matrix.h"

#include <chrono>

struct CalcRes {
  Matrix<double> Res;
  long long Time;
};

template <typename T>
T f(T x) {
  return sin(3 * x);
}

CalcRes linear(Matrix<double> a) {
  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  for (size_t i = 3; i < a.w(); i++)
    for (size_t j = 0; j < a.h() - 2; j++)
      a[i][j] = sin(3 * a[i - 3][j + 2]);
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a),  std::chrono::duration_cast<std::chrono::nanoseconds>(End - Begin).count()};
}


/*

Вычисления диагонали для первого потока:
==================
                X||
                X||
                X||
           X     ||
           X     ||
           X     ||

Для второго:
==================
               OX||
               OX||
               OX||
          OX     || <- начало 1 уровня (после того как все слева заполнено)
          OX     ||
          OX     ||

*/

// смещение, чтобы получить прошлые значения
constexpr int YShift = 3;
constexpr int XShift = -2;

using Pos = Matrix<double>::Pos;

Pos getNextPos(Pos CurPos) {
  return {CurPos.X + XShift, CurPos.Y + YShift};
}

Pos getPrevPos(Pos CurPos) {
  return {CurPos.X - XShift, CurPos.Y - YShift};
}

double calcElem(Matrix<double> &a, Pos CurPos) {
  auto PrevPos = getPrevPos(CurPos);
  if (CurPos.X < XShift || CurPos.Y < YShift || !a.isValid(PrevPos))
    return a[CurPos];
  return f(a[PrevPos]);
}

void calcRound(Matrix<double> &a, Pos CurPos, 
              std::vector<std::pair<Pos, size_t>> &Res) {
  for (int dy = 0; dy < YShift; ++dy) {
    a[CurPos] = calcElem(a, CurPos);
    Res.emplace_back(CurPos, a[CurPos]);
    CurPos.Y += dy;
  }
}

void calcDiag(Matrix<double> &a, Pos CurPos, 
              std::vector<std::pair<Pos, size_t>> &Res) {
  // underflow сделает обращение невалидным
  while (a.isValid(CurPos)) {
    calcRound(a, CurPos, Res);
    CurPos = getNextPos(CurPos);
  }
}

Pos getNextDiagPos(Pos DiagPos, size_t IterNum, size_t NumOfThreads, size_t Width) {
  // значит не надо подниматься по Y
  if (DiagPos.X >= NumOfThreads - 1)
    return {DiagPos.X + 1 - NumOfThreads, DiagPos.Y};
  
  // уровень - еще не посчитанные элементы
  // после каждого подсчета дисагонали уровни смещаются вверх, 
  // но текущий все равно считается от 1 тк все ниже заполнены
  // ширина уровня - XShift * CurLevel
  auto CurLevel = 1ul; // то насколько надо будет подняться по Y
  assert(CurLevel > 0);
  auto CurLevelSize = (CurLevel + 1) * XShift;

  while (DiagPos.X + CurLevelSize < NumOfThreads - 1) {
    CurLevel++;
    CurLevelSize = (CurLevel + 1) * XShift;
  }

  DiagPos.Y += CurLevel;
  DiagPos.X = Width - (DiagPos.X + CurLevelSize + 1 - NumOfThreads);
  return DiagPos;
}

CalcRes parallel(Matrix<double> a, 
                 size_t ThreadNum, 
                 size_t NumOfThreads) {
  auto InitPos = Pos{a.w() - ThreadNum, 0};
  auto CurLevel = 0ul;
  auto Res = std::vector<std::pair<Pos, size_t>>{};
  auto IterNum = 0ul;

  while (a.isValid(InitPos)) {
    calcDiag(a, InitPos, Res);
    InitPos = getNextDiagPos(InitPos, IterNum, NumOfThreads, a.w());
    IterNum++;
  }
}

int main(int Argc, char **Argv) {
  assert(Argc == 3);
  auto a = Matrix<double>(std::stoul(Argv[1]), std::stoul(Argv[2]));


}