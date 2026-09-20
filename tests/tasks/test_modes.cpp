#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::CheckHistory;
using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Sphere;

TEST(Modes, MinimizeMaximizeAndEdgeSizes) {
  const auto mn = Optimize(8, -5.0, 5.0, Sphere, Config(true));
  const auto mx = Optimize(8, -5.0, 5.0, Sphere, Config(false));

  EXPECT_LE(mn.history.back().best_iter, mn.history.back().worst_iter);
  EXPECT_GE(mx.history.back().best_iter, mx.history.back().worst_iter);

  // odd agent count (median) must not crash and must satisfy invariants.
  const auto odd = Config(true, 51);
  CheckHistory(Optimize(8, -5.0, 5.0, Sphere, odd), odd, "odd");

  // small population exercises the k-best clamp.
  const auto small = Config(true, 3);
  CheckHistory(Optimize(8, -5.0, 5.0, Sphere, small), small, "small");

  // boundary configs for the arena layout: single agent, 1D, and both.
  const auto single = Config(true, 1);
  CheckHistory(Optimize(8, -5.0, 5.0, Sphere, single), single, "single");

  const auto dim1 = Config(true);
  CheckHistory(Optimize(1, -5.0, 5.0, Sphere, dim1), dim1, "dim1");

  const auto single_dim1 = Config(true, 1);
  CheckHistory(Optimize(1, -5.0, 5.0, Sphere, single_dim1), single_dim1,
               "single_dim1");
}
