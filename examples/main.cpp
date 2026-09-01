#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#endif

#include "gsa/gsa.hpp"
#include "gsa/json_io.hpp"
#include "objective.hpp"

namespace fs = std::filesystem;

namespace clr {

constexpr std::string_view kReset{"\033[0m"};
constexpr std::string_view kRed{"\033[31m"};
constexpr std::string_view kGreen{"\033[32m"};
constexpr std::string_view kYellow{"\033[33m"};
constexpr std::string_view kCyan{"\033[36m"};
constexpr std::string_view kDim{"\033[2m"};

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

const fs::path kConfigPath{"config.json"};

static void WriteConfig(const fs::path& path) {
  if (fs::exists(path)) return;
  nlohmann::json j;
  j["dimensions"] = 10;
  j["lower"] = -2.048;
  j["upper"] = 2.048;
  j["n_agents"] = 50;
  j["max_iter"] = 5000;
  j["g0"] = 10.0;
  j["alpha"] = 10.0;
  j["minimize"] = true;
  j["seed"] = 0;
  std::ofstream out{path};
  out << j.dump(2) << "\n";
}

static void PrintConfigFile(const fs::path& path) {
  std::ifstream file{path};
  if (file) {
    std::cout << clr::kCyan << file.rdbuf() << clr::kReset << "\n";
  }
}

// Show the current configuration that actually is being used by the GSA
// instance, including the bounds and other parameters.
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
  std::cout << "]\n  upper=[";
  for (double v : upper) std::cout << v << ' ';
  std::cout << ']' << clr::kReset << "\n";
}

static void WriteHistoryCsv(const gsa::GsaResult& res,
                           const fs::path& path) {
  std::ofstream out{path};
  if (!out) {
    std::cout << clr::kRed << "Failed to open " << path << clr::kReset << "\n";
    return;
  }

  out << "iteration,best_so_far,best_iter,worst_iter,mean_fitness,"
         "median_fitness,stddev_fitness\n";
  for (size_t i = 0; i < res.history.size(); ++i) {
    const auto& h{res.history[i]};
    out << i + 1 << ',' << h.best_so_far << ',' << h.best_iter << ','
        << h.worst_iter << ',' << h.mean_fitness << ',' << h.median_fitness
        << ',' << h.stddev_fitness << '\n';
  }

  std::cout << clr::kGreen << "Saved convergence history to " << path
            << clr::kReset << "\n";
}

