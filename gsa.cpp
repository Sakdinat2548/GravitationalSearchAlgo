#include "gsa.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <stdexcept>

// Helper to compute the 1D index for a 2D agent-dimension array
inline constexpr size_t GravitationalSearchAlgorithm::idx(
    size_t agent, size_t dim) const noexcept {
    return agent * dimensions + dim;
}

// Helper to export particle locations to a text file
void GravitationalSearchAlgorithm::save_positions_to_file(
    const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    for (int i = 0; i < config.n_agents; ++i) {
        for (int d = 0; d < dimensions; ++d) {
            file << X[idx(i, d)] << (d == dimensions - 1 ? "" : " ");
        }
        file << "\n";
    }
}

// Helper 1: Initialize positions randomly across bounds
void GravitationalSearchAlgorithm::initialize_positions(std::mt19937& gen) {
    const int n_agents = config.n_agents;
    const int dim = dimensions;

    std::vector<std::uniform_real_distribution<double>> dists;
    dists.reserve(dim);
    for (int d = 0; d < dim; ++d) {
        dists.emplace_back(min_bounds[d], max_bounds[d]);
    }

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            X[idx(i, d)] = dists[d](gen);
        }
    }
}

// Helper 2: Evaluate fitness and update the global best solution
void GravitationalSearchAlgorithm::evaluate_fitness(
    double& global_best_val, std::vector<double>& global_best_pos) {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;
    const int dim = dimensions;

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            agent_buffer[d] = X[idx(i, d)];
        }

        const double fitness_value = objective_fn(agent_buffer);
        fitness[i] = fitness_value;

        const bool is_better = minimize ? (fitness_value < global_best_val)
                                        : (fitness_value > global_best_val);
        if (is_better) {
            global_best_val = fitness_value;
            global_best_pos = agent_buffer;
        }
    }
}

// Helper 3: Normalize agent masses based on fitness performance
void GravitationalSearchAlgorithm::compute_masses() {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;

    auto [min_it, max_it] = std::minmax_element(fitness.begin(), fitness.end());
    const double min_fit = *min_it;
    const double max_fit = *max_it;

    const double best_fit = minimize ? min_fit : max_fit;
    const double worst_fit = minimize ? max_fit : min_fit;

    double fit_diff = best_fit - worst_fit;
    if (std::abs(fit_diff) < 1e-12) fit_diff = 1e-6;

    const double inv_fit_diff = 1.0 / fit_diff;

    double sum_q = 0.0;
    for (int i = 0; i < n_agents; ++i) {
        M[i] = (fitness[i] - worst_fit) * inv_fit_diff;
        sum_q += M[i];
    }
    if (sum_q == 0.0) sum_q = 1e-6;

    const double inv_sum_q = 1.0 / sum_q;
    for (int i = 0; i < n_agents; ++i) {
        M[i] *= inv_sum_q;
    }
}

// Helper 4: Compute gravitational forces using Kbest (Eq. 21)
void GravitationalSearchAlgorithm::compute_accelerations(
    const double G, const int current_iter, std::mt19937& gen,
    std::uniform_real_distribution<double>& rand_uni) {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;
    const int max_iter = config.max_iter;
    const int dim = dimensions;
    const double epsilon = 1e-6;

    std::fill(A.begin(), A.end(), 0.0);

    // 1. Sort agent indices based on fitness (best fitness first)
    std::vector<int> sorted_indices(n_agents);
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);

    std::sort(sorted_indices.begin(), sorted_indices.end(),
              [this, minimize](int a, int b) {
                  return minimize ? (fitness[a] < fitness[b])
                                  : (fitness[a] > fitness[b]);
              });

    // 2. Linearly decrease Kbest from N down to 1 over max_iter
    const double progress =
        static_cast<double>(current_iter) / static_cast<double>(max_iter);
    int k_best_count = static_cast<int>(n_agents - progress * (n_agents - 1));
    k_best_count = std::clamp(k_best_count, 1, n_agents);

    // 3. Compute forces only from agents in the Kbest set (Eq. 21)
    for (int i = 0; i < n_agents; ++i) {
        std::fill(total_F.begin(), total_F.end(), 0.0);
        const double m_i = M[i];

        for (int k = 0; k < k_best_count; ++k) {
            const int j = sorted_indices[k];

            if (i == j) continue;

            double r_squared = 0.0;
            for (int d = 0; d < dim; ++d) {
                const double diff = X[idx(i, d)] - X[idx(j, d)];
                r_squared += diff * diff;
            }

            const double R = std::sqrt(r_squared);
            const double force_mag = G * (m_i * M[j]) / (R + epsilon);

            for (int d = 0; d < dim; ++d) {
                total_F[d] +=
                    rand_uni(gen) * force_mag * (X[idx(j, d)] - X[idx(i, d)]);
            }
        }

        const double inv_mass = 1.0 / (m_i + epsilon);
        for (int d = 0; d < dim; ++d) {
            A[idx(i, d)] = total_F[d] * inv_mass;
        }
    }
}

