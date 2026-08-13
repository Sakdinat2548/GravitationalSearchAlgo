#include <cmath>
#include <iostream>
#include <span>
#include <vector>

#include "gsa.hpp"

static double Sphere(std::span<const double> x) {
  double s = 0.0;
  for (double v : x) s += v * v;
  return s;
}

static double Rosenbrock(std::span<const double> x) {
  double s = 0.0;
  for (size_t i = 0; i + 1 < x.size(); ++i) {
    const double d = x[i + 1] - (x[i] * x[i]);
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

static void PrintResult(const GsaResult& res) {
  std::cout << "best_val = " << res.best_val << "\n  position:";
  for (double p : res.best_pos) std::cout << " " << p;
  std::cout << "\n";
}

int main() {
  // 1. Equal scalar bounds (3 dimensions, all [-5.0, 5.0]), default config.
  GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, Sphere);
  std::cout << "Sphere, equal bounds:\n";
  PrintResult(gsa1.Optimize());

  // 2. Per-dimension bounds (3 dimensions with unique ranges)
  GravitationalSearchAlgorithm gsa2(
      {-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, Sphere,
      {.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "Sphere, per-dim bounds:\n";
  PrintResult(gsa2.Optimize());

  // 3. Rosenbrock with tuned settings
  GravitationalSearchAlgorithm gsa3(
      10, -2.048, 2.048, Rosenbrock,
      {.n_agents = 50, .max_iter = 5000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "Rosenbrock, custom config:\n";
  PrintResult(gsa3.Optimize());

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
  PrintResult(gsa4.Optimize());

  return 0;
}
