#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

TEST(modes) {
  bool ok = true;

  const auto mn = Optimize(8, -5.0, 5.0, Sphere, Config(true));
  const auto mx = Optimize(8, -5.0, 5.0, Sphere, Config(false));

  ok = mn.history.back().best_iter <= mn.history.back().worst_iter && ok;
  ok = mx.history.back().best_iter >= mx.history.back().worst_iter && ok;

  // odd agent count (median) must not crash and must satisfy invariants.
  const auto odd = Config(true, 51);
  const auto res = Optimize(8, -5.0, 5.0, Sphere, odd);
  ok = CheckHistory(res, odd, "odd") && ok;

  return ok;
}