static void WriteParticleCsv(const gsa::GsaResult& res,
                            const fs::path& path) {
  if (res.best_pos.size() < 2 || res.agent_positions_by_iteration.empty()) {
    std::cout << clr::kRed << "2D particle export requires at least 2 dimensions"
              << clr::kReset << "\n";
    return;
  }

  std::ofstream out{path};
  if (!out) {
    std::cout << clr::kRed << "Failed to open " << path << clr::kReset << "\n";
    return;
  }

  out << "iteration,agent,x,y,mass,fitness\n";
  const size_t dims{2};
  for (size_t iter = 0; iter < res.agent_positions_by_iteration.size(); ++iter) {
    const auto& snapshot{res.agent_positions_by_iteration[iter]};
    const auto& masses{res.agent_masses_by_iteration[iter]};
    const auto& fits{res.agent_fitness_by_iteration[iter]};
    const size_t n_agents{snapshot.size() / dims};
    for (size_t agent = 0; agent < n_agents; ++agent) {
      const size_t offset{agent * dims};
      out << iter << ',' << agent << ',' << snapshot[offset] << ','
          << snapshot[offset + 1] << ',' << masses[agent] << ','
          << fits[agent] << '\n';
    }
  }

  std::cout << clr::kGreen << "Saved 2D particle positions to " << path
            << clr::kReset << "\n";
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

int main() {
  EnableColors();
  WriteConfig(kConfigPath);
  PrintConfigFile(kConfigPath);

  gsa::GsaConfig cfg;
  gsa::Bounds bounds;
  gsa::GravitationalSearchAlgorithm gsa{
      1, std::vector<double>{-1.0}, std::vector<double>{1.0}, objective::Fn};
  try {
    nlohmann::json j;
    std::ifstream{kConfigPath} >> j;
    cfg = gsa::LoadConfigFromJson(j);
    bounds = gsa::LoadBoundsFromJson(j);
    gsa = gsa::GravitationalSearchAlgorithm(bounds.dimensions, bounds.lower,
                                            bounds.upper, objective::Fn, cfg);
  } catch (const std::exception& e) {
    std::cout << clr::kRed << "Invalid config: " << e.what() << clr::kReset
              << "\n";
    return 1;
  }

  auto run{[&] {
    PrintRunningState(gsa);
    std::cout << "Run (current config):\n";
    const auto start{std::chrono::steady_clock::now()};
    const auto result{gsa.Optimize()};
    WriteHistoryCsv(result, "gsa_history.csv");
    PrintResult(result, start);
  }};

  const auto is_python_command = [](const std::string& line) {
    const std::string trimmed{line};
    return trimmed.rfind("python", 0) == 0 || trimmed == "p" ||
           trimmed == "plot";
  };

  const auto run_2d_visualizer = [] {
    gsa::GravitationalSearchAlgorithm gsa2d(
        2, -10.0, 10.0,
        [](std::span<const double> x) { return (x[0] * x[0]) + (x[1] * x[1]); },
        {.n_agents = 30, .max_iter = 80, .g0 = 20.0, .alpha = 20.0, .seed = 123});
    const auto result{gsa2d.Optimize()};
    WriteParticleCsv(result, "gsa_particles_2d.csv");
    const int status{std::system(
        "python scripts\\animate_gsa_2d.py --input gsa_particles_2d.csv "
        "--output gsa_gravity_2d.gif")};
    if (status != 0) {
      std::cout << clr::kRed << "2D visualizer failed with exit code " << status
                << clr::kReset << "\n";
    } else {
      std::cout << clr::kGreen << "Saved 2D gravitational visualizer to "
                << "gsa_gravity_2d.gif" << clr::kReset << "\n";
    }
  };

  run();
  while (true) {
    std::cout << clr::kYellow
              << "Command (r=reload, p=plot, v=2D visual, q=quit): "
              << clr::kReset;
    std::string line;
    if (!std::getline(std::cin, line)) break;
    const auto first{line.find_first_not_of(" \t\r\n")};
    const std::string cmd{first == std::string::npos
                              ? std::string{}
                              : line.substr(first, 1)};
    if (cmd == "q") break;
    if (cmd == "r") {
      std::cout << clr::kYellow << "Reloading config from file..."
                << clr::kReset << "\n";
      try {
        nlohmann::json j2;
        std::ifstream{kConfigPath} >> j2;
        gsa::GsaConfig cfg2{gsa::LoadConfigFromJson(j2)};
        gsa::Bounds b2{gsa::LoadBoundsFromJson(j2)};
        cfg = cfg2;
        gsa = gsa::GravitationalSearchAlgorithm(b2.dimensions, b2.lower,
                                                b2.upper, objective::Fn, cfg);
        std::cout << clr::kDim << "-----" << clr::kReset << "\n";
        PrintConfigFile(kConfigPath);
      } catch (const std::exception& e) {
        std::cout << clr::kRed
                  << "Reload failed, keeping previous config: " << e.what()
                  << clr::kReset << "\n"
                  << clr::kDim << "-----" << clr::kReset << "\n";
      }

      run();

      continue;
    }
    if (is_python_command(line)) {
      const std::string python_cmd =
          line == "p" || line == "plot"
              ? "python scripts\\plot_gsa_history.py --input gsa_history.csv "
                "--output gsa_convergence.png"
              : line;
      std::cout << clr::kYellow << "Executing: " << python_cmd << clr::kReset
                << "\n";
      const int status{std::system(python_cmd.c_str())};
      if (status != 0) {
        std::cout << clr::kRed << "Plot command failed with exit code "
                  << status << clr::kReset << "\n";
      } else {
        std::cout << clr::kGreen << "Plot completed successfully."
                  << clr::kReset << "\n";
      }
      continue;
    }
    if (cmd == "v") {
      std::cout << clr::kYellow << "Running 2D gravitational visualizer..."
                << clr::kReset << "\n";
      run_2d_visualizer();
      continue;
    }
    std::cout << clr::kRed << "Invalid command '" << line
              << "' — use r, p, v, or q." << clr::kReset << "\n";
  }

  return 0;
}
