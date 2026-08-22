#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <span>

#ifdef _WIN32
#include <windows.h>
#endif

#include "gsa/json_io.hpp"
#include "gsa/gsa.hpp"

namespace clr {

inline constexpr const char* kReset{"\033[0m"};
inline constexpr const char* kRed{"\033[31m"};
inline constexpr const char* kGreen{"\033[32m"};
inline constexpr const char* kYellow{"\033[33m"};
inline constexpr const char* kCyan{"\033[36m"};
inline constexpr const char* kDim{"\033[2m"};

}  // namespace clr

static void EnableColors() {
#ifdef _WIN32
  HANDLE handle{GetStdHandle(STD_OUTPUT_HANDLE)};
  DWORD mode{};
  if (GetConsoleMode(handle, &mode)) {
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
#endif
}

static void WriteConfig(const std::string& path) {
  if (std::filesystem::exists(path)) return;
  const std::string json = R"({
    "dimensions": 10,
    "lower": -2.048,
    "upper": 2.048,
    "n_agents": 50,
    "max_iter": 5000,
    "g0": 10.0,
    "alpha": 10.0,
    "minimize": true,
    "seed": 0
  })";
  std::ofstream(path) << json;
}

static void PrintConfigFile(const std::string& path) {
  std::ifstream file(path);
  if (file) {
    std::cout << clr::kCyan << file.rdbuf() << clr::kReset << "\n";
  }
}

// Show the effective settings from the instance, not the raw file
// (config.json may omit fields, which then fall back to defaults).
template <typename Gsa>
static void PrintRunningState(const Gsa& gsa) {
  const auto cfg{gsa.GetConfig()};
  const auto lower{gsa.GetLowerBounds()};
  const auto upper{gsa.GetUpperBounds()};
  std::cout << clr::kCyan << "running:"
            << "\n  dims=" << gsa.GetDimensions()
            << "  n_agents=" << cfg.n_agents << "  max_iter=" << cfg.max_iter
            << "  g0=" << cfg.g0 << "  alpha=" << cfg.alpha
            << "  minimize=" << (cfg.minimize ? "true" : "false")
            << "  seed=" << cfg.seed << (cfg.seed == 0 ? " (random)" : "")
            << "\n  lower=[";
  for (double v : lower) std::cout << v << ' ';
  std::cout << "] upper=[";
  for (double v : upper) std::cout << v << ' ';
  std::cout << ']' << clr::kReset << "\n";
}

static void PrintResult(const gsa::GsaResult& res,
                        std::chrono::steady_clock::time_point start) {
  const double ms{std::chrono::duration<double, std::milli>(
                      std::chrono::steady_clock::now() - start)
                      .count()};
  std::cout << "best_val = " << clr::kGreen << res.best_val << clr::kReset
            << clr::kDim << "  (" << ms << " ms)" << clr::kReset
            << "\n  position:";
  for (double p : res.best_pos) std::cout << " " << p;
  std::cout << "\n" << clr::kDim << "-----" << clr::kReset << "\n";
}

static double Rosenbrock(std::span<const double> x) {
  double s{};
  for (auto i : std::views::iota(0ULL, x.size() - 1)) {
    const double d{x[i + 1] - (x[i] * x[i])};
    s += (100.0 * d * d) + ((x[i] - 1.0) * (x[i] - 1.0));
  }
  return s;
}

int main() {
  EnableColors();
  WriteConfig("config.json");
  PrintConfigFile("config.json");

  gsa::GsaConfig cfg;
  gsa::Bounds bounds;
  gsa::GravitationalSearchAlgorithm gsa{1, std::vector<double>{-1.0},
                                        std::vector<double>{1.0}, Rosenbrock};
  try {
    nlohmann::json j;
    std::ifstream("config.json") >> j;
    cfg = gsa::LoadConfigFromJson(j);
    bounds = gsa::LoadBoundsFromJson(j);
    gsa = gsa::GravitationalSearchAlgorithm(bounds.dimensions, bounds.lower,
                                            bounds.upper, Rosenbrock, cfg);
    PrintRunningState(gsa);
  } catch (const std::exception& e) {
    std::cout << clr::kRed << "Invalid config: " << e.what() << clr::kReset
              << "\n";
    return 1;
  }

  while (true) {
    std::cout << "Run (current config):\n";
    const auto start{std::chrono::steady_clock::now()};
    PrintResult(gsa.Optimize(), start);

    std::cout << clr::kYellow << "Command (r=reload, q=quit): " << clr::kReset;
    char cmd{};
    std::cin >> cmd;
    if (cmd == 'q') break;
    if (cmd == 'r') {
      std::cout << clr::kYellow << "Reloading config from file..."
                << clr::kReset << "\n";
      try {
        nlohmann::json j2;
        std::ifstream("config.json") >> j2;
        gsa::GsaConfig cfg2{gsa::LoadConfigFromJson(j2)};
        gsa::Bounds b2{gsa::LoadBoundsFromJson(j2)};
        cfg = cfg2;
        gsa = gsa::GravitationalSearchAlgorithm(b2.dimensions, b2.lower,
                                                b2.upper, Rosenbrock, cfg);
        std::cout << clr::kDim << "-----" << clr::kReset << "\n";
        PrintConfigFile("config.json");
        PrintRunningState(gsa);
      } catch (const std::exception& e) {
        std::cout << clr::kRed
                  << "Reload failed, keeping previous config: " << e.what()
                  << clr::kReset << "\n"
                  << clr::kDim << "-----" << clr::kReset;
      }
    }
    std::cout << "\n";
  }

  return 0;
}
