#include <cmath>

namespace utils {

template <typename T1, typename T2>
size_t ceilDiv(T1 Lhs, T2 Rhs) {
  auto LhsF = static_cast<float>(Lhs);
  auto RhsF = static_cast<float>(Rhs);
  return ceil(LhsF / RhsF);
}

template <typename T1, typename T2>
double floatDiv(T1 Lhs, T2 Rhs) {
  auto LhsF = static_cast<double>(Lhs);
  auto RhsF = static_cast<double>(Rhs);
  return LhsF / RhsF;
}

bool cmpFloats(double Lhs, double Rhs) {
  constexpr auto e = 0.00000001L;
  return (Lhs - Rhs) > -e && (Lhs - Rhs) < e;
}

} // namespace utils