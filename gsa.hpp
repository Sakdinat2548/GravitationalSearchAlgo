#ifndef GSA_HPP
#define GSA_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include "XoshiroCpp.hpp"

struct GsaConfig {
    int n_agents = 40;
    int max_iter = 500;
    double g0 = 100.0;
    double alpha = 20.0;
    bool minimize = true;
    uint64_t seed = 0;
};

struct GsaResult {
    double best_val;
    std::vector<double> best_pos;
    std::vector<double> history;
};

using ObjectiveFunction = std::function<double(std::span<const double>)>;

using random_engine_t = XoshiroCpp::Xoshiro256PlusPlus;

class GravitationalSearchAlgorithm {
   private:
    GsaConfig config{};
    int dimensions{0};
    std::vector<double> min_bounds;
    std::vector<double> max_bounds;
    ObjectiveFunction objective_fn;

    std::vector<double> X;
    std::vector<double> V;
    std::vector<double> A;
    std::vector<double> fitness;
    std::vector<double> M;
    std::vector<double> total_F;
    std::vector<int> sorted_indices;

    static void validate_inputs(const std::vector<double>& lower,
                            const std::vector<double>& upper,
                            const GsaConfig& cfg);

    /** Compute the 1D storage index for agent `agent` and dimension `dim`. */
    inline constexpr size_t idx(size_t agent, size_t dim) const noexcept;

    /** Initialize agent positions uniformly at random between bounds. */
    void initialize_positions(random_engine_t& gen);

    /** Evaluate objective for each agent and update the global best. */
    void evaluate_fitness(double& global_best_val,
                          std::vector<double>& global_best_pos);

    /** Compute normalized agent masses from fitness values. */
    void compute_masses();

    /**
     * Compute accelerations using interactions from the K-best agents.
     * Kbest decreases linearly from N to 1 over iterations; `G` is the
     * gravitational constant for the current iteration.
     */
    void compute_accelerations(const int current_iter,
                              random_engine_t& gen);

    /** Update velocities, move agents, and clamp positions to bounds. */
    void update_kinematics(random_engine_t& gen);

   public:
    GravitationalSearchAlgorithm(std::vector<double> lower,
                                 std::vector<double> upper,
                                 ObjectiveFunction func,
                                 GsaConfig cfg = GsaConfig{});

    GravitationalSearchAlgorithm(int dims, double lower, double upper,
                                 ObjectiveFunction func,
                                 GsaConfig cfg = GsaConfig{});

    [[nodiscard]] GsaResult optimize();
};

#endif  // GSA_HPP