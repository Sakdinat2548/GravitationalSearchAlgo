#include <ranges>
#include <vector>

#include <gtest/gtest.h>

#include "gsa/stats.hpp"

TEST(FitnessStats, MedianCorrectness) {
  const auto expect = [](const std::vector<double>& f, bool minimize,
                         double expected) {
    std::vector<size_t> idx(f.size());
    const gsa::FitnessStats stats{gsa::ComputeFitnessStats(f, minimize, idx)};
    EXPECT_DOUBLE_EQ(stats.median, expected) << "median mismatch";
  };

  for (size_t n : {4ULL, 5ULL}) {
    std::vector<double> f;
    for (auto i : std::views::iota(0ULL, n)) f.push_back(static_cast<double>(i));
    const double expected{static_cast<double>(n - 1) / 2.0};
    expect(f, true, expected);
    expect(f, false, expected);
  }

  expect({10.0, 1.0, 5.0, 3.0}, true, 4.0);
  expect({10.0, 1.0, 5.0, 3.0}, false, 4.0);

  const std::vector<double> empty;
  std::vector<size_t> empty_idx;
  EXPECT_THROW((void)gsa::ComputeFitnessStats(empty, true, empty_idx),
               std::invalid_argument)
      << "empty fitness span rejected";

  const std::vector<double> three{3.0, 1.0, 2.0};
  std::vector<size_t> short_idx(2);
  EXPECT_THROW((void)gsa::ComputeFitnessStats(three, true, short_idx),
               std::invalid_argument)
      << "short sorted_indices rejected";
}
