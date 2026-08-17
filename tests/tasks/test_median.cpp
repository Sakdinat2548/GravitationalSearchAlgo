#include "../test_common.hpp"
#include "../test_framework.hpp"

#include <string>

using gsa_test::Config;
using gsa_test::Optimize;

namespace {
size_t g_call{};
size_t g_agents{};

double SeqFitness([[maybe_unused]] std::span<const double>) {
  return static_cast<double>(g_call++ % g_agents);
}
}  // namespace

TEST(median_correctness) {
  bool ok{true};
  const auto run = [&](bool minimize, size_t n_agents) {
    g_call = 0;
    g_agents = n_agents;
    const auto res =
        Optimize(1, -1.0, 1.0, &SeqFitness, Config(minimize, n_agents));
    const double reference{static_cast<double>(n_agents - 1) / 2.0};
    const double expect_hi{static_cast<double>(n_agents - 1)};
    const std::string label{std::string(minimize ? "min" : "max") + "/n=" +
                            std::to_string(n_agents)};

    for (const auto& it : res.history) {
      if (it.median_fitness != reference) {
        ok = false;
        gsa_test::Expect(false, "median mismatch " + label);
        break;
      }
      const bool pop_ok =
          (it.best_iter == 0.0 && it.worst_iter == expect_hi) ||
          (it.best_iter == expect_hi && it.worst_iter == 0.0);
      if (!pop_ok) {
        ok = false;
        gsa_test::Expect(false, "population not 0..n-1 " + label);
        break;
      }
    }
  };

  run(true, 4);
  run(false, 4);
  run(true, 5);
  run(false, 5);
  return ok;
}
