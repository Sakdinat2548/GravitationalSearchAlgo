#include "../test_common.hpp"
#include "../test_framework.hpp"

#include <numeric>
#include <vector>

TEST(median_correctness) {
  bool ok{true};
  const auto expect = [&](const std::vector<double>& f, bool minimize,
                          double expected) {
    std::vector<size_t> idx(f.size());
    const FitnessStats stats{ComputeFitnessStats(f, minimize, idx)};
    if (stats.median != expected) {
      ok = false;
      gsa_test::Expect(false, "median mismatch");
    }
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
  return ok;
}
