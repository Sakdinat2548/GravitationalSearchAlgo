#include <ranges>
#include <thread>
#include <vector>

#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::Config;
using gsa_test::Expect;
using gsa_test::Sphere;

TEST(thread_safety) {
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

  bool ok{true};
  const double best = results[0].best_val;
  for (const auto& r : results) {
    if (r.best_val != best) {
      Expect(false, "concurrent Optimize() results differ");
      ok = false;
    }
  }
  return ok;
}
