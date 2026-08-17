#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::Sphere;

TEST(convergence) {
  GravitationalSearchAlgorithm gsa(
      3, -5.0, 5.0, Sphere,
      {.n_agents = 50, .max_iter = 500, .g0 = 10.0, .alpha = 20.0,
       .minimize = true, .seed = 12345});
  const GsaResult res{gsa.Optimize()};
  if (res.best_val >= 0.01) {
    gsa_test::Expect(false, "sphere did not converge below 0.01");
    return false;
  }
  return true;
}