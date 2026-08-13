#include "../test_common.hpp"
#include "../test_framework.hpp"

using namespace gsa_test;

TEST(modes) {
    bool ok = true;

    const auto mn = optimize(8, -5.0, 5.0, sphere, config(true));
    const auto mx = optimize(8, -5.0, 5.0, sphere, config(false));

    ok = mn.history.back().best_iter <= mn.history.back().worst_iter && ok;
    ok = mx.history.back().best_iter >= mx.history.back().worst_iter && ok;

    // odd agent count (median) must not crash and must satisfy invariants.
    const auto odd = config(true, 51);
    const auto res = optimize(8, -5.0, 5.0, sphere, odd);
    ok = check_history(res, odd, "odd") && ok;

    return ok;
}
