#include <array>
#include <cmath>
#include <iostream>
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

int main() {
    // --- Example: Inline equal bounds (3 dimensions, all [-5.0, 5.0]) ---
    // GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, sphere);
    // auto res1 = gsa1.optimize();

    // --- Example: Per-dimension bounds (3 dimensions with unique bounds) ---
    // GravitationalSearchAlgorithm gsa2({-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, sphere);
    // auto res2 = gsa2.optimize();

    // std::cout << "Inline Scalar Bounds Result: " << res1.best_val << "\n";
    // std::cout << "Inline Vector Bounds Result: " << res2.best_val << "\n";

    // GravitationalSearchAlgorithm gsa3(10, -2.048, 2.048, rosenbrock,
    //                                   {.n_agents = 50,
    //                                    .max_iter = 5000,
    //                                    .g0 = 10.0,
    //                                    .alpha = 10.0,
    //                                    .minimize = true});
    // auto res3 = gsa3.optimize();
    // std::cout << "Rosenbrock Result: " << res3.best_val << "\n";
    // for (auto& p : res3.best_pos) std::cout << p << " ";
    // std::cout << "\n";

    // GravitationalSearchAlgorithm gsa4(
    //     30, -500, 500,
    //     [](std::span<const double> x) {
    //         double sum = 0.0;
    //         for (double val : x)
    //             sum += -val * std::sin(std::sqrt(std::abs(val)));
    //         return sum;
    //     },
    //     {.n_agents = 100,
    //      .max_iter = 1000,
    //      .g0 = 10.0,
    //      .alpha = 20.0,
    //      .minimize = true});
    // auto res4 = gsa4.optimize();
    // std::cout << "Schwefel Result: " << res4.best_val << "\n";
    // for (auto& p : res4.best_pos) std::cout << p << " ";
    // std::cout << "\n";

    GravitationalSearchAlgorithm gsa5(
        2, -65.53, 65.53,
        [](std::span<const double> x) noexcept -> double {
            static constexpr std::array<std::array<double, 25>, 2> aij = {
                {{-32, -16, 0,   16,  32, -32, -16, 0,   16,  32, -32, -16, 0,
                  16,  32,  -32, -16, 0,  16,  32,  -32, -16, 0,  16,  32},
                 {-32, -32, -32, -32, -32, -16, -16, -16, -16, -16, 0,  0, 0,
                  0,   0,   16,  16,  16,  16,  16,  32,  32,  32,  32, 32}}};
            double sum = 0.0;
            sum += 1.0/500.0;
            for (size_t j = 0; j < 25; ++j) {
                double inner_sum = 0.0;
                for (size_t i = 0; i < 2; ++i) {
                    const double xi_minus_aij = (x[i] - aij[i][j]);
                    inner_sum += xi_minus_aij * xi_minus_aij * xi_minus_aij *
                                 xi_minus_aij * xi_minus_aij * xi_minus_aij;
                }
                sum += 1.0 / (j + 1 + inner_sum);
            }
            double inv_sum = 1.0 / sum;
            return 1 * inv_sum;
        },
        {.n_agents = 50,
         .max_iter = 500,
         .g0 = 1.0,
         .alpha = 10.0,
         .minimize = true});
    const int n_shekel = 30;
    double avg_val = 0.0;
    for (int i = 0; i < n_shekel; ++i) {
        auto res5 = gsa5.optimize();
        avg_val += res5.best_val;
        std::cout << "Shekel Result " << i << ": " << res5.best_val
                  << ", Position: ";
        for (auto& p : res5.best_pos) std::cout << p << " ";
        std::cout << "\n";
    }
    std::cout << "Average Shekel Result: "
              << avg_val / static_cast<double>(n_shekel) << "\n";

    return 0;
}