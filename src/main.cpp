#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <span>

#include "gsa/gsa.hpp"
#include "gsa/config.hpp"

static double Rosenbrock(std::span<const double> x) {
  double s{};
  for (auto i : std::views::iota(0ULL, x.size() - 1)) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

static void WriteConfig(const std::string& path) {
  if (std::filesystem::exists(path)) return;
  const std::string json = R"({
    "n_agents": 50,
    "max_iter": 5000,
    "g0": 10.0,
    "alpha": 10.0,
    "minimize": true,
    "seed": 12345
  })";
  std::ofstream(path) << json;
}

static void PrintConfigFile(const std::string& path) {
  std::ifstream file(path);
  if (file) {
    std::cout << file.rdbuf() << "\n";
  }
}

static void PrintResult(const gsa::GsaResult& res,
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
  WriteConfig("config.json");
  PrintConfigFile("config.json");

  gsa::GsaConfig cfg = gsa::LoadConfigFromFile("config.json");
  gsa::GravitationalSearchAlgorithm gsa(10, -2.048, 2.048, Rosenbrock, cfg);

  std::cout << "Press 'r' to reload config from file and re-run.\n";
  std::cout << "Press 'q' to quit.\n\n";

  while (true) {
    std::cout << "Run (current config):\n";
    const auto start{std::chrono::steady_clock::now()};
    PrintResult(gsa.Optimize(), start);

    std::cout << "Command (r=reload, q=quit): ";
    char cmd{};
    std::cin >> cmd;
    if (cmd == 'q') break;
    if (cmd == 'r') {
      std::cout << "Reloading config from file...";
      std::cout << "\n-----\n";
      cfg = gsa::LoadConfigFromFile("config.json");
      gsa = gsa::GravitationalSearchAlgorithm(10, -2.048, 2.048, Rosenbrock, cfg);
      PrintConfigFile("config.json");
    }
    std::cout << "\n";
  }

  return 0;
}
