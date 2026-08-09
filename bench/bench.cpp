#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

#include "../gsa.hpp"

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

static double shekel(std::span<const double> x) {
    static constexpr std::array<std::array<double, 25>, 2> aij = {
        {{-32, -16, 0,   16,  32,  -32, -16, 0,   16,  32,  -32, -16, 0,
          16,  32,  -32, -16, 0,   16,  32,  -32, -16, 0,   16,  32},
         {-32, -32, -32, -32, -32, -16, -16, -16, -16, -16, 0,   0,   0,
          0,   0,   16,  16,  16,  16,  16,  32,  32,  32,  32,  32}}};
    double sum = 1.0 / 500.0;
    for (size_t j = 0; j < 25; ++j) {
        double inner = 0.0;
        for (size_t i = 0; i < 2; ++i) {
            const double d = x[i] - aij[i][j];
            inner += d * d * d * d * d * d;
        }
        sum += 1.0 / (j + 1.0 + inner);
    }
    return 1.0 / sum;
}

static void run_case(const char* name, int dims, double lo, double hi,
                     double (*fn)(std::span<const double>)) {
    GsaConfig cfg{.n_agents = 50, .max_iter = 500, .seed = 12345};
    GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);

    auto t0 = std::chrono::steady_clock::now();
    auto res = gsa.optimize();
    auto t1 = std::chrono::steady_clock::now();

    std::cout << name << ": best_val = " << res.best_val
              << "  time = " << std::chrono::duration<double>(t1 - t0).count()
              << "s\n";
    std::cout << "    history size = " << res.history.size() << "\n";

    const auto again = gsa.optimize();
    std::cout << "    deterministic (same seed) = "
              << (again.best_val == res.best_val ? "yes" : "NO") << "\n";
}

int main() {
    run_case("sphere", 8, -5.0, 5.0, sphere);
    run_case("rosenbrock", 8, -2.048, 2.048, rosenbrock);
    run_case("shekel", 2, -65.53, 65.53, shekel);
    return 0;
}