#include "gsa.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <cstdint>

static constexpr double kEpsilon = 1e-12;              // small value to avoid div-by-zero
static constexpr int kSmallProblemThreshold = 10;     // population size under which Kbest == N

// Fast conversion of 64-bit random to double in [min, max). Use top 53 bits.
static inline double rand_uni(random_engine_t& gen, double min, double max) noexcept {
    constexpr double scale = 0x1.0p-53; // 2^-53
    return min + static_cast<double>(gen() >> 11) * scale * (max - min);
}

// Helper to compute the 1D index for a 2D agent-dimension array
inline constexpr size_t GravitationalSearchAlgorithm::idx(
    size_t agent, size_t dim) const noexcept {
    return agent * dimensions + dim;
}

/**
 * Save current particle positions to `filename`.
 * Each line contains one agent's coordinates separated by spaces.
 */
void GravitationalSearchAlgorithm::save_positions_to_file(
    const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    const int n_agents = config.n_agents;
    const int dim = dimensions;

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            file << X[idx(i, d)] << (d == dim - 1 ? "" : " ");
        }
        file << "\n";
    }
}

/** Initialize agent positions uniformly at random between `min_bounds` and
 * `max_bounds`. */
void GravitationalSearchAlgorithm::initialize_positions(
    random_engine_t& gen) {
    const int n_agents = config.n_agents;
    const int dim = dimensions;
    const auto& min_b = min_bounds;
    const auto& max_b = max_bounds;

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            X[idx(i, d)] = rand_uni(gen, min_b[d], max_b[d]);
        }
    }
}

/** Evaluate objective function for each agent and update the global best. */
void GravitationalSearchAlgorithm::evaluate_fitness(
    double& global_best_val, std::vector<double>& global_best_pos) {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;
    const int dim = dimensions;
    const auto& obj = objective_fn;

    for (int i = 0; i < n_agents; ++i) {
        const size_t row_offset = idx(i, 0);
        std::copy_n(X.begin() + row_offset, dim, agent_buffer.begin());

        const double fitness_value = obj(std::span<const double>(agent_buffer));
        fitness[i] = fitness_value;

        const bool is_better = minimize ? (fitness_value < global_best_val)
                                        : (fitness_value > global_best_val);
        if (is_better) {
            global_best_val = fitness_value;
            global_best_pos = agent_buffer;
        }
    }
}

/** Compute normalized agent masses from fitness values.
 *  Masses are scaled so that sum(M) == 1 and depend on fitness ranking.
 */
void GravitationalSearchAlgorithm::compute_masses() {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;

    auto [min_it, max_it] = std::minmax_element(fitness.begin(), fitness.end());
    const double min_fit = *min_it;
    const double max_fit = *max_it;

    double fit_diff = std::max(max_fit - min_fit, kEpsilon);

    const double inv_fit_diff = 1.0 / fit_diff;

    double sum_q = 0.0;
    for (int i = 0; i < n_agents; ++i) {
        M[i] = std::max(
            (minimize ? (max_fit - fitness[i]) : (fitness[i] - min_fit)) *
                inv_fit_diff,
            0.0);
        sum_q += M[i];
    }
    if (sum_q < kEpsilon) sum_q = kEpsilon;

    const double inv_sum_q = 1.0 / sum_q;
    for (int i = 0; i < n_agents; ++i) {
        M[i] *= inv_sum_q;
    }
}

/**
 * Compute accelerations for each agent using weighted interactions from the
 * current K-best subset of agents. Kbest decreases linearly from N to 1 as
 * iterations progress (Eq. 21); for very small populations the full set is
 * used.
 */
