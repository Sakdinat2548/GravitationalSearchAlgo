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
  GravitationalSearchAlgorithm gsa(8, -5.0, 5.0, Sphere, cfg);

  constexpr int n_threads{8};
  std::vector<GsaResult> results(n_threads);
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  for (auto t : std::views::iota(0, n_threads)) {
    threads.emplace_back([&gsa, &results, t] { results[t] = gsa.Optimize(); });
  }
  for (auto& th : threads) th.join();

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
