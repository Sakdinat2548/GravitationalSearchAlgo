#ifndef GSA_HPP
#define GSA_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <ranges>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <xoshiro-cpp/XoshiroCpp.hpp>

#include "gsa/stats.hpp"

namespace gsa {

using RandomEngine = XoshiroCpp::Xoshiro256PlusPlus;

template <std::integral T>
constexpr auto Range(T stop) {
  return std::views::iota(T{}, stop);
}

template <std::integral T, std::integral U>
constexpr auto Range(T start, U stop) {
  return std::views::iota(start, stop);
}

struct GsaConfig {
  size_t n_agents{40};
  size_t max_iter{500};
  double g0{100.0};
  double alpha{20.0};
  bool minimize{true};
  uint64_t seed{};
};

/** Per-iteration snapshot of the agent population. */
struct GsaIterationInfo {
  double best_so_far;     // best value up to and including this iteration
  double best_iter;       // best agent fitness this iteration
  double worst_iter;      // worst agent fitness this iteration
  double mean_fitness;    // mean agent fitness this iteration
  double median_fitness;  // median agent fitness this iteration
  double stddev_fitness;  // std dev of agent fitness this iteration
};

struct GsaResult {
  double best_val;
  std::vector<double> best_pos;
  std::vector<GsaIterationInfo> history;
};

// Fast uniform random generator using top 53 bits
inline double RandUni(RandomEngine& gen, double min, double max) noexcept {
  const double scale{0x1.0p-53};  // 2^-53
  return min + (((gen() >> 11) * scale) * (max - min));
}

constexpr double kEpsilon{1e-12};

template <typename Fn>
  requires std::invocable<Fn, std::span<const double>> &&
           std::convertible_to<
               std::invoke_result_t<Fn, std::span<const double>>, double>
class GravitationalSearchAlgorithm {
 public:
  GravitationalSearchAlgorithm(std::vector<double> lower,
                               std::vector<double> upper, Fn func,
                               GsaConfig cfg = {})
      : GravitationalSearchAlgorithm(ValidatedTag{}, std::move(lower),
                                     std::move(upper), 0, std::move(func),
                                     std::move(cfg)) {}

  GravitationalSearchAlgorithm(int dims, double lower, double upper, Fn func,
                               GsaConfig cfg = {})
      : GravitationalSearchAlgorithm(ValidatedTag{}, FilledBounds(dims, lower),
                                     FilledBounds(dims, upper),
                                     PositiveDims(dims), std::move(func),
                                     std::move(cfg)) {}

  GravitationalSearchAlgorithm(size_t dims, std::vector<double> lower,
                               std::vector<double> upper, Fn func,
                               GsaConfig cfg = {})
      : GravitationalSearchAlgorithm(ValidatedTag{}, std::move(lower),
                                     std::move(upper), dims, std::move(func),
                                     std::move(cfg)) {}

  [[nodiscard]] GsaResult Optimize() const {
    IterationState s{config_.n_agents, dimensions_};

    std::random_device rd;
    RandomEngine gen(config_.seed != 0 ? config_.seed : rd());

    double global_best_val{config_.minimize
                               ? std::numeric_limits<double>::max()
                               : std::numeric_limits<double>::lowest()};
    std::vector<double> global_best_pos(dimensions_);

    GsaResult result{};
    result.history.reserve(config_.max_iter + 1);

    InitializePositions(s, gen);

    for (auto k : Range(1ULL, config_.max_iter + 1)) {
      EvaluateFitness(s, global_best_val, global_best_pos);
      result.history.push_back(RecordIteration(s, global_best_val));
      ComputeMasses(s);
      ComputeAccelerations(s, k, gen);
      UpdateKinematics(s, gen);
    }

    EvaluateFitness(s, global_best_val, global_best_pos);
    result.history.push_back(RecordIteration(s, global_best_val));

    result.best_val = global_best_val;
    result.best_pos = std::move(global_best_pos);
    return result;
  }

  [[nodiscard]] GsaConfig GetConfig() const noexcept { return config_; }

  [[nodiscard]] size_t GetDimensions() const noexcept { return dimensions_; }

  [[nodiscard]] std::span<const double> GetLowerBounds() const noexcept {
    return min_bounds_;
  }

  [[nodiscard]] std::span<const double> GetUpperBounds() const noexcept {
    return max_bounds_;
  }

 private:
  struct ValidatedTag {};

