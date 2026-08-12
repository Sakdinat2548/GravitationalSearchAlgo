#include <cmath>
#include <iostream>
#include <span>
#include <vector>

#include "gsa.hpp"

static double sphere(std::span<const double> x) {
  double s = 0.0;
  for (double v : x) s += v * v;
  return s;
}

static double rosenbrock(std::span<const double> x) {
  double s = 0.0;
  for (size_t i = 0; i + 1 < x.size(); ++i) {
    const double d = x[i + 1] - x[i] * x[i];
    s += 100.0 * d * d + (x[i] - 1.0) * (x[i] - 1.0);
  }
  return s;
}

static void print_result(const GsaResult& res) {
  std::cout << "best_val = " << res.best_val << "\n  position:";
  for (double p : res.best_pos) std::cout << " " << p;
  std::cout << "\n";
}

int main() {
  // 1. Equal scalar bounds (3 dimensions, all [-5.0, 5.0]), default config.
  GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, sphere);
  std::cout << "sphere, equal bounds:\n";
  print_result(gsa1.optimize());

  // 2. Per-dimension bounds (3 dimensions with unique ranges)
  GravitationalSearchAlgorithm gsa2(
      {-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, sphere,
      {.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "sphere, per-dim bounds:\n";
  print_result(gsa2.optimize());

  // 3. Rosenbrock with tuned settings
  GravitationalSearchAlgorithm gsa3(
      10, -2.048, 2.048, rosenbrock,
      {.n_agents = 50, .max_iter = 5000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "rosenbrock, custom config:\n";
  print_result(gsa3.optimize());

  // 4. Lambda objective (Schwefel)
  GravitationalSearchAlgorithm gsa4(
      30, -500, 500,
      [](std::span<const double> x) {
        double sum = 0.0;
        for (double val : x) sum += -val * std::sin(std::sqrt(std::abs(val)));
        return sum;
      },
      {.n_agents = 100, .max_iter = 1000, .g0 = 10.0, .alpha = 20.0});
  std::cout << "schwefel, lambda objective:\n";
  print_result(gsa4.optimize());

  return 0;
}
