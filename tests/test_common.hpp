#ifndef GSA_TEST_COMMON_HPP
#define GSA_TEST_COMMON_HPP

#include <array>
#include <cmath>
#include <limits>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "gsa/gsa.hpp"

namespace gsa_test {

inline double Sphere(std::span<const double> x) {
  double s{};
  for (double v : x) s += v * v;
  return s;
}

inline double Rosenbrock(std::span<const double> x) {
  double s{};
  for (auto i : std::views::iota(0ULL, x.size() - 1)) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

inline double Shekel(std::span<const double> x) {
  static constexpr std::array<std::array<double, 25>, 2> kAij = {
      {{-32, -16, 0,   16,  32, -32, -16, 0,   16,  32, -32, -16, 0,
        16,  32,  -32, -16, 0,  16,  32,  -32, -16, 0,  16,  32},
       {-32, -32, -32, -32, -32, -16, -16, -16, -16, -16, 0,  0, 0,
         0,   0,   16,  16,  16,  16,  16,  32,  32,  32,  32, 32}}};
  double sum{1.0 / 500.0};
  for (auto j : std::views::iota(0U, 25U)) {
    double inner{};
    for (auto i : std::views::iota(0U, 2U)) {
      const double d{x[i] - kAij[i][j]};
      inner += d * d * d * d * d * d;
    }
    sum += 1.0 / (j + 1.0 + inner);
  }
  return 1.0 / sum;
}

inline gsa::GsaConfig Config(bool minimize = true, size_t n_agents = 50) {
  return {.n_agents = n_agents,
          .max_iter = 500,
          .g0 = 100.0,
          .alpha = 20.0,
          .minimize = minimize,
          .seed = 12345};
}

inline gsa::GsaResult Optimize(int dims, double lo, double hi,
                           double (*fn)(std::span<const double>),
                           const gsa::GsaConfig& cfg) {
  gsa::GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
  return gsa.Optimize();
}

/** Assert per-iteration invariants. */
inline void CheckHistory(const gsa::GsaResult& res, const gsa::GsaConfig& cfg,
                         std::string_view name) {
  EXPECT_EQ(res.history.size(), cfg.max_iter + 1)
      << name << ": history size == max_iter + 1";

  double prev_best{cfg.minimize ? std::numeric_limits<double>::max()
                                  : std::numeric_limits<double>::lowest()};
  for (const auto& it : res.history) {
    if (cfg.minimize) {
      EXPECT_LE(it.best_so_far, prev_best) << name << ": best_so_far monotonic";
    } else {
      EXPECT_GE(it.best_so_far, prev_best) << name << ": best_so_far monotonic";
    }
    prev_best = it.best_so_far;

    const double lo{cfg.minimize ? it.best_iter : it.worst_iter};
    const double hi{cfg.minimize ? it.worst_iter : it.best_iter};
    EXPECT_GE(it.mean_fitness, lo) << name << ": mean in [best, worst]";
    EXPECT_LE(it.mean_fitness, hi) << name << ": mean in [best, worst]";
    EXPECT_GE(it.median_fitness, lo) << name << ": median in [best, worst]";
    EXPECT_LE(it.median_fitness, hi) << name << ": median in [best, worst]";
    EXPECT_GE(it.stddev_fitness, 0.0) << name << ": stddev finite";
    EXPECT_TRUE(std::isfinite(it.stddev_fitness))
        << name << ": stddev finite";
  }
}

}  // namespace gsa_test

#endif  // GSA_TEST_COMMON_HPP
