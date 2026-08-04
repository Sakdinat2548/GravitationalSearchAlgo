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

    // Helper to compute the 1D index for a 2D agent-dimension array
    inline constexpr size_t idx(size_t agent, size_t dim) const noexcept;

    // Helper to export particle locations to a text file
    void save_positions_to_file(const std::string& filename) const;

    // Helper 1: Initialize positions randomly across bounds
    void initialize_positions(std::mt19937& gen);

    // Helper 2: Evaluate fitness and update the global best solution
    void evaluate_fitness(double& global_best_val,
                          std::vector<double>& global_best_pos);

    // Helper 3: Normalize agent masses based on fitness performance
    void compute_masses();

    // Helper 4: Compute gravitational forces using Kbest (Eq. 21)
    void compute_accelerations(
        const double G, const int current_iter, std::mt19937& gen,
        std::uniform_real_distribution<double>& rand_uni);

    // Helper 5: Update velocity, move particles, and clamp to bounds
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