#ifndef OBJECTIVE_HPP
#define OBJECTIVE_HPP

#include <ranges>
#include <span>

namespace objective {

// The optimization target. Edit this function, then recompile.
inline double Fn(std::span<const double> x) {
  double s{};
  for (auto i : std::views::iota(0ULL, x.size() - 1)) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

}  // namespace objective

#endif