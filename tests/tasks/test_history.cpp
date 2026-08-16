#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::CheckHistory;
using gsa_test::Config;
using gsa_test::Optimize;
using gsa_test::Sphere;

TEST(history_size) {
  bool ok{true};

  for (bool minimize : {true, false}) {
    const auto cfg = Config(minimize);
    const auto res = Optimize(8, -5.0, 5.0, Sphere, cfg);
    ok = CheckHistory(res, cfg, minimize ? "min" : "max") && ok;
  }

  const auto odd = Config(true, 51);
  const auto res = Optimize(8, -5.0, 5.0, Sphere, odd);
  ok = CheckHistory(res, odd, "odd") && ok;

  return ok;
}
