#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <ranges>
#include <span>
#include <vector>

#include "gsa/gsa.hpp"

static double Sphere(std::span<const double> x) {
  double s{};
  for (double v : x) s += v * v;
  return s;
}

static double Rosenbrock(std::span<const double> x) {
  double s{};
  for (auto i : std::views::iota(0ULL, x.size() - 1)) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

static void PrintResult(const GsaResult& res,
                        std::chrono::steady_clock::time_point start) {
  const double ms{std::chrono::duration<double, std::milli>(
                      std::chrono::steady_clock::now() - start)
                      .count()};
  std::cout << "best_val = " << res.best_val << "  (" << ms << " ms)"
            << "\n  position:";
  for (double p : res.best_pos) std::cout << " " << p;
  std::cout << "\n-----\n";
}

int main() {
  // 1. Equal scalar bounds (3 dimensions, all [-5.0, 5.0]), default config.
  GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, Sphere, {.g0 = 10.0});
  std::cout << "Sphere, equal bounds:\n";
  const auto start1{std::chrono::steady_clock::now()};
  PrintResult(gsa1.Optimize(), start1);

  // 2. Per-dimension bounds (3 dimensions with unique ranges)
  GravitationalSearchAlgorithm gsa2(
      {-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, Sphere,
      {.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "Sphere, per-dim bounds:\n";
  const auto start2{std::chrono::steady_clock::now()};
  PrintResult(gsa2.Optimize(), start2);

  // 3. Rosenbrock with tuned settings
  GravitationalSearchAlgorithm gsa3(
      10, -2.048, 2.048, Rosenbrock,
      {.n_agents = 50, .max_iter = 5000, .g0 = 10.0, .alpha = 10.0});
  std::cout << "Rosenbrock, custom config:\n";
  const auto start3{std::chrono::steady_clock::now()};
  PrintResult(gsa3.Optimize(), start3);

  // 4. Lambda objective (Schwefel)
  GravitationalSearchAlgorithm gsa4(
      30, -500, 500,
      [](std::span<const double> x) {
        double sum{};
        for (double val : x) sum += -val * std::sin(std::sqrt(std::abs(val)));
        return sum;
      },
      {.n_agents = 100, .max_iter = 1000, .g0 = 10.0, .alpha = 20.0});
  std::cout << "schwefel, lambda objective:\n";
  const auto start4{std::chrono::steady_clock::now()};
  PrintResult(gsa4.Optimize(), start4);

  // 5. JSON config file example
  std::cout << "Sphere, JSON config file:\n";
  const std::string json_config = R"({
    "n_agents": 40,
    "max_iter": 500,
    "g0": 10.0,
    "alpha": 20.0,
    "minimize": true,
    "seed": 12345
  })";
  std::ofstream("config.json") << json_config;
  GravitationalSearchAlgorithm gsa5(3, -5.0, 5.0, Sphere, "config.json");
  const auto start5{std::chrono::steady_clock::now()};
  PrintResult(gsa5.Optimize(), start5);

  // 6. Runtime config mutation
  std::cout << "Runtime config mutation:\n";
  GravitationalSearchAlgorithm gsa6(3, -5.0, 5.0, Sphere);
  gsa6.SetG0(5.0).SetAlpha(5.0).SetNAgents(20);
  const auto start6{std::chrono::steady_clock::now()};
  PrintResult(gsa6.Optimize(), start6);

  return 0;
}