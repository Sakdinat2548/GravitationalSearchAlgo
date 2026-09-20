# Tests

GoogleTest-based suite (`gtest/1.14.0` via Conan). One `TEST(Suite, Case)` per
file in `tests/tasks/`; shared objectives (`Sphere`/`Rosenbrock`/`Shekel`),
`Config`/`Optimize` helpers, and the `CheckHistory` invariant checker live in
`tests/test_common.hpp`. `main()` comes from `GTest::gtest_main` — there is no
test `main()` in this repo.

## Files

| File | Role |
|------|------|
| `test_common.hpp` | Shared objectives, config/result helpers, `CheckHistory` (all `EXPECT_*`, non-fatal). |
| `tasks/` | One `test_*.cpp` per suite, written as `TEST(Suite, Case) { ... }`. |

## CTest mapping

`tests/CMakeLists.txt` builds one `gsa_test` binary and registers each suite as
its own CTest entry via `--gtest_filter`, keeping the historical names:

| CTest | Filter |
|---|---|
| `gsa_history` | `History.SizeAndInvariants` |
| `gsa_stats` | `History.StatsFinite` |
| `gsa_determinism` | `Determinism.SameSeedIdentical` |
| `gsa_modes` | `Modes.MinimizeMaximizeAndEdgeSizes` |
| `gsa_thread_safety` | `ThreadSafety.ConcurrentOptimizeIdentical` |
| `gsa_median` | `FitnessStats.MedianCorrectness` |
| `gsa_convergence` | `Convergence.SphereBelowThreshold` |
| `gsa_validation` | `Validation.RejectsBadConfig` |
| `gsa_json_io_config` | `JsonIo.ConfigDefaultsAndErrors` |
| `gsa_json_io_bounds` | `JsonIo.BoundsParsingAndErrors` |
| `gsa_snapshots` | `Snapshots.CaptureAndCsvRoundTrip` |

Run them with `ctest --preset conan-release`, or one suite directly:

```bash
./build/Release/tests/gsa_test.exe --gtest_filter=Determinism.*
```

## Adding a test

Add a `TEST(NewSuite, NewCase)` block to a new (or existing) file under
`tasks/`, list the file in `tests/CMakeLists.txt`, and register a CTest entry
with the matching `--gtest_filter`. Prefer `EXPECT_*` (collects all failures)
over `ASSERT_*`; use `EXPECT_THROW(..., std::invalid_argument)` for validation
checks. Note: `EXPECT_THROW` splits macro args on top-level commas, so wrap
multi-arg throwing statements in a named lambda first.
