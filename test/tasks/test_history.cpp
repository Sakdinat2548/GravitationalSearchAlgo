#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

TEST(history_size) {
  bool ok = true;

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