  GravitationalSearchAlgorithm(ValidatedTag, std::vector<double> lower,
                               std::vector<double> upper, size_t expected_dims,
                               Fn func, GsaConfig cfg)
      : config_(std::move(cfg)),
        min_bounds_(std::move(lower)),
        max_bounds_(std::move(upper)),
        objective_fn_(std::move(func)) {
    dimensions_ = min_bounds_.size();
    ValidateConfigs(min_bounds_, max_bounds_, config_, expected_dims);
  }

static void ValidateConfigs(const std::vector<double>& lower,
                              const std::vector<double>& upper,
                              const GsaConfig& cfg, size_t expected_dims) {
    if (lower.empty() || upper.empty()) [[unlikely]] {
      throw std::invalid_argument("Bounds vectors must not be empty");
    }
    if (lower.size() != upper.size()) [[unlikely]] {
      throw std::invalid_argument(
          "Lower and upper bounds must have the same size");
    }
    if (expected_dims != 0 &&
        (lower.size() != expected_dims || upper.size() != expected_dims)) [[unlikely]] {
      throw std::invalid_argument(
          std::format("dims must be positive and match bounds sizes (expected: {}, got lower: {}, upper: {})",
                      expected_dims, lower.size(), upper.size()));
    }
    if (cfg.n_agents == 0 || cfg.max_iter == 0) [[unlikely]] {
      throw std::invalid_argument("n_agents and max_iter must be positive");
    }
    for (auto i : Range(lower.size())) {
      if (lower[i] > upper[i]) [[unlikely]] {
        throw std::invalid_argument(
            std::format("lower[{}] = {} > upper[{}] = {}: lower bound must be <= upper bound",
                        i, lower[i], i, upper[i]));
      }
    }
  }

  static size_t PositiveDims(int dims) {
    if (dims <= 0) throw std::invalid_argument("dims must be positive");
    return static_cast<size_t>(dims);
  }

  static std::vector<double> FilledBounds(int dims, double value) {
    return std::vector<double>(PositiveDims(dims), value);
  }

  /** Per-run working buffers; held by Optimize() so the instance stays const.
   */
  struct IterationState {
    std::unique_ptr<double[]> arena_d;
    std::unique_ptr<size_t[]> arena_i;
    std::span<double> position, velocity, acceleration, fitness, mass,
        total_force;
    std::span<size_t> sorted_indices;

    IterationState(size_t n_agents, size_t dims)
        : arena_d(std::make_unique_for_overwrite<double[]>(
              (3 * n_agents * dims) + (2 * n_agents) + dims)),
          arena_i(std::make_unique_for_overwrite<size_t[]>(n_agents)) {
      double* const base{arena_d.get()};
      position = {base, n_agents * dims};
      velocity = {base + (n_agents * dims), n_agents * dims};
      acceleration = {base + (2 * n_agents * dims), n_agents * dims};
      fitness = {base + (3 * n_agents * dims), n_agents};
      mass = {base + (3 * n_agents * dims) + n_agents, n_agents};
      total_force = {base + (3 * n_agents * dims) + (2 * n_agents), dims};
      sorted_indices = {arena_i.get(), n_agents};
      std::ranges::fill(velocity, 0.0);
      std::ranges::fill(acceleration, 0.0);
    }

    IterationState(const IterationState&) = delete;
    IterationState& operator=(const IterationState&) = delete;
  };

  GsaConfig config_{};
  size_t dimensions_{};
  std::vector<double> min_bounds_;
  std::vector<double> max_bounds_;
  Fn objective_fn_;

  /** Base 1D offset of agent `agent`'s row (start of its position slice). */
  [[nodiscard]] constexpr size_t AgentOffset(size_t agent) const noexcept {
    return agent * dimensions_;
  }

  /** Initialize agent positions uniformly at random between bounds. */
  void InitializePositions(IterationState& s, RandomEngine& gen) const {
    for (auto i : Range(config_.n_agents)) {
      const size_t offset{AgentOffset(i)};
      for (auto d : Range(dimensions_)) {
        s.position[offset + d] = RandUni(gen, min_bounds_[d], max_bounds_[d]);
      }
    }
  }

  /** Evaluate objective for each agent and update the global best. */
  void EvaluateFitness(IterationState& s, double& global_best_val,
                       std::vector<double>& global_best_pos) const {
    for (auto i : Range(config_.n_agents)) {
      const size_t offset{AgentOffset(i)};
      const double fitness_value{
          objective_fn_(s.position.subspan(offset, dimensions_))};
      s.fitness[i] = fitness_value;

      const bool is_better{config_.minimize
                               ? (fitness_value < global_best_val)
                               : (fitness_value > global_best_val)};
      if (is_better) {
        global_best_val = fitness_value;
        const std::span<const double> row{
            s.position.subspan(offset, dimensions_)};
        global_best_pos.assign(row.begin(), row.end());
      }
    }
  }

  /** Compute normalized agent masses from fitness values. */
  void ComputeMasses(IterationState& s) const {
    auto [min_it, max_it] = std::ranges::minmax_element(s.fitness);
    const double min_fit{*min_it};
    const double max_fit{*max_it};

    const double fit_diff{std::max(max_fit - min_fit, kEpsilon)};
    const double inv_fit_diff{1.0 / fit_diff};

    double sum_q{};
    for (auto i : Range(config_.n_agents)) {
      s.mass[i] = (config_.minimize ? (max_fit - s.fitness[i])
                                    : (s.fitness[i] - min_fit)) *
                  inv_fit_diff;
      s.mass[i] = std::max(s.mass[i], 0.0);
      sum_q += s.mass[i];
    }
    sum_q = std::max(sum_q, kEpsilon);

    const double inv_sum_q{1.0 / sum_q};
    for (auto i : Range(config_.n_agents)) {
      s.mass[i] *= inv_sum_q;
    }
  }

  /**
   * Compute accelerations using interactions from the K-best agents.
   * Kbest decreases linearly from N to 1 over iterations.
   */
  void ComputeAccelerations(IterationState& s, size_t current_iter,
                            RandomEngine& gen) const {
    const double inv_max_iter{1.0 / config_.max_iter};
    const double gravitational_const{
        config_.g0 * std::exp(-config_.alpha * inv_max_iter * current_iter)};
    const double progress{current_iter * inv_max_iter};

    const double k_best_fraction =
        config_.n_agents - (progress * (config_.n_agents - 1));
    const size_t k_best_raw{static_cast<size_t>(k_best_fraction)};
    size_t k_best_count{std::clamp(k_best_raw, size_t{1}, config_.n_agents)};

    // Partition agent indices based on fitness (best first)
    std::iota(s.sorted_indices.begin(), s.sorted_indices.end(), 0);
    auto comp = [&](size_t a, size_t b) {
      return BetterFit(s.fitness[a], s.fitness[b], config_.minimize);
    };

    if (k_best_count < config_.n_agents) {
      std::ranges::nth_element(s.sorted_indices,
                               s.sorted_indices.begin() + k_best_count, comp);
    }

    for (auto i : Range(config_.n_agents)) {
      const size_t i_offset{AgentOffset(i)};
      const std::span<const double> x_i{
          s.position.subspan(i_offset, dimensions_)};
      const double m_i{s.mass[i]};

      std::ranges::fill(s.total_force, 0.0);

      const double gmi{gravitational_const * m_i};

      for (auto k : Range(k_best_count)) {
        const size_t j{s.sorted_indices[k]};
        if (i == j) continue;

        const std::span<const double> x_j{
            s.position.subspan(AgentOffset(j), dimensions_)};

        double r_squared{};
        for (auto d : Range(dimensions_)) {
          const double diff{x_i[d] - x_j[d]};
          r_squared += diff * diff;
        }

        const double distance{std::sqrt(r_squared)};
        const double force_mag{gmi * s.mass[j] / (distance + kEpsilon)};

        for (auto d : Range(dimensions_)) {
          s.total_force[d] +=
              RandUni(gen, 0.0, 1.0) * force_mag * (x_j[d] - x_i[d]);
        }
      }

      const double inv_mass{1.0 / (m_i + kEpsilon)};
      for (auto d : Range(dimensions_)) {
        s.acceleration[i_offset + d] = s.total_force[d] * inv_mass;
      }
    }
  }

  /** Update velocities, move agents, and clamp positions to bounds. */
  void UpdateKinematics(IterationState& s, RandomEngine& gen) const {
    for (auto i : Range(config_.n_agents)) {
      const size_t offset{AgentOffset(i)};
      for (auto d : Range(dimensions_)) {
        const size_t index{offset + d};
        s.velocity[index] = (RandUni(gen, 0.0, 1.0) * s.velocity[index]) +
                            s.acceleration[index];
        s.position[index] = std::clamp(s.position[index] + s.velocity[index],
                                       min_bounds_[d], max_bounds_[d]);
      }
    }
  }

  /** Record iteration statistics for history. */
  [[nodiscard]] GsaIterationInfo RecordIteration(IterationState& s,
                                                 double global_best_val) const {
    const FitnessStats stats{
        ComputeFitnessStats(s.fitness, config_.minimize, s.sorted_indices)};
    return {.best_so_far = global_best_val,
            .best_iter = stats.best,
            .worst_iter = stats.worst,
            .mean_fitness = stats.mean,
            .median_fitness = stats.median,
            .stddev_fitness = stats.stddev};
  }
};
}  // namespace gsa

#endif  // GSA_HPP
