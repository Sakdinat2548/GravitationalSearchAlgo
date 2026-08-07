#ifndef GSA_HPP
#define GSA_HPP

#include <cstddef>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

struct GsaConfig {
    int n_agents = 40;
    int max_iter = 500;
    double g0 = 100.0;
    double alpha = 20.0;
    bool minimize = true;
};

using ObjectiveFunction = std::function<double(const std::vector<double>&)>;

class GravitationalSearchAlgorithm {
   private:
    GsaConfig config{};
    int dimensions{0};
    std::vector<double> min_bounds;
    std::vector<double> max_bounds;
    ObjectiveFunction objective_fn;

    // Physical state vectors
    std::vector<double> X;
    std::vector<double> V;
    std::vector<double> A;

    // Pre-allocated scratchpad buffers
    std::vector<double> fitness;
    std::vector<double> M;
    std::vector<double> agent_buffer;
    std::vector<double> total_F;
    std::vector<int> sorted_indices;

    /** Compute the 1D storage index for agent `agent` and dimension `dim`. */
    inline constexpr size_t idx(size_t agent, size_t dim) const noexcept;

    /**
     * Save current particle positions to a whitespace-separated text file.
     * Each line is one agent; columns correspond to dimensions.
     */
    void save_positions_to_file(const std::string& filename) const;

    /** Initialize agent positions uniformly at random between bounds. */
    void initialize_positions(std::mt19937& gen);

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
    void compute_accelerations(
        const double G, const int current_iter, std::mt19937& gen,
        std::uniform_real_distribution<double>& rand_uni);

    /** Update velocities, move agents, and clamp positions to bounds. */
    void update_kinematics(std::mt19937& gen,
                           std::uniform_real_distribution<double>& rand_uni);

   public:
    GravitationalSearchAlgorithm(
        const std::vector<double>& lower, const std::vector<double>& upper,
        const ObjectiveFunction& func, const GsaConfig& cfg = GsaConfig{});

    GravitationalSearchAlgorithm(
        int dims, double lower, double upper, const ObjectiveFunction& func,
        const GsaConfig& cfg = GsaConfig{});

    [[nodiscard]] std::pair<double, std::vector<double>> optimize();
};

#endif  // GSA_HPP`