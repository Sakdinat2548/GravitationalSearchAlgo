#include <gtest/gtest.h>

#include "../test_common.hpp"

using gsa_test::Config;
using gsa_test::Rosenbrock;
using gsa_test::Shekel;
using gsa_test::Sphere;

namespace {

void ExpectDeterministic(int dims, double lo, double hi,
                         double (*fn)(std::span<const double>),
                         const gsa::GsaConfig& cfg) {
  gsa::GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
  const auto a = gsa.Optimize();
  const auto b = gsa.Optimize();
  EXPECT_EQ(a.best_val, b.best_val) << "same seed must give identical result";
  EXPECT_EQ(a.history.size(), b.history.size());
}

void ExpectChurnDeterministic(int dims, double lo, double hi,
                              double (*fn)(std::span<const double>),
                              const gsa::GsaConfig& cfg, int runs = 16) {
  double first{};
  for (int r = 0; r < runs; ++r) {
    gsa::GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
    const auto res = gsa.Optimize();
    if (r == 0) {
      first = res.best_val;
    } else {
      EXPECT_EQ(res.best_val, first)
          << "fresh-instance run diverged (uninitialized read?)";
    }
    std::vector<double> noise(cfg.n_agents * dims, static_cast<double>(r));
  }
}

}  // namespace

TEST(Determinism, SameSeedIdentical) {
  const auto cfg = Config();
  ExpectDeterministic(8, -5.0, 5.0, Sphere, cfg);
  ExpectDeterministic(8, -2.048, 2.048, Rosenbrock, cfg);
  ExpectDeterministic(2, -65.53, 65.53, Shekel, cfg);

  const auto mx = Config(false);
  ExpectDeterministic(8, -5.0, 5.0, Sphere, mx);

  ExpectChurnDeterministic(8, -5.0, 5.0, Sphere, cfg);
  ExpectChurnDeterministic(8, -2.048, 2.048, Rosenbrock, mx);
}
