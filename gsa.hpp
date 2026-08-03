#ifndef GSA_HPP
#define GSA_HPP

#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

struct GsaConfig {
    int n_agents = 40;
    int max_iter = 500;
    double g0 = 100.0;
    double alpha = 20.0;
    bool minimize = true;
};

class GravitationalSearchAlgorithm {
private:
    GsaConfig config;
    int dimensions;
    std::vector<double> min_bounds;
    std::vector<double> max_bounds;
    std::function<double(const std::vector<double>&)> objective_fn;

    std::vector<double> X; 
    std::vector<double> V;
    std::vector<double> A;

    inline constexpr size_t idx(size_t agent, size_t dim) const noexcept {
        return agent * dimensions + dim;
    }

public:
    GravitationalSearchAlgorithm(
        const std::vector<double>& lower, 
        const std::vector<double>& upper, 
        std::function<double(const std::vector<double>&)> func,
        GsaConfig cfg = GsaConfig{})
        : min_bounds(lower), max_bounds(upper), objective_fn(func), config(cfg) {
        
        dimensions = static_cast<int>(lower.size());
        X.resize(config.n_agents * dimensions);
        V.resize(config.n_agents * dimensions, 0.0);
        A.resize(config.n_agents * dimensions, 0.0);
    }

    GravitationalSearchAlgorithm(
        int dims, double lower, double upper, 
        std::function<double(const std::vector<double>&)> func,
        GsaConfig cfg = GsaConfig{})
        : GravitationalSearchAlgorithm(
            std::vector<double>(dims, lower), 
            std::vector<double>(dims, upper), 
            func, cfg) {}

    std::pair<double, std::vector<double>> optimize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> rand_uni(0.0, 1.0);

        // random distributions for each dimension
        std::vector<std::uniform_real_distribution<double>> rand_space;
        rand_space.reserve(dimensions);
        for (int d = 0; d < dimensions; ++d) {
            rand_space.emplace_back(min_bounds[d], max_bounds[d]);
        }

        // Initialize positions
        for (int i = 0; i < config.n_agents; ++i) {
            for (int d = 0; d < dimensions; ++d) {
                X[idx(i, d)] = rand_space[d](gen);
            }
        }

        std::vector<double> fitness(config.n_agents);
        std::vector<double> M(config.n_agents);
        std::vector<double> agent_buffer(dimensions);
        std::vector<double> total_F(dimensions);

        double global_best_val = config.minimize ? std::numeric_limits<double>::max() 
                                                 : std::numeric_limits<double>::lowest();
        std::vector<double> global_best_pos(dimensions);

        const double inv_max_iter = 1.0 / static_cast<double>(config.max_iter);

        for (int k = 1; k <= config.max_iter; ++k) {
            double G = config.g0 * std::exp(-config.alpha * static_cast<double>(k) * inv_max_iter);

            // 1. Evaluate Fitness
            for (int i = 0; i < config.n_agents; ++i) {
                for (int d = 0; d < dimensions; ++d) {
                    agent_buffer[d] = X[idx(i, d)];
                }
                fitness[i] = objective_fn(agent_buffer);

                bool is_better = config.minimize ? (fitness[i] < global_best_val) 
                                                 : (fitness[i] > global_best_val);

                if (is_better) {
                    global_best_val = fitness[i];
                    global_best_pos = agent_buffer;
                }
            }

            // Single pass min and max extraction
            auto [min_it, max_it] = std::minmax_element(fitness.begin(), fitness.end());
            double min_fit = *min_it;
            double max_fit = *max_it;

            double best_fit  = config.minimize ? min_fit : max_fit;
            double worst_fit = config.minimize ? max_fit : min_fit;

            double fit_diff = best_fit - worst_fit;
            if (std::abs(fit_diff) < 1e-12) fit_diff = 1e-6;

            // 2. Compute Relative Mass
            double sum_q = 0.0;
            for (int i = 0; i < config.n_agents; ++i) {
                M[i] = (fitness[i] - worst_fit) / fit_diff;
                sum_q += M[i];
            }
            if (sum_q == 0.0) sum_q = 1e-6;
            
            double inv_sum_q = 1.0 / sum_q;
            for (int i = 0; i < config.n_agents; ++i) {
                M[i] *= inv_sum_q;
            }

            // 3. Compute Forces & Acceleration
            std::fill(A.begin(), A.end(), 0.0);
            for (int i = 0; i < config.n_agents; ++i) {
                std::fill(total_F.begin(), total_F.end(), 0.0);
                const double m_i = M[i];

                for (int j = 0; j < config.n_agents; ++j) {
                    if (i == j) continue;

                    double r_squared = 0.0;
                    for (int d = 0; d < dimensions; ++d) {
                        double diff = X[idx(i, d)] - X[idx(j, d)];
                        r_squared += diff * diff;
                    }

                    double R = std::sqrt(r_squared);
                    double force_mag = G * (m_i * M[j]) / (R + 1e-6);

                    for (int d = 0; d < dimensions; ++d) {
                        total_F[d] += rand_uni(gen) * force_mag * (X[idx(j, d)] - X[idx(i, d)]);
                    }
                }

                double inv_mass = 1.0 / (m_i + 1e-6);
                for (int d = 0; d < dimensions; ++d) {
                    A[idx(i, d)] = total_F[d] * inv_mass;
                }
            }

            // 4. Update Kinematics
            for (int i = 0; i < config.n_agents; ++i) {
                for (int d = 0; d < dimensions; ++d) {
                    size_t index = idx(i, d);
                    V[index] = rand_uni(gen) * V[index] + A[index];
                    X[index] = std::clamp(X[index] + V[index], min_bounds[d], max_bounds[d]);
                }
            }
        }

        return {global_best_val, global_best_pos};
    }
};

#endif // GSA_HPP