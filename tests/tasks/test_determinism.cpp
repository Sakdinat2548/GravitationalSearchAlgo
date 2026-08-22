#include "../test_common.hpp"
#include "../test_framework.hpp"

using gsa_test::Config;
using gsa_test::Expect;
using gsa_test::Rosenbrock;
using gsa_test::Shekel;
using gsa_test::Sphere;

static bool Deterministic(int dims, double lo, double hi,
                          double (*fn)(std::span<const double>),
                          const gsa::GsaConfig& cfg) {
  gsa::GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
  const auto a = gsa.Optimize();
  const auto b = gsa.Optimize();
  const bool ok{
      a.best_val == b.best_val && a.history.size() == b.history.size()};
  if (!ok) Expect(false, "same seed must give identical result");
  return ok;
}

static bool ChurnDeterministic(int dims, double lo, double hi,
                               double (*fn)(std::span<const double>),
                               const gsa::GsaConfig& cfg, int runs = 16) {
  bool ok{true};
  double first{};
  for (int r = 0; r < runs; ++r) {
    gsa::GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
    const auto res = gsa.Optimize();
    if (r == 0) {
      first = res.best_val;
    } else if (res.best_val != first) {
      ok = false;
      Expect(false, "fresh-instance run diverged (uninitialized read?)");
    }
    std::vector<double> noise(cfg.n_agents * dims, static_cast<double>(r));
  }
  return ok;
}

TEST(determinism) {
  bool ok{true};

  const auto cfg = Config();
  ok = Deterministic(8, -5.0, 5.0, Sphere, cfg) && ok;
  ok = Deterministic(8, -2.048, 2.048, Rosenbrock, cfg) && ok;
  ok = Deterministic(2, -65.53, 65.53, Shekel, cfg) && ok;

  const auto mx = Config(false);
  ok = Deterministic(8, -5.0, 5.0, Sphere, mx) && ok;

  ok = ChurnDeterministic(8, -5.0, 5.0, Sphere, cfg) && ok;
  ok = ChurnDeterministic(8, -2.048, 2.048, Rosenbrock, mx) && ok;

  return ok;
}
