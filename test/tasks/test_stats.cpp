#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Shekel;
using gsa_test::Sphere;

static bool AllStatsFinite(const GsaResult& res) {
  bool ok = true;
  for (const auto& it : res.history) {
    ok = ok && it.stddev_fitness >= 0.0 && std::isfinite(it.stddev_fitness);
  }
  return ok;
}

TEST(history_stats) {
  bool ok = true;

  for (bool minimize : {true, false}) {
    const auto cfg = Config(minimize);
    const auto res = Optimize(8, -5.0, 5.0, Sphere, cfg);
    ok = AllStatsFinite(res) && ok;
  }

  const auto shekel_res = Optimize(2, -65.53, 65.53, Shekel, Config());
  ok = AllStatsFinite(shekel_res) && ok;

  return ok;
}