// Helper 5: Update velocity, move particles, and clamp to bounds
void GravitationalSearchAlgorithm::update_kinematics(
    std::mt19937& gen, std::uniform_real_distribution<double>& rand_uni) {
    const int n_agents = config.n_agents;
    const int dim = dimensions;

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            const size_t index = idx(i, d);
            V[index] = rand_uni(gen) * V[index] + A[index];
            X[index] =
                std::clamp(X[index] + V[index], min_bounds[d], max_bounds[d]);
        }
    }
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    const std::vector<double>& lower, const std::vector<double>& upper,
    const ObjectiveFunction& func, const GsaConfig& cfg)
    : min_bounds(lower), max_bounds(upper), objective_fn(func), config(cfg) {
    if (min_bounds.empty() || max_bounds.empty()) {
        throw std::invalid_argument("Bounds vectors must not be empty");
    }
    if (min_bounds.size() != max_bounds.size()) {
        throw std::invalid_argument(
            "Lower and upper bounds must have the same size");
    }
    if (config.n_agents <= 0 || config.max_iter <= 0) {
        throw std::invalid_argument("n_agents and max_iter must be positive");
    }
    for (size_t i = 0; i < min_bounds.size(); ++i) {
        if (min_bounds[i] > max_bounds[i]) {
            throw std::invalid_argument(
                "Each lower bound must be <= its upper bound");
        }
    }

    dimensions = static_cast<int>(min_bounds.size());

    X.resize(config.n_agents * dimensions);
    V.resize(config.n_agents * dimensions, 0.0);
    A.resize(config.n_agents * dimensions, 0.0);

    fitness.resize(config.n_agents);
    M.resize(config.n_agents);
    agent_buffer.resize(dimensions);
    total_F.resize(dimensions);
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    int dims, double lower, double upper, const ObjectiveFunction& func,
    const GsaConfig& cfg)
    : GravitationalSearchAlgorithm(std::vector<double>(dims, lower),
                                   std::vector<double>(dims, upper), func,
                                   cfg) {}

std::pair<double, std::vector<double>>
GravitationalSearchAlgorithm::optimize() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> rand_uni(0.0, 1.0);

    initialize_positions(gen);

    // save_positions_to_file("initial_positions.txt");

    const bool minimize = config.minimize;
    const int max_iter = config.max_iter;
    const double inv_max_iter = 1.0 / static_cast<double>(max_iter);

    double global_best_val = minimize ? std::numeric_limits<double>::max()
                                      : std::numeric_limits<double>::lowest();
    std::vector<double> global_best_pos(dimensions);

    for (int k = 1; k <= max_iter; ++k) {
        const double G =
            config.g0 *
            std::exp(-config.alpha * static_cast<double>(k) * inv_max_iter);

        evaluate_fitness(global_best_val, global_best_pos);
        compute_masses();
        compute_accelerations(G, k, gen, rand_uni);
        update_kinematics(gen, rand_uni);
    }

    evaluate_fitness(global_best_val, global_best_pos);

    return {global_best_val, global_best_pos};
}