void GravitationalSearchAlgorithm::compute_accelerations(
    const double G, const int current_iter, random_engine_t& gen) {
    const bool minimize = config.minimize;
    const int n_agents = config.n_agents;
    const int max_iter = config.max_iter;
    const int dim = dimensions;

    std::fill(A.begin(), A.end(), 0.0);

    // Determine Kbest: full population for small problems, otherwise
    // linearly decrease from N to 1 over iterations.
    const bool is_small_problem = n_agents <= kSmallProblemThreshold;
    const double progress =
        static_cast<double>(current_iter) / static_cast<double>(max_iter);
    int k_best_count =
        is_small_problem
            ? n_agents
            : static_cast<int>(n_agents - progress * (n_agents - 1));
    k_best_count = std::clamp(k_best_count, 1, n_agents);

    // Sort/partition agent indices based on fitness (best first)
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    auto comp = [this, minimize](int a, int b) {
        return minimize ? (fitness[a] < fitness[b]) : (fitness[a] > fitness[b]);
    };

    if (k_best_count < n_agents) {
        // Partition so that the first k_best_count indices are the K-best
        // (unordered) ex. nth_element=6; 3, 2, 10, 45, 33, 56, 23, 47
        // -> 33, 2, 10, 23, 3, 45, 47, 56; 45 is the 6th largest, and all
        // elements before it are <= 45, all after are >= 45
        std::nth_element(sorted_indices.begin(),
                         sorted_indices.begin() + k_best_count,
                         sorted_indices.end(), comp);
    } else {
        // small problem: keep full ordering
        std::sort(sorted_indices.begin(), sorted_indices.end(), comp);
    }

    // Compute forces only from agents in the Kbest set (Eq. 21)
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
            const double force_mag = G * (m_i * M[j]) / (R + kEpsilon);

            for (int d = 0; d < dim; ++d) {
                total_F[d] += rand_uni(gen, 0.0, 1.0) * force_mag * (X[idx(j, d)] - X[idx(i, d)]);
            }
        }

        const double inv_mass = 1.0 / (m_i + kEpsilon);
        for (int d = 0; d < dim; ++d) {
            A[idx(i, d)] = total_F[d] * inv_mass;
        }
    }
}

/** Update velocities, move agents, and clamp positions to bounds. */
void GravitationalSearchAlgorithm::update_kinematics(random_engine_t& gen) {
    const int n_agents = config.n_agents;
    const int dim = dimensions;
    const auto& min_b = min_bounds;
    const auto& max_b = max_bounds;

    for (int i = 0; i < n_agents; ++i) {
        for (int d = 0; d < dim; ++d) {
            const size_t index = idx(i, d);
            V[index] = rand_uni(gen, 0.0, 1.0) * V[index] + A[index];
            X[index] = std::clamp(X[index] + V[index], min_b[d], max_b[d]);
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
    sorted_indices.resize(config.n_agents);
}

// Delegating constructor for inline equal bounds (scalar lower and upper)
GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    int dims, double lower, double upper, const ObjectiveFunction& func,
    const GsaConfig& cfg)
    : GravitationalSearchAlgorithm(std::vector<double>(dims, lower),
                                   std::vector<double>(dims, upper), func,
                                   cfg) {}

std::pair<double, std::vector<double>>
GravitationalSearchAlgorithm::optimize() {
    std::random_device rd;
    random_engine_t gen(rd());

    const bool minimize = config.minimize;
    const int max_iter = config.max_iter;
    const double g0 = config.g0;
    const double alpha = config.alpha;

    initialize_positions(gen);
    // save_positions_to_file("initial_positions.txt");

    double global_best_val = minimize ? std::numeric_limits<double>::max()
                                      : std::numeric_limits<double>::lowest();
    std::vector<double> global_best_pos(dimensions);

    const double inv_max_iter = 1.0 / static_cast<double>(max_iter);
    for (int k = 1; k <= max_iter; ++k) {
        // Update gravitational constant G for this iteration (Eq. 28)
        const double G =
            g0 * std::exp(-alpha * static_cast<double>(k) * inv_max_iter);

        evaluate_fitness(global_best_val, global_best_pos);
        compute_masses();
        compute_accelerations(G, k, gen);
        update_kinematics(gen);
    }

    evaluate_fitness(global_best_val, global_best_pos);

    return {global_best_val, global_best_pos};
}
