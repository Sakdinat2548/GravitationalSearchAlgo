#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

static bool deterministic(int dims, double lo, double hi,
                          double (*fn)(std::span<const double>),
                          const GsaConfig& cfg) {
    GravitationalSearchAlgorithm gsa(dims, lo, hi, fn, cfg);
    const auto a = gsa.optimize();
    const auto b = gsa.optimize();
    const bool ok = a.best_val == b.best_val && a.history.size() == b.history.size();
    if (!ok) expect(false, "same seed must give identical result");
    return ok;
}

TEST(determinism) {
    bool ok = true;

    const auto cfg = config();
    ok = deterministic(8, -5.0, 5.0, sphere, cfg) && ok;
    ok = deterministic(8, -2.048, 2.048, rosenbrock, cfg) && ok;
    ok = deterministic(2, -65.53, 65.53, shekel, cfg) && ok;

    const auto mx = config(false);
    ok = deterministic(8, -5.0, 5.0, sphere, mx) && ok;

    return ok;
}
