#include "gsa.hpp"

#include <algorithm>
#include <cmath>
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
  return agent * dimensions + dim;
}

void GravitationalSearchAlgorithm::initialize_positions(
    IterationState& s, random_engine_t& gen) const {
  for (int i = 0; i < config.n_agents; ++i) {
    const size_t offset = idx(i, 0);
    for (size_t d = 0; d < dimensions; ++d) {
      s.X[offset + d] = rand_uni(gen, min_bounds[d], max_bounds[d]);
    }
  }
}

void GravitationalSearchAlgorithm::evaluate_fitness(
    IterationState& s, double& global_best_val,
    std::vector<double>& global_best_pos) const {
  for (int i = 0; i < config.n_agents; ++i) {
    const size_t offset = idx(i, 0);
    const double fitness_value = objective_fn({&s.X[offset], dimensions});
    s.fitness[i] = fitness_value;

    const bool is_better = config.minimize ? (fitness_value < global_best_val)
                                           : (fitness_value > global_best_val);
    if (is_better) {
      global_best_val = fitness_value;
      global_best_pos.assign(&s.X[offset], &s.X[offset + dimensions]);
    }
  }
}

void GravitationalSearchAlgorithm::compute_masses(IterationState& s) const {
  auto [min_it, max_it] = std::ranges::minmax_element(s.fitness);
  const double min_fit = *min_it;
  const double max_fit = *max_it;

  const double fit_diff = std::max(max_fit - min_fit, kEpsilon);
  const double inv_fit_diff = 1.0 / fit_diff;

  double sum_q = 0.0;
  for (int i = 0; i < config.n_agents; ++i) {
    s.M[i] = std::max((config.minimize ? (max_fit - s.fitness[i])
                                       : (s.fitness[i] - min_fit)) *
                          inv_fit_diff,
                      0.0);
    sum_q += s.M[i];
  }
  if (sum_q < kEpsilon) sum_q = kEpsilon;

  const double inv_sum_q = 1.0 / sum_q;
  for (int i = 0; i < config.n_agents; ++i) {
    s.M[i] *= inv_sum_q;
  }
}

void GravitationalSearchAlgorithm::compute_accelerations(
    IterationState& s, int current_iter, random_engine_t& gen) const {
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
  std::iota(s.sorted_indices.begin(), s.sorted_indices.end(), 0);
  auto comp = [minimizing = config.minimize, &f = s.fitness](int a, int b) {
    return minimizing ? (f[a] < f[b]) : (f[a] > f[b]);
  };

  if (!is_small_problem) {
    std::ranges::nth_element(s.sorted_indices,
                             s.sorted_indices.begin() + k_best_count, comp);
  } else {
    // small problem: keep full ordering
    std::ranges::sort(s.sorted_indices, comp);
  }

  for (int i = 0; i < config.n_agents; ++i) {
    const size_t i_offset = idx(i, 0);
    const double* x_i = s.X.data() + i_offset;
    const double m_i = s.M[i];

    std::ranges::fill(s.total_F, 0.0);

    for (int k = 0; k < k_best_count; ++k) {
      const int j = s.sorted_indices[k];
      if (i == j) continue;

      const double* x_j = s.X.data() + idx(j, 0);

      double r_squared = 0.0;
      for (size_t d = 0; d < dimensions; ++d) {
        const double diff = x_i[d] - x_j[d];
        r_squared += diff * diff;
      }

      const double R = std::sqrt(r_squared);
      const double force_mag = G * (m_i * s.M[j]) / (R + kEpsilon);

      for (size_t d = 0; d < dimensions; ++d) {
        s.total_F[d] += rand_uni(gen, 0.0, 1.0) * force_mag * (x_j[d] - x_i[d]);
      }
    }

    const double inv_mass = 1.0 / (m_i + kEpsilon);
    for (size_t d = 0; d < dimensions; ++d) {
      s.A[i_offset + d] = s.total_F[d] * inv_mass;
    }
  }
}

void GravitationalSearchAlgorithm::update_kinematics(
    IterationState& s, random_engine_t& gen) const {
  for (int i = 0; i < config.n_agents; ++i) {
    const size_t offset = idx(i, 0);
    for (size_t d = 0; d < dimensions; ++d) {
      const size_t index = offset + d;
      s.V[index] = rand_uni(gen, 0.0, 1.0) * s.V[index] + s.A[index];
      s.X[index] =
          std::clamp(s.X[index] + s.V[index], min_bounds[d], max_bounds[d]);
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
  dimensions = min_bounds.size();

  validate_inputs(min_bounds, max_bounds, config);
}

GravitationalSearchAlgorithm::GravitationalSearchAlgorithm(
    int dims, double lower, double upper, ObjectiveFunction func, GsaConfig cfg)
    : GravitationalSearchAlgorithm(
          dims > 0 ? std::vector<double>(dims, lower)
                   : throw std::invalid_argument("dims must be positive"),
          dims > 0 ? std::vector<double>(dims, upper) : std::vector<double>{},
          std::move(func), std::move(cfg)) {}

GsaResult GravitationalSearchAlgorithm::optimize() const {
  const size_t total_size = static_cast<size_t>(config.n_agents) * dimensions;
  IterationState s;
  s.X.resize(total_size);
  s.V.resize(total_size, 0.0);
  s.A.resize(total_size, 0.0);
  s.fitness.resize(config.n_agents);
  s.M.resize(config.n_agents);
  s.total_F.resize(dimensions);
  s.sorted_indices.resize(config.n_agents);

  std::random_device rd;
  random_engine_t gen(config.seed != 0 ? config.seed : rd());

  double global_best_val = config.minimize
                               ? std::numeric_limits<double>::max()
                               : std::numeric_limits<double>::lowest();
  std::vector<double> global_best_pos(dimensions);

  auto record_iteration = [&]() -> GsaIterationInfo {
    const auto& f = s.fitness;
    const size_t n = f.size();
    const auto [min_it, max_it] = std::ranges::minmax_element(f);
    std::iota(s.sorted_indices.begin(), s.sorted_indices.end(), 0);
    auto comp = [&](int a, int b) {
      return config.minimize ? f[a] < f[b] : f[a] > f[b];
    };
    std::ranges::nth_element(s.sorted_indices, s.sorted_indices.begin() + n / 2,
                             comp);
    double sm = 0, sq = 0;
    for (double x : f) sm += x, sq += x * x;
    const double m = sm / n;
    const double median =
        (f[s.sorted_indices[(n - 1) / 2]] + f[s.sorted_indices[n / 2]]) / 2;
    return {global_best_val,
            config.minimize ? *min_it : *max_it,
            config.minimize ? *max_it : *min_it,
            m,
            median,
            std::sqrt(std::max(0.0, sq / n - m * m))};
  };

  GsaResult result{};
  result.history.reserve(config.max_iter + 1);

  initialize_positions(s, gen);

  for (int k = 1; k <= config.max_iter; ++k) {
    evaluate_fitness(s, global_best_val, global_best_pos);
    result.history.push_back(record_iteration());
    compute_masses(s);
    compute_accelerations(s, k, gen);
    update_kinematics(s, gen);
  }

  evaluate_fitness(s, global_best_val, global_best_pos);
  result.history.push_back(record_iteration());

  result.best_val = global_best_val;
  result.best_pos = std::move(global_best_pos);
  return result;
}
