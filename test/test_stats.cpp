#include "test_common.hpp"
#include "test_framework.hpp"

using namespace gsa_test;

static bool all_stats_finite(const GsaResult& res) {
    bool ok = true;
    for (const auto& it : res.history) {
        ok = ok && it.stddev_fitness >= 0.0 && std::isfinite(it.stddev_fitness);
    }
    return ok;
}

TEST(history_stats) {
    bool ok = true;

    for (bool minimize : {true, false}) {
        const auto cfg = config(minimize);
        const auto res = optimize(8, -5.0, 5.0, sphere, cfg);
        ok = all_stats_finite(res) && ok;
    }

    const auto shekel_res = optimize(2, -65.53, 65.53, shekel, config());
    ok = all_stats_finite(shekel_res) && ok;

    return ok;
}
