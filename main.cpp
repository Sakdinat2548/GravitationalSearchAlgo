#include <cmath>
#include <iostream>
#include <vector>

#include "gsa.hpp"

double sphere(std::span<const double> x) {
    double sum = 0.0;
    for (double val : x) sum += val * val;
    return sum;
}

double rosenbrock(std::span<const double> x) {
    double sum = 0.0;
    for (size_t i = 0; i < x.size() - 1; ++i) {
        sum += 100.0 * std::pow(x[i + 1] - std::pow(x[i], 2), 2) +
               std::pow(x[i] - 1, 2);
    }
    return sum;
}

int main() {
    // --- Example: Inline equal bounds (3 dimensions, all [-5.0, 5.0]) ---
    // GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, sphere);
    // auto [val1, pos1] = gsa1.optimize();

    // --- Example: Per-dimension bounds (3 dimensions with unique bounds) ---
    // GravitationalSearchAlgorithm gsa2({-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, sphere);
    // auto [val2, pos2] = gsa2.optimize();

    // std::cout << "Inline Scalar Bounds Result: " << val1 << "\n";
    // std::cout << "Inline Vector Bounds Result: " << val2 << "\n";

    // GravitationalSearchAlgorithm gsa3(10, -2.048, 2.048, rosenbrock,
    //                                   {.n_agents = 50,
    //                                    .max_iter = 5000,
    //                                    .g0 = 10.0,
    //                                    .alpha = 10.0,
    //                                    .minimize = true});
    // auto [val3, pos3] = gsa3.optimize();
    // std::cout << "Rosenbrock Result: " << val3 << "\n";
    // for (auto& p : pos3) std::cout << p << " ";
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
    // auto [val4, pos4] = gsa4.optimize();
    // std::cout << "Schwefel Result: " << val4 << "\n";
    // for (auto& p : pos4) std::cout << p << " ";
    // std::cout << "\n";

    const std::vector<std::vector<double>> aij{
        {-32, -16, 0,   16,  32, -32, -16, 0,   16,  32, -32, -16, 0,
         16,  32,  -32, -16, 0,  16,  32,  -32, -16, 0,  16,  32},
        {-32, -32, -32, -32, -32, -16, -16, -16, -16, -16, 0,  0, 0,
         0,   0,   16,  16,  16,  16,  16,  32,  32,  32,  32, 32}};
    GravitationalSearchAlgorithm gsa5(
        2, -65.53, 65.53,
        [&aij](std::span<const double> x) {
            double sum = 0.0;
            sum += 1.0/500.0;
            for (size_t j = 0; j < 25; ++j) {
                double inner_sum = 0.0;
                for (size_t i = 0; i < 2; ++i) {
                    inner_sum += std::pow(x[i] - aij[i][j], 6);
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
    double avg_val = 0.0;
    for (int i = 0; i < 30; ++i) {
        auto [val5, pos5] = gsa5.optimize();
        avg_val += val5;
        std::cout << "Shekel Result "<< i << ": " << val5 << ", Position: ";
        for (auto& p : pos5) std::cout << p << " ";
        std::cout << "\n";
    }
    std::cout << "Average Shekel Result: " << avg_val / 30.0 << "\n";

    return 0;
}