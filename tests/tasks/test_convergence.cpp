#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::Sphere;

TEST(Convergence, SphereBelowThreshold) {
  gsa::GravitationalSearchAlgorithm gsa(
      3, -5.0, 5.0, Sphere,
      {.n_agents = 50, .max_iter = 500, .g0 = 10.0, .alpha = 20.0,
       .minimize = true, .seed = 12345});
  const gsa::GsaResult res{gsa.Optimize()};
  EXPECT_LT(res.best_val, 0.01) << "sphere did not converge below 0.01";
}
