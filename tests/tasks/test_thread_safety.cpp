#include <ranges>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::Config;
using gsa_test::Sphere;

TEST(ThreadSafety, ConcurrentOptimizeIdentical) {
  const auto cfg = Config();
  gsa::GravitationalSearchAlgorithm gsa(8, -5.0, 5.0, Sphere, cfg);

  constexpr int kNThreads{8};
  std::vector<gsa::GsaResult> results(kNThreads);
  std::vector<std::jthread> threads;
  threads.reserve(kNThreads);

  for (auto t : std::views::iota(0, kNThreads)) {
    threads.emplace_back([&gsa, &results, t] { results[t] = gsa.Optimize(); });
  }
  for (auto& t : threads) {
    t.join();
  }

  const double best = results[0].best_val;
  for (const auto& r : results) {
    EXPECT_EQ(r.best_val, best) << "concurrent Optimize() results differ";
  }
}
