#include "Manager.h"
#include "Matrix.h"

#include <chrono>

using Data_t = double;

struct CalcRes {
  Matrix<Data_t> Res;
  long long Time;
};

template <typename T>
T f(T x) {
  return sin(3 * x);
}

CalcRes linear(Matrix<Data_t> a) {
  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  for (size_t i = 3; i < a.w(); i++)
    for (size_t j = 0; j < a.h() - 2; j++)
      a[i][j] = sin(3 * a[i - 3][j + 2]);
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a),  
    std::chrono::duration_cast<std::chrono::nanoseconds>(End - Begin).count()};
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

using Pos = Matrix<Data_t>::Pos;

Pos getNextPos(Pos CurPos) {
  return {CurPos.X + XShift, CurPos.Y + YShift};
}

Pos getPrevPos(Pos CurPos) {
  return {CurPos.X - XShift, CurPos.Y - YShift};
}

Data_t calcElem(Matrix<Data_t> &a, Pos CurPos) {
  auto PrevPos = getPrevPos(CurPos);
  if (CurPos.X < XShift || CurPos.Y < YShift || !a.isValid(PrevPos))
    return a[CurPos];
  return f(a[PrevPos]);
}

void calcRound(Matrix<Data_t> &a, Pos CurPos, 
              std::vector<std::pair<Pos, Data_t>> &Res) {
  for (int dy = 0; dy < YShift; ++dy) {
    a[CurPos] = calcElem(a, CurPos);
    Res.emplace_back(CurPos, a[CurPos]);
    CurPos.Y += dy;
  }
}

void calcDiag(Matrix<Data_t> &a, Pos CurPos, 
              std::vector<std::pair<Pos, Data_t>> &Res) {
  // underflow сделает обращение невалидным
  while (a.isValid(CurPos)) {
    calcRound(a, CurPos, Res);
    CurPos = getNextPos(CurPos);
  }
}

Pos getNextDiagPos(Pos DiagPos, size_t IterNum, size_t NumOfThreads, size_t Width) {
  // значит не надо подниматься по Y
  if (DiagPos.X >= NumOfThreads)
    return {DiagPos.X - NumOfThreads, DiagPos.Y};
  // уровень - еще не посчитанные элементы
  // после каждого подсчета дисагонали уровни смещаются вверх, 
  // но текущий все равно считается от 1 тк все ниже заполнены
  // ширина уровня - XShift * CurLevel
  auto CurLevel = 1ul; // то насколько надо будет подняться по Y
  auto CurLevelSize = (CurLevel + 1) * std::abs(XShift);
  std::cout << " Num of threads: " << NumOfThreads << std::endl;
  while (DiagPos.X + CurLevelSize < NumOfThreads) {
    CurLevel++;
    CurLevelSize = (CurLevel + 1) * std::abs(XShift);
    std::cout << "Cur level size: " << CurLevelSize << std::endl; //надо протестить
  }

  DiagPos.Y += CurLevel;
  std::cout << "Cur level: " << CurLevel << " cur level size: " << CurLevelSize;
  std::cout << " pos: " << (DiagPos.X + CurLevelSize + 1 - NumOfThreads) << std::endl;
  DiagPos.X = Width - (DiagPos.X + CurLevelSize + 1 - NumOfThreads);
  return DiagPos;
}

void collectData(Matrix<Data_t> &a, 
                 std::vector<std::pair<Pos, Data_t>> &Res, 
                 MPIManager &M) {
  std::cout << "Size: " << Res.size()  << std::endl;
  assert(Res.size() == a.w() * a.h());
  for (auto [Pos, Val] : Res)
    a[Pos] = Val;
}

CalcRes parallel(Matrix<Data_t> a, MPIManager &M) {
  auto InitPos = Pos{a.w() - M.getPID() - 1, 0};
  auto CurLevel = 0ul;
  auto Res = std::vector<std::pair<Pos, Data_t>>{};
  auto IterNum = 0ul;

  std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();
  while (a.isValid(InitPos)) {
    std::cout << "Cur pos: " << InitPos.X << " " << InitPos.Y << std::endl;
    calcDiag(a, InitPos, Res);
    InitPos = getNextDiagPos(InitPos, IterNum, M.getMPIGroupSize(), a.w());
    IterNum++;
  }

  collectData(a, Res, M);
  std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
  return {std::move(a),  
    std::chrono::duration_cast<std::chrono::nanoseconds>(End - Begin).count()};
}

int main(int Argc, char **Argv) {
  assert(Argc == 3);
  auto a = Matrix<Data_t>(std::stoul(Argv[1]), std::stoul(Argv[2]));
  auto M = MPIManager{};

  auto ParRres = parallel(a, M);
  if (M.getPID() != 0)
    return 0;
  auto LinearRes = linear(a);

  std::cout << "Linear time: " << LinearRes.Time << std::endl;
  std::cout << "Parallel time: " << ParRres.Time << std::endl;

  if (LinearRes.Res != ParRres.Res)
    std::cout << "Incorrect parallel result\n" << std::endl;
}