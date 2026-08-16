#include "gsa/gsa.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>

using std::views::iota;

static constexpr double kEpsilon{1e-12};

// Fast uniform random generator using top 53 bits
static inline double RandUni(RandomEngine& gen, double min,
                             double max) noexcept {
  const double scale{0x1.0p-53};  // 2^-53
  return min + (((gen() >> 11) * scale) * (max - min));
}

constexpr size_t GravitationalSearchAlgorithm::Idx(size_t agent,
                                                   size_t dim) const noexcept {
  return (agent * dimensions_) + dim;
}

void GravitationalSearchAlgorithm::InitializePositions(
    IterationState& s, RandomEngine& gen) const {
  for (auto i : iota(0ULL, config_.n_agents)) {
    const size_t offset{Idx(i, 0)};
    for (auto d : iota(0ULL, dimensions_)) {
      s.position[offset + d] = RandUni(gen, min_bounds_[d], max_bounds_[d]);
    }
  }
}

void GravitationalSearchAlgorithm::EvaluateFitness(
    IterationState& s, double& global_best_val,
    std::vector<double>& global_best_pos) const {
  for (auto i : iota(0ULL, config_.n_agents)) {
    const size_t offset{Idx(i, 0)};
    const double fitness_value{
        objective_fn_({&s.position[offset], dimensions_})};
    s.fitness[i] = fitness_value;

    const bool is_better{config_.minimize ? (fitness_value < global_best_val)
                                          : (fitness_value > global_best_val)};
    if (is_better) {
      global_best_val = fitness_value;
      global_best_pos.assign(&s.position[offset],
                             &s.position[offset + dimensions_]);
    }
  }
}

void GravitationalSearchAlgorithm::ComputeMasses(IterationState& s) const {
  auto [min_it, max_it] = std::ranges::minmax_element(s.fitness);
  const double min_fit{*min_it};
  const double max_fit{*max_it};

  const double fit_diff{std::max(max_fit - min_fit, kEpsilon)};
  const double inv_fit_diff{1.0 / fit_diff};

  double sum_q{};
  for (auto i : iota(0ULL, config_.n_agents)) {
    s.mass[i] = (config_.minimize ? (max_fit - s.fitness[i])
                                  : (s.fitness[i] - min_fit)) *
                inv_fit_diff;
    // Ensure mass is non-negative
    s.mass[i] = std::max(s.mass[i], 0.0);
    sum_q += s.mass[i];
  }
  sum_q = std::max(sum_q, kEpsilon);

  const double inv_sum_q{1.0 / sum_q};
  for (auto i : iota(0ULL, config_.n_agents)) {
    s.mass[i] *= inv_sum_q;
  }
}

void GravitationalSearchAlgorithm::ComputeAccelerations(
    IterationState& s, size_t current_iter, RandomEngine& gen) const {
  const double inv_max_iter{1.0 / config_.max_iter};
  const double gravitational_const{
      config_.g0 * std::exp(-config_.alpha * inv_max_iter * current_iter)};
  const double progress{current_iter * inv_max_iter};

  const bool is_small_problem{config_.n_agents <= 10};
  const double k_best_fraction =
      config_.n_agents - (progress * (config_.n_agents - 1));
  const size_t k_best_raw{is_small_problem ? config_.n_agents
                                           : static_cast<size_t>(k_best_fraction)};
  size_t k_best_count{std::clamp(k_best_raw, size_t{1}, config_.n_agents)};

  // Sort/partition agent indices based on fitness (best first)
  std::iota(s.sorted_indices.begin(), s.sorted_indices.end(), 0);
  auto comp = [minimizing = config_.minimize, &f = s.fitness](size_t a,
                                                              size_t b) {
    return minimizing ? (f[a] < f[b]) : (f[a] > f[b]);
  };

  if (is_small_problem) {
    std::ranges::sort(s.sorted_indices, comp);
  } else {
    std::ranges::nth_element(s.sorted_indices,
                             s.sorted_indices.begin() + k_best_count, comp);
  }

  for (auto i : iota(0ULL, config_.n_agents)) {
    const size_t i_offset{Idx(i, 0)};
    const double* x_i{s.position.data() + i_offset};
    const double m_i{s.mass[i]};

    std::ranges::fill(s.total_force, 0.0);

    for (auto k : iota(0ULL, k_best_count)) {
      const size_t j{s.sorted_indices[k]};
      if (i == j) continue;

      const double* x_j{s.position.data() + Idx(j, 0)};

      double r_squared{};
      for (auto d : iota(0ULL, dimensions_)) {
        const double diff{x_i[d] - x_j[d]};
        r_squared += diff * diff;
      }

      const double distance{std::sqrt(r_squared)};
      const double force_mag{
          gravitational_const * (m_i * s.mass[j]) / (distance + kEpsilon)};

      for (auto d : iota(0ULL, dimensions_)) {
        s.total_force[d] +=
            RandUni(gen, 0.0, 1.0) * force_mag * (x_j[d] - x_i[d]);
      }
    }

    const double inv_mass{1.0 / (m_i + kEpsilon)};
    for (auto d : iota(0ULL, dimensions_)) {
      s.acceleration[i_offset + d] = s.total_force[d] * inv_mass;
    }
  }
}

void GravitationalSearchAlgorithm::UpdateKinematics(IterationState& s,
                                                    RandomEngine& gen) const {
  for (auto i : iota(0ULL, config_.n_agents)) {
    const size_t offset{Idx(i, 0)};
    for (auto d : iota(0ULL, dimensions_)) {
      const size_t index{offset + d};
      s.velocity[index] =
          (RandUni(gen, 0.0, 1.0) * s.velocity[index]) + s.acceleration[index];
      s.position[index] = std::clamp(s.position[index] + s.velocity[index],
                                     min_bounds_[d], max_bounds_[d]);
    }
  }
}

void GravitationalSearchAlgorithm::ValidateInputs(
    const std::vector<double>& lower, const std::vector<double>& upper,
    const GsaConfig& cfg) {
  if (lower.empty() || upper.empty()) {
    throw std::invalid_argument("Bounds vectors must not be empty");
  }
  if (lower.size() != upper.size()) {
    throw std::invalid_argument(
        "Lower and upper bounds must have the same size");
  }
  if (cfg.n_agents == 0 || cfg.max_iter == 0) {
    throw std::invalid_argument("n_agents and max_iter must be positive");
  }
  for (auto i : iota(0ULL, lower.size())) {
    if (lower[i] > upper[i]) {
      throw std::invalid_argument(
          "Each lower bound must be <= its upper bound");
    }
  }
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    std::vector<double> lower, std::vector<double> upper,
    ObjectiveFunction func, GsaConfig cfg)
    : config_(std::move(cfg)),
      min_bounds_(std::move(lower)),
      max_bounds_(std::move(upper)),
      objective_fn_(std::move(func)) {
  dimensions_ = min_bounds_.size();

  ValidateInputs(min_bounds_, max_bounds_, config_);
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    int dims, double lower, double upper, ObjectiveFunction func, GsaConfig cfg)
    : GravitationalSearchAlgorithm(
          dims > 0 ? std::vector<double>(dims, lower)
                   : throw std::invalid_argument("dims must be positive"),
          dims > 0 ? std::vector<double>(dims, upper) : std::vector<double>{},
          std::move(func), std::move(cfg)) {}

GsaResult GravitationalSearchAlgorithm::Optimize() const {
  const size_t total_size{config_.n_agents * dimensions_};
  IterationState s;
  s.position.resize(total_size);
  s.velocity.resize(total_size, 0.0);
  s.acceleration.resize(total_size, 0.0);
  s.fitness.resize(config_.n_agents);
  s.mass.resize(config_.n_agents);
  s.total_force.resize(dimensions_);
  s.sorted_indices.resize(config_.n_agents);

  std::random_device rd;
  RandomEngine gen(config_.seed != 0 ? config_.seed : rd());

  double global_best_val{config_.minimize
                             ? std::numeric_limits<double>::max()
                             : std::numeric_limits<double>::lowest()};
  std::vector<double> global_best_pos(dimensions_);

  auto record_iteration = [&]() -> GsaIterationInfo {
    const auto& f = s.fitness;
    const size_t n{f.size()};
    const auto [min_it, max_it] = std::ranges::minmax_element(f);
    std::iota(s.sorted_indices.begin(), s.sorted_indices.end(), 0);
    auto comp = [&](size_t a, size_t b) {
      return config_.minimize ? f[a] < f[b] : f[a] > f[b];
    };
    std::ranges::nth_element(s.sorted_indices, s.sorted_indices.begin() + n / 2,
                             comp);
    double sm{};
    double sq{};
    for (double x : f) {
      sm += x;
      sq += x * x;
    }
    const double m{sm / n};
    const double median{
        (f[s.sorted_indices[(n - 1) / 2]] + f[s.sorted_indices[n / 2]]) / 2};
    return {.best_so_far = global_best_val,
            .best_iter = config_.minimize ? *min_it : *max_it,
            .worst_iter = config_.minimize ? *max_it : *min_it,
            .mean_fitness = m,
            .median_fitness = median,
            .stddev_fitness = std::sqrt(std::max(0.0, sq / n - m * m))};
  };

  GsaResult result{};
  result.history.reserve(config_.max_iter + 1);

  InitializePositions(s, gen);

  for (auto k : iota(1ULL, config_.max_iter + 1)) {
    EvaluateFitness(s, global_best_val, global_best_pos);
    result.history.push_back(record_iteration());
    ComputeMasses(s);
    ComputeAccelerations(s, k, gen);
    UpdateKinematics(s, gen);
  }

  EvaluateFitness(s, global_best_val, global_best_pos);
  result.history.push_back(record_iteration());

  result.best_val = global_best_val;
  result.best_pos = std::move(global_best_pos);
  return result;
}
