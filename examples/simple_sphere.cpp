#include <chrono>
#include <iostream>
#include <vector>

#include "../gsa.hpp"

static double sphere(const std::vector<double>& x) {
    double s = 0.0;
    for (double v : x) s += v * v;
    return s;
}

int main() {
    // Quick preset
    GsaConfig quick{.n_agents = 20,
                    .max_iter = 200,
                    .g0 = 50.0,
                    .alpha = 10.0,
                    .minimize = true};
    GravitationalSearchAlgorithm gsa_quick(5, -5.0, 5.0, sphere, quick);

    auto t0 = std::chrono::high_resolution_clock::now();
    auto [val_q, pos_q] = gsa_quick.optimize();
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Quick preset result: " << val_q
              << " (time: " << std::chrono::duration<double>(t1 - t0).count()
              << "s)\n";
    std::cout << "Position: ";
    for (auto& p : pos_q) std::cout << p << " ";
    std::cout << "\n";

    // Balanced preset
    GsaConfig balanced{.n_agents = 80,
                       .max_iter = 2000,
                       .g0 = 50.0,
                       .alpha = 10.0,
                       .minimize = true};
    GravitationalSearchAlgorithm gsa_bal(5, -5.0, 5.0, sphere, balanced);

    t0 = std::chrono::high_resolution_clock::now();
    auto [val_b, pos_b] = gsa_bal.optimize();
    t1 = std::chrono::high_resolution_clock::now();

    std::cout << "\n";
    std::cout << "Balanced preset result: " << val_b
              << " (time: " << std::chrono::duration<double>(t1 - t0).count()
              << "s)\n";
    std::cout << "Position: ";
    for (auto& p : pos_b) std::cout << p << " ";
    std::cout << "\n";

    return 0;
}
