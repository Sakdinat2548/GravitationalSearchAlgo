#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::CheckHistory;
using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Sphere;

TEST(modes) {
  bool ok{true};

  const auto mn = Optimize(8, -5.0, 5.0, Sphere, Config(true));
  const auto mx = Optimize(8, -5.0, 5.0, Sphere, Config(false));

  ok = mn.history.back().best_iter <= mn.history.back().worst_iter && ok;
  ok = mx.history.back().best_iter >= mx.history.back().worst_iter && ok;

  // odd agent count (median) must not crash and must satisfy invariants.
  const auto odd = Config(true, 51);
  const auto res = Optimize(8, -5.0, 5.0, Sphere, odd);
  ok = CheckHistory(res, odd, "odd") && ok;

  // small population (n_agents <= 10) takes the is_small_problem path.
  const auto small = Config(true, 3);
  const auto res_small = Optimize(8, -5.0, 5.0, Sphere, small);
  ok = CheckHistory(res_small, small, "small") && ok;

  return ok;
}
