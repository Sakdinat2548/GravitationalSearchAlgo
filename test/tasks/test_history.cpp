#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

TEST(history_size) {
  bool ok = true;

  for (bool minimize : {true, false}) {
    const auto cfg = config(minimize);
    const auto res = optimize(8, -5.0, 5.0, sphere, cfg);
    ok = check_history(res, cfg, minimize ? "min" : "max") && ok;
  }

  const auto odd = config(true, 51);
  const auto res = optimize(8, -5.0, 5.0, sphere, odd);
  ok = check_history(res, odd, "odd") && ok;

  return ok;
}
