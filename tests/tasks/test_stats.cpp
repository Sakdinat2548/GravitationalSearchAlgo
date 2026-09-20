#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Shekel;
using gsa_test::Sphere;

TEST(History, StatsFinite) {
  for (bool minimize : {true, false}) {
    SCOPED_TRACE(minimize ? "min" : "max");
    const auto cfg = Config(minimize);
    const auto res = Optimize(8, -5.0, 5.0, Sphere, cfg);
    for (const auto& it : res.history) {
      EXPECT_GE(it.stddev_fitness, 0.0);
      EXPECT_TRUE(std::isfinite(it.stddev_fitness));
    }
  }

  SCOPED_TRACE("shekel");
  const auto shekel_res = Optimize(2, -65.53, 65.53, Shekel, Config());
  for (const auto& it : shekel_res.history) {
    EXPECT_GE(it.stddev_fitness, 0.0);
    EXPECT_TRUE(std::isfinite(it.stddev_fitness));
  }
}
