#include "gsa.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>

static constexpr double kEpsilon = 1e-12;

// Fast uniform random generator using top 53 bits
static inline double rand_uni(random_engine_t& gen, double min,
                              double max) noexcept {
    constexpr double scale = 0x1.0p-53;  // 2^-53
    return min + static_cast<double>(gen() >> 11) * scale * (max - min);
}

inline constexpr size_t GravitationalSearchAlgorithm::idx(
    size_t agent, size_t dim) const noexcept {
    return agent * static_cast<size_t>(dimensions) + dim;
}

void GravitationalSearchAlgorithm::initialize_positions(random_engine_t& gen) {
    for (int i = 0; i < config.n_agents; ++i) {
        const size_t offset = idx(i, 0);
        for (int d = 0; d < dimensions; ++d) {
            X[offset + d] = rand_uni(gen, min_bounds[d], max_bounds[d]);
        }
    }
}

void GravitationalSearchAlgorithm::evaluate_fitness(
    double& global_best_val, std::vector<double>& global_best_pos) {
    for (int i = 0; i < config.n_agents; ++i) {
        const size_t offset = idx(i, 0);
        const double fitness_value =
            objective_fn({&X[offset], static_cast<size_t>(dimensions)});
        fitness[i] = fitness_value;

        const bool is_better = config.minimize
                                   ? (fitness_value < global_best_val)
                                   : (fitness_value > global_best_val);
        if (is_better) {
            global_best_val = fitness_value;
            global_best_pos.assign(&X[offset], &X[offset + dimensions]);
        }
    }
}

void GravitationalSearchAlgorithm::compute_masses() {
    auto [min_it, max_it] = std::ranges::minmax_element(fitness);
    const double min_fit = *min_it;
    const double max_fit = *max_it;

    const double fit_diff = std::max(max_fit - min_fit, kEpsilon);
    const double inv_fit_diff = 1.0 / fit_diff;

    double sum_q = 0.0;
    for (int i = 0; i < config.n_agents; ++i) {
        M[i] = std::max((config.minimize ? (max_fit - fitness[i])
                                         : (fitness[i] - min_fit)) *
                            inv_fit_diff,
                        0.0);
        sum_q += M[i];
    }
    if (sum_q < kEpsilon) sum_q = kEpsilon;

    const double inv_sum_q = 1.0 / sum_q;
    for (int i = 0; i < config.n_agents; ++i) {
        M[i] *= inv_sum_q;
    }
}

void GravitationalSearchAlgorithm::compute_accelerations(const int current_iter,
                                                         random_engine_t& gen) {
    const double inv_max_iter = 1.0 / static_cast<double>(config.max_iter);
    const double G = config.g0 * std::exp(-config.alpha * inv_max_iter *
                                          static_cast<double>(current_iter));
    const double progress = static_cast<double>(current_iter) * inv_max_iter;

    const bool is_small_problem = config.n_agents <= 10;
    int k_best_count = is_small_problem
                           ? config.n_agents
                           : static_cast<int>(config.n_agents -
                                              progress * (config.n_agents - 1));
    k_best_count = std::clamp(k_best_count, 1, config.n_agents);

    // Sort/partition agent indices based on fitness (best first)
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    auto comp = [this](int a, int b) {
        return config.minimize ? (fitness[a] < fitness[b]) : (fitness[a] > fitness[b]);
    };

    if (!is_small_problem) {
        std::ranges::nth_element(sorted_indices,
                                 sorted_indices.begin() + k_best_count, comp);
    } else {
        // small problem: keep full ordering
        std::ranges::sort(sorted_indices, comp);
    }

    for (int i = 0; i < config.n_agents; ++i) {
        const size_t i_offset = idx(i, 0);
        const double m_i = M[i];

        std::ranges::fill(total_F, 0.0);

        for (int k = 0; k < k_best_count; ++k) {
            const int j = sorted_indices[k];
            if (i == j) continue;

            const size_t j_offset = idx(j, 0);

            double r_squared = 0.0;
            for (int d = 0; d < dimensions; ++d) {
                const double diff = X[i_offset + d] - X[j_offset + d];
                r_squared += diff * diff;
            }

            const double R = std::sqrt(r_squared);
            const double force_mag = G * (m_i * M[j]) / (R + kEpsilon);

            for (int d = 0; d < dimensions; ++d) {
                total_F[d] += rand_uni(gen, 0.0, 1.0) * force_mag *
                              (X[j_offset + d] - X[i_offset + d]);
            }
        }

        const double inv_mass = 1.0 / (m_i + kEpsilon);
        for (int d = 0; d < dimensions; ++d) {
            A[i_offset + d] = total_F[d] * inv_mass;
        }
    }
}

