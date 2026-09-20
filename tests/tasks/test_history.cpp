#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::CheckHistory;
using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Sphere;

TEST(History, SizeAndInvariants) {
  for (bool minimize : {true, false}) {
    SCOPED_TRACE(minimize ? "min" : "max");
    const auto cfg = Config(minimize);
    CheckHistory(Optimize(8, -5.0, 5.0, Sphere, cfg), cfg,
                 minimize ? "min" : "max");
  }

  SCOPED_TRACE("odd");
  const auto odd = Config(true, 51);
  CheckHistory(Optimize(8, -5.0, 5.0, Sphere, odd), odd, "odd");
}
