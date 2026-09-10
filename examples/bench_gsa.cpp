// Multi-run GSA benchmark: runs Optimize() N times with distinct seeds
// and writes the averaged best-so-far per iteration.
//
// Usage: bench [runs=30] [iters=1000] [agents=30] [dims=2] [lo=-2.048]
//              [hi=2.048] [seed_base=1000] [objective=rosenbrock]
//              [g0=100.0] [alpha=20.0]
// Writes exports/bench/gsa_avg.csv (header iter,avg_best,std_best;
// std is population std, matching scripts/benchmark.py).
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include "gsa/gsa.hpp"

namespace fs = std::filesystem;

namespace {

double Sphere(std::span<const double> x) {
  double s{};
  for (double v : x) s += v * v;
  return s;
}

double Rosenbrock(std::span<const double> x) {
  double s{};
  for (size_t i{}; i + 1 < x.size(); ++i) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

template <typename T>
T Arg(const std::vector<std::string>& args, size_t i, T fallback) {
  if (i < args.size()) {
    if constexpr (std::is_same_v<T, std::string>) {
      return args[i];
    } else if constexpr (std::is_floating_point_v<T>) {
      return std::stod(args[i]);
    } else {
      return static_cast<T>(std::stoull(args[i]));
    }
  }
  return fallback;
}

}  // namespace

int main(int argc, char** argv) {
  const std::vector<std::string> args(argv + 1, argv + argc);
  const size_t runs{Arg<size_t>(args, 0, 30)};
  const size_t iters{Arg<size_t>(args, 1, 1000)};
  const size_t agents{Arg<size_t>(args, 2, 30)};
  const size_t dims{Arg<size_t>(args, 3, 2)};
  const double lo{Arg<double>(args, 4, -2.048)};
  const double hi{Arg<double>(args, 5, 2.048)};
  const uint64_t seed_base{Arg<uint64_t>(args, 6, 1000)};
  const std::string objective{Arg<std::string>(args, 7, "rosenbrock")};
  const double g0{Arg<double>(args, 8, 100.0)};
  const double alpha{Arg<double>(args, 9, 20.0)};
  auto fn = objective == "sphere"
                ? static_cast<double (*)(std::span<const double>)>(Sphere)
                : static_cast<double (*)(std::span<const double>)>(Rosenbrock);

  std::vector<double> sum(iters + 1, 0.0);
  std::vector<double> sumsq(iters + 1, 0.0);
  for (size_t r{}; r < runs; ++r) {
    gsa::GravitationalSearchAlgorithm gsa(
        dims, lo, hi, fn,
        {.n_agents = agents,
         .max_iter = iters,
         .g0 = g0,
         .alpha = alpha,
         .minimize = true,
         .seed = seed_base + r});
    const auto res{gsa.Optimize()};
    for (size_t i{}; i <= iters; ++i) {
      const double b{res.history[i].best_so_far};
      sum[i] += b;
      sumsq[i] += b * b;
    }
    std::cout << "run " << (r + 1) << "/" << runs
              << " best=" << res.best_val << "\n";
  }

  fs::create_directories("exports/bench");
  std::ofstream out{"exports/bench/gsa_avg.csv"};
  out << "iter,avg_best,std_best\n";
  for (size_t i{}; i <= iters; ++i) {
    const double mean{sum[i] / runs};
    const double var{std::max(sumsq[i] / runs - mean * mean, 0.0)};
    out << i << ',' << mean << ',' << std::sqrt(var) << '\n';
  }
  std::cout << "wrote exports/bench/gsa_avg.csv\n";
  return 0;
}
