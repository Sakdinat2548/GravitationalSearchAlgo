#include <thread>
#include <vector>

#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

TEST(thread_safety) {
  const auto cfg = config();
  GravitationalSearchAlgorithm gsa(8, -5.0, 5.0, sphere, cfg);

  const int n_threads = 8;
  std::vector<GsaResult> results(n_threads);
  std::vector<std::thread> threads;
  threads.reserve(n_threads);

  for (int t = 0; t < n_threads; ++t) {
    threads.emplace_back([&gsa, &results, t] { results[t] = gsa.optimize(); });
  }
  for (auto& th : threads) th.join();

  bool ok = true;
  const double best = results[0].best_val;
  for (const auto& r : results) {
    if (r.best_val != best) {
      expect(false, "concurrent optimize() results differ");
      ok = false;
    }
  }
  return ok;
}
