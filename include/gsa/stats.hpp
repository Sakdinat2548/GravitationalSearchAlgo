#ifndef GSA_STATS_HPP
#define GSA_STATS_HPP

#include <algorithm>
#include <cmath>
#include <numeric>
#include <span>
#include <vector>

struct FitnessStats {
  double best;
  double worst;
  double mean;
  double median;
  double stddev;
};

inline bool BetterFit(double a, double b, bool minimize) {
  return minimize ? a < b : a > b;
}

[[nodiscard]] inline FitnessStats ComputeFitnessStats(
    std::span<const double> fitness, bool minimize,
    std::span<size_t> sorted_indices) {
  const size_t n{fitness.size()};
  const auto [min_it, max_it] = std::ranges::minmax_element(fitness);
  std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
  const auto comp = [&](size_t a, size_t b) {
    return BetterFit(fitness[a], fitness[b], minimize);
  };
  const size_t mid{n / 2};
  std::ranges::nth_element(sorted_indices, sorted_indices.begin() + mid, comp);
  const double hi{fitness[sorted_indices[mid]]};
  double median{hi};
  if (n % 2 == 0) {
    std::ranges::nth_element(sorted_indices, sorted_indices.begin() + mid - 1,
                             comp);
    median = (fitness[sorted_indices[mid - 1]] + hi) / 2;
  }
  double sm{};
  double sq{};
  for (double x : fitness) {
    sm += x;
    sq += x * x;
  }
  const double m{sm / n};
  return {.best = minimize ? *min_it : *max_it,
          .worst = minimize ? *max_it : *min_it,
          .mean = m,
          .median = median,
          .stddev = std::sqrt(std::max(0.0, sq / n - m * m))};
}

#endif  // GSA_STATS_HPP
