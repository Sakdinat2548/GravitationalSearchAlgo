#ifndef GSA_TEST_COMMON_HPP
#define GSA_TEST_COMMON_HPP

#include <array>
#include <cmath>
#include <limits>
#include <ranges>
#include <span>
#include <string>

#include "gsa/gsa.hpp"
#include "test_framework.hpp"

namespace gsa_test {

inline double Sphere(std::span<const double> x) {
  double s = 0.0;
  for (double v : x) s += v * v;
  return s;
}

inline double Rosenbrock(std::span<const double> x) {
  double s = 0.0;
  for (size_t i : std::views::iota(size_t{0}, x.size() - 1)) {
    const double d = x[i + 1] - (x[i] * x[i]);
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
  double sum = 1.0 / 500.0;
  for (size_t j : std::views::iota(size_t{0}, size_t{25})) {
    double inner = 0.0;
    for (size_t i : std::views::iota(size_t{0}, size_t{2})) {
      const double d = x[i] - kAij[i][j];
      inner += d * d * d * d * d * d;
    }
    sum += 1.0 / (j + 1.0 + inner);
  }
  return 1.0 / sum;
}

inline GsaConfig Config(bool minimize = true, int n_agents = 50) {
  return {.n_agents = n_agents,
          .max_iter = 500,
          .g0 = 100.0,
          .alpha = 20.0,
          .minimize = minimize,
          .seed = 12345};
}

inline GsaResult Optimize(int dims, double lo, double hi,
                          double (*fn)(std::span<const double>),
                          const GsaConfig& cfg) {
  GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
  return gsa.Optimize();
}

/** Assert per-iteration invariants; returns true if all pass. */
inline bool CheckHistory(const GsaResult& res, const GsaConfig& cfg,
                         const char* name) {
  bool ok = true;
  const auto report = [&](bool cond, const char* what) {
    ok = ok && cond;
    if (!cond) Expect(false, (std::string(name) + ": " + what).c_str());
  };

  report(res.history.size() == static_cast<size_t>(cfg.max_iter) + 1,
         "history size == max_iter + 1");

  double prev_best = cfg.minimize ? std::numeric_limits<double>::max()
                                  : std::numeric_limits<double>::lowest();
  for (const auto& it : res.history) {
    const bool monotonic = cfg.minimize ? it.best_so_far <= prev_best
                                        : it.best_so_far >= prev_best;
    report(monotonic, "best_so_far monotonic");
    prev_best = it.best_so_far;

    const double lo = cfg.minimize ? it.best_iter : it.worst_iter;
    const double hi = cfg.minimize ? it.worst_iter : it.best_iter;
    report(it.mean_fitness >= lo && it.mean_fitness <= hi,
           "mean in [best, worst]");
    report(it.median_fitness >= lo && it.median_fitness <= hi,
           "median in [best, worst]");
    report(it.stddev_fitness >= 0.0 && std::isfinite(it.stddev_fitness),
           "stddev finite");
  }
  return ok;
}

}  // namespace gsa_test

#endif  // GSA_TEST_COMMON_HPP
