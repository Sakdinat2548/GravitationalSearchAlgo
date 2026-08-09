#include <chrono>
#include <iostream>
#include <vector>

#include "../gsa.hpp"

static double sphere(std::span<const double> x) {
    double s = 0.0;
    for (double v : x) s += v * v;
    return s;
}

int main() {
    // Quick
    GsaConfig quick{.n_agents = 30,
                    .max_iter = 1000,
                    .g0 = 50.0,
                    .alpha = 10.0,
                    .minimize = true};
    GravitationalSearchAlgorithm gsa_quick(5, -5.0, 5.0, sphere, quick);

    auto t0 = std::chrono::high_resolution_clock::now();
    auto res_q = gsa_quick.optimize();
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Quick preset result: " << res_q.best_val
              << " (time: " << std::chrono::duration<double>(t1 - t0).count()
              << "s)\n";
    std::cout << "Position: ";
    for (auto& p : res_q.best_pos) std::cout << p << " ";
    std::cout << "\n";

    // Balanced
    GsaConfig balanced{.n_agents = 80,
                       .max_iter = 2000,
                       .g0 = 50.0,
                       .alpha = 10.0,
                       .minimize = true};
    GravitationalSearchAlgorithm gsa_bal(5, -5.0, 5.0, sphere, balanced);

    t0 = std::chrono::high_resolution_clock::now();
    auto res_b = gsa_bal.optimize();
    t1 = std::chrono::high_resolution_clock::now();

    std::cout << "\n";
    std::cout << "Balanced preset result: " << res_b.best_val
              << " (time: " << std::chrono::duration<double>(t1 - t0).count()
              << "s)\n";
    std::cout << "Position: ";
    for (auto& p : res_b.best_pos) std::cout << p << " ";
    std::cout << "\n";

    return 0;
}