void GravitationalSearchAlgorithm::update_kinematics(random_engine_t& gen) {
    for (int i = 0; i < config.n_agents; ++i) {
        const size_t offset = idx(i, 0);
        for (int d = 0; d < dimensions; ++d) {
            const size_t index = offset + d;
            V[index] = rand_uni(gen, 0.0, 1.0) * V[index] + A[index];
            X[index] =
                std::clamp(X[index] + V[index], min_bounds[d], max_bounds[d]);
        }
    }
}

void GravitationalSearchAlgorithm::validate_inputs(
    const std::vector<double>& lower, const std::vector<double>& upper,
    const GsaConfig& cfg) {
    if (lower.empty() || upper.empty()) {
        throw std::invalid_argument("Bounds vectors must not be empty");
    }
    if (lower.size() != upper.size()) {
        throw std::invalid_argument(
            "Lower and upper bounds must have the same size");
    }
    if (cfg.n_agents <= 0 || cfg.max_iter <= 0) {
        throw std::invalid_argument("n_agents and max_iter must be positive");
    }
    for (size_t i = 0; i < lower.size(); ++i) {
        if (lower[i] > upper[i]) {
            throw std::invalid_argument(
                "Each lower bound must be <= its upper bound");
        }
    }
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    std::vector<double> lower, std::vector<double> upper,
    ObjectiveFunction func, GsaConfig cfg)
    : config(std::move(cfg)),
      min_bounds(std::move(lower)),
      max_bounds(std::move(upper)),
      objective_fn(std::move(func)) {
    dimensions = static_cast<int>(min_bounds.size());

    validate_inputs(min_bounds, max_bounds, config);

    const size_t total_size = static_cast<size_t>(config.n_agents) * dimensions;
    X.resize(total_size);
    V.assign(total_size, 0.0);
    A.assign(total_size, 0.0);
    fitness.resize(config.n_agents);
    M.resize(config.n_agents);
    total_F.resize(dimensions);
    sorted_indices.resize(config.n_agents);
    sorted_fitness.resize(config.n_agents);
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    int dims, double lower, double upper, ObjectiveFunction func, GsaConfig cfg)
    : GravitationalSearchAlgorithm(
          dims > 0 ? std::vector<double>(dims, lower)
                   : throw std::invalid_argument("dims must be positive"),
          dims > 0 ? std::vector<double>(dims, upper) : std::vector<double>{},
          std::move(func), std::move(cfg)) {}

GsaResult GravitationalSearchAlgorithm::optimize() {
    std::random_device rd;
    random_engine_t gen(config.seed != 0 ? config.seed : rd());

    initialize_positions(gen);

    double global_best_val = config.minimize
                                 ? std::numeric_limits<double>::max()
                                 : std::numeric_limits<double>::lowest();
    std::vector<double> global_best_pos(dimensions);

    GsaResult result{};
    result.history.reserve(config.max_iter + 1);

    auto record_iteration = [&]() -> GsaIterationInfo {
        auto& v = sorted_fitness;
        v = fitness;
        std::ranges::sort(v);
        double s = 0, sq = 0;
        for (double x : v) s += x, sq += x * x;
        const size_t n = v.size();
        const double m = s / n;
        return {global_best_val, config.minimize ? v.front() : v.back(),
                config.minimize ? v.back() : v.front(), m,
                (v[(n - 1) / 2] + v[n / 2]) / 2,
                std::sqrt(std::max(0.0, sq / n - m * m))};
    };

    for (int k = 1; k <= config.max_iter; ++k) {
        evaluate_fitness(global_best_val, global_best_pos);
        result.history.push_back(record_iteration());
        compute_masses();
        compute_accelerations(k, gen);
        update_kinematics(gen);
    }

    evaluate_fitness(global_best_val, global_best_pos);
    result.history.push_back(record_iteration());

    result.best_val = global_best_val;
    result.best_pos = std::move(global_best_pos);
    return result;
}