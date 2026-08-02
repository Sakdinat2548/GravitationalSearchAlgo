#include <iostream>
#include <vector>
#include <cmath>
#include "Gsa.hpp"

double sphere(const std::vector<double>& x) {
    double sum = 0.0;
    for (double val : x) sum += std::pow(val, 2);
    return sum;
}

double rosenbrock(const std::vector<double>& x) {
    double sum = 0.0;
    for (size_t i = 0; i < x.size() - 1; ++i) {
        sum += 100.0 * std::pow(x[i + 1] - std::pow(x[i], 2), 2) + std::pow(x[i] - 1, 2);
    }
    return sum;
}

int main() {
    // // --- WAY 1: Inline Equal Bounds (3 dimensions, all [-5.0, 5.0]) ---
    // GravitationalSearchAlgorithm gsa1(3, -5.0, 5.0, sphere);
    // auto [val1, pos1] = gsa1.optimize();


    // // --- WAY 2: Inline Vector List Bounds (3 dimensions with unique bounds) ---
    // GravitationalSearchAlgorithm gsa2({-10.0, 0.0, -1.0}, {10.0, 50.0, 1.0}, sphere);
    // auto [val2, pos2] = gsa2.optimize();

    // std::cout << "Inline Scalar Bounds Result: " << val1 << "\n";
    // std::cout << "Inline Vector Bounds Result: " << val2 << "\n";

    GsaConfig custom_config;
    custom_config.n_agents = 50;
    custom_config.max_iter = 2000;

    GravitationalSearchAlgorithm gsa3(3, -2.048, 2.048, rosenbrock, custom_config);
    auto [val3, pos3] = gsa3.optimize();

    std::cout << "Rosenbrock Result: " << val3 << "\n";
    for(auto &p : pos3) std::cout << p << " ";
    std::cout << "\n";

    return 0;
}