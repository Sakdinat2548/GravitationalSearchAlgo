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

  // small population exercises the k-best clamp.
  const auto small = Config(true, 3);
  const auto res_small = Optimize(8, -5.0, 5.0, Sphere, small);
  ok = CheckHistory(res_small, small, "small") && ok;

  // boundary configs for the arena layout: single agent, 1D, and both.
  const auto single = Config(true, 1);
  const auto res_single = Optimize(8, -5.0, 5.0, Sphere, single);
  ok = CheckHistory(res_single, single, "single") && ok;

  const auto dim1 = Config(true);
  const auto res_dim1 = Optimize(1, -5.0, 5.0, Sphere, dim1);
  ok = CheckHistory(res_dim1, dim1, "dim1") && ok;

  const auto single_dim1 = Config(true, 1);
  const auto res_single_dim1 = Optimize(1, -5.0, 5.0, Sphere, single_dim1);
  ok = CheckHistory(res_single_dim1, single_dim1, "single_dim1") && ok;

  return ok;
}
