# AGENTS.md

Guidance for AI agents working in this repository.

<!-- graft:start -->
## Graft — repo context graph

This repo is indexed in `graft/`: small linked markdown nodes that explain each
system and carry exact file:line spans, kept in sync with the code through git.

For ANY task here — understanding how something works, finding where code lives,
or scoping a change — get context from the graph before grepping or opening
source files. Re-ask freely (it's cheap) and reuse literal identifiers you
already have (symbol, error string, file name) as the query. New to this repo?
Run `graft map` first — a token-budgeted orientation (dir clusters, hubs,
hotspots), no LLM, no key.

- Run `graft ask "<your question>" --source` → ranked nodes with the relevant
  code spans inlined (each hit's ≤8-line crux by default; `--full` for whole
  definitions when the crux isn't enough). Match the tool to the task shape:
  for understanding or editing, the top node IS the answer — cite its
  `covers:` file:line spans and edit straight from `--source`. For
  exhaustive tasks ("every occurrence / every caller of this pattern"), ranked
  results are top-N, not complete — run `graft grep "<literal>"` instead
  (exhaustive over indexed files, grouped by enclosing symbol), falling back
  to raw `grep -rn` only for unindexed files.
- `graft skeleton <file>` → every definition's signature + span, ~10× cheaper
  than reading the file; use it to skim an API surface.
- `graft callers <symbol>` gives precomputed, exact edges — who calls this.
  Add `--direction out` for what it calls, or `--depth N` to walk
  transitively for the full blast radius. For structural questions, skip
  ranking and use this directly.
- Or browse: `graft/INDEX.md` lists every node; follow the links.
- Monorepos and folders of multiple repos rank fairly across sub-projects —
  hits carry `[scope/]` labels naming which one they're from. Narrow with
  `graft ask "<task>" --in <scope>/` once you know where you're working.

If a returned span is truncated ("+N more lines"), open the file at that exact
range before finalizing. Only open source files when a node genuinely lacks a
needed detail, and then at the exact file:line the node points to — never
re-read whole files.

After big code changes, refresh the graph with `graft build` (deterministic,
no API key, $0).
<!-- graft:end -->

## Project

C++20 implementation of the Gravitational Search Algorithm (GSA, Rashedi et al.).
Dependencies managed by Conan 2:
- `nlohmann_json/3.11.2` — JSON configuration support
- `xoshiro-cpp/1.1` — Xoshiro256++ PRNG (included via `<xoshiro-cpp/XoshiroCpp.hpp>`)

## Build / Verify (CMake + Conan 2, MSYS2 UCRT64 g++ on Windows)

```bash
# 1. Install Conan dependencies (pick profile from profiles/ per OS;
#    Windows shown; both deps are header-only so no binaries are built)
conan install . --build=missing -pr:h=profiles/windows-msys2-gcc -pr:b=profiles/windows-msys2-gcc

# 2. Configure with Conan toolchain
cmake --preset conan-release

# 3. Build
cmake --build --preset conan-release

# Fast syntax check; must be warning-free
cmake --build --preset conan-release --target check-gsa

# Run the test suite (GSA invariants + determinism)
ctest --preset conan-release

# Google-casing naming lint (see .clang-tidy); must be warning-free too
cmake --build --preset conan-release --target check-naming
```

Build outputs land in `build/Release/` (`build/Release/demo.exe`, `build/Release/main.exe`, `build/Release/tests/gsa_test.exe`).

Profiles live in `profiles/`: `windows-msys2-gcc`, `linux-gcc`, `macos-clang`. Do NOT use bare `conan profile detect` — it mis-detects MSVC on Windows machines with VS remnants and defaults to cppstd=14, which rejects xoshiro-cpp (requires ≥17). The `conan-release` preset itself is Conan-generated per machine (`CMakeUserPresets.json` includes it); its generator varies by OS but the name is stable.

Alternative: `default` preset (Ninja, no Conan) — requires system-installed `nlohmann_json` and `xoshiro-cpp`:
```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

The tracked tests are eleven CTest entries (`gsa_history`, `gsa_stats`, `gsa_determinism`,
`gsa_modes`, `gsa_thread_safety`, `gsa_median`, `gsa_convergence`, `gsa_validation`,
`gsa_json_io_config`, `gsa_json_io_bounds`, `gsa_snapshots`) built from `tests/tasks/*.cpp` (see Files). Each asserts one invariant group, plus same-seed determinism, both `minimize` modes, an odd agent count (median), and concurrent `Optimize()` on one instance. Recurring invariants:
- history size == `max_iter + 1`; `best_so_far` monotonic per mode.
- `mean_fitness` and `median_fitness` within `[best_iter, worst_iter]` (reversed for maximize).
- `stddev_fitness >= 0` and finite; same `seed` => identical results.
- Check both `minimize = true` and `false`, and an odd agent count (median).

## Files

- `src/gsa/gsa.hpp` — header-only algorithm library (primary work here). `GravitationalSearchAlgorithm` is a class template on the objective callable (any `std::invocable<std::span<const double>>` returning `double`, deduced via CTAD). Public header included as `#include "gsa/gsa.hpp"`. Xoshiro RNG resolved via Conan (`<xoshiro-cpp/XoshiroCpp.hpp>`).
- `src/gsa/stats.hpp` — `FitnessStats` + `ComputeFitnessStats` (order statistics: min/max, two-`nth_element` median, mean, stddev) and `BetterFit`; the `gsa_median` test calls it directly.
- `src/gsa/json_io.hpp` — JSON loading utilities (namespace `gsa`): `LoadConfigFromJson`/`LoadConfigFromString` for `GsaConfig`, plus `Bounds` + `LoadBoundsFromJson` for dimensions/bounds (scalar or per-dim arrays; bounds are required — no defaults).
- `src/gsa/csv_io.hpp` — `WriteHistoryCsv`/`WriteSnapshotsCsv`: Maxima-readable CSVs (no header rows; `.csv` extension implies comma separator in `read_matrix`).
- `examples/demo.cpp` — API demo: equal-bounds, per-dimension-bounds, rosenbrock (custom `g0`/`alpha`), and lambda-objective examples; prints `best_val` + position for each.
- `examples/main.cpp` — JSON config hot-reload demo: creates `config.json` only if missing, prints the raw file, runs Rosenbrock; pressing `r` reloads the file and reassigns a fresh instance. Every run exports to `exports/run_<yyyymmdd_hhmmss>/` (`history.csv` always, `snapshots.csv` when `snapshot_count > 0`, plus effective `config.json`).
- `examples/gsa_plot.mac` — Maxima loader: `gsa_read_history`/`gsa_plot_convergence`/`gsa_read_snapshots`/`gsa_plot_snapshot` over the exported CSVs.
- `tests/` — self-contained framework (`test_framework.hpp` registers named tests; `test_common.hpp` shares objectives/invariant helpers; `test_main.cpp` dispatches, passing a test name as argv[1] to run only that one) + `tests/tasks/*.cpp`, one file per test: `test_history.cpp`, `test_stats.cpp`, `test_determinism.cpp`, `test_modes.cpp`, `test_thread_safety.cpp`, `test_median.cpp`, `test_convergence.cpp`, `test_validation.cpp`, `test_json_io.cpp` (registers both JSON IO tests), `test_snapshots.cpp`. Its own `CMakeLists.txt` builds `gsa_test` and registers CTest; run a single test directly: `build/Release/tests/gsa_test.exe <name>` (name = argv[1], e.g. `determinism`).
- `conanfile.py` — Conan 2 recipe declaring `nlohmann_json/3.11.2` and `xoshiro-cpp/1.1` requirements.
- `profiles/` — Conan profiles: `windows-msys2-gcc` (MSYS2 UCRT64 g++), `linux-gcc`, `macos-clang`.
- `CMakeLists.txt` / `CMakePresets.json` — Ninja + MinGW Makefiles Release build; options `GSA_BUILD_DEMO`, `GSA_BUILD_MAIN`, `GSA_BUILD_TESTS`; outputs in `build/` or `build/Release/`.
- `.clang-format` / `.clang-tidy` — Google formatting and Google-casing naming conventions.
- `.vscode/` — VS Code C++20 IntelliSense config (includes conan paths via env vars).
- `.venv/` + `pyrightconfig.json` — Python venv with `conan` + `pyright` for `conanfile.py` type checking.

## API

- `GsaConfig { n_agents, max_iter, g0, alpha, minimize, seed, snapshot_count }` — seed `0` means `std::random_device`; `snapshot_count` (`0` = off, `1` = final only, max `max_iter + 1`) captures all agent positions at evenly spaced iterations incl. iter 0 and `max_iter`.
- **Changing config between runs**: the instance is immutable — assign a fresh `GravitationalSearchAlgorithm` with a new `GsaConfig` (all members are value types; reassignment is cheap and memory-safe). Each `Optimize()` is an independent run.
**JSON config**: `gsa::LoadConfigFromString(str)`, `gsa::LoadConfigFromJson(json)` return `GsaConfig`.
- `GravitationalSearchAlgorithm` ctors: `(lower, upper, func, cfg)`,
  `(dims, lo, hi, func, cfg)` (scalar bounds), or
  `(dims, lower_vec, upper_vec, func, cfg)`. All three delegate to a private
  `ValidatedTag` canonical ctor that runs the single validator
  `ValidateConfigs(lower, upper, cfg, expected_dims)` (`expected_dims == 0`
  derives from the vectors).
  Public ctors all take by-value sources and `std::move` into the canonical
  ctor — safe because no argument reads another argument's moved-from state
  (mem-initializer argument evaluation order is unspecified; never inspect one
  moved-from parameter from a sibling argument expression).
- `GsaResult Optimize() const` returns `{ best_val, best_pos, history, snapshot_iters, snapshot_positions, snapshot_dims }` (positions flat: snap → agent → dim).
- `GsaIterationInfo` (per iteration): `best_so_far, best_iter, worst_iter, mean_fitness, median_fitness, stddev_fitness`.
- Calling `Optimize()` again on the same instance is an independent run: positions are re-randomized and `V`/`A` are reset to zero at the start. Within a single run the paper-faithful recurrence `V(t+1) = rand * V(t) + A(t)` holds ("old V feeds new V").

## Conventions

- C++20 (`-std=c++20`). `std::ranges` preferred where C++20 supports it (not `std::ranges::iota` — that is C++23; `std::iota` stays). Integer loops use the Python-style `Range(n)` / `Range(start, stop)` helper (wraps `std::views::iota`; `Range(n)` starts at 0).
- Particle arrays are flat `std::vector<double>`; use `AgentOffset(agent)` to get the base offset of an agent's row (`agent * dimensions_`). `dimensions_` and per-dimension loop counters are `size_t` (no sign-conversion casts).
- All working buffers live in a private per-call `IterationState` (created per `Optimize()`). The instance stores only immutable config, so `Optimize()` is `const` and concurrent calls on one instance are safe (the objective function itself must be thread-safe). Buffers are resized once per call — no per-iteration allocation.
- `IterationState` allocates exactly two buffers per call: `arena_d` (3*N*D + 2*N + D doubles, uninitialized `make_unique_for_overwrite` arrays) and `arena_i` (N indices); only `velocity` + `acceleration` are zeroed (the sole required initial state); six `std::span<double>` + one `std::span<size_t>` slice the arenas (position, velocity, acceleration, fitness, mass, total_force, sorted_indices). Copy/move are deleted — spans must never dangle.
- `ComputeAccelerations` is the hot spot: O(N^2 * kbest * dims). It uses k-best selection via `nth_element`, flat contiguous arrays, and hoisted row spans (`std::span<const double> x_i = positions.subspan(AgentOffset(i), dimensions_)`) inside the pair loops. Keep that pattern; do not re-index with `s.position[offset + d]` in the inner loops. `total_force` is a per-dimension scratch, refilled once per agent.
- **Determinism contract**: same seed + same build => identical results (enforced by `gsa_determinism`). Cross-version ULP drift is acceptable — hot-loop refactors do NOT need bit-identical-to-baseline verification. When changing hot loops, time a quick benchmark (e.g. `n_agents=200, max_iter=500`) so perf does not silently regress; results may differ by ULPs from previous builds.
- `RecordIteration` (private member) calls `ComputeFitnessStats` (in `stats.hpp`; takes `std::span<size_t>` for `sorted_indices`) for history stats via order statistics, no per-iteration sort: min/max from `minmax_element`, median via two `nth_element` calls on `sorted_indices` — one at `n / 2` (upper-middle), and for even `n` one at `n / 2 - 1` (lower-middle; a single `nth_element` cannot pin the `(n-1)/2` slot, which sits in an unordered half). `sorted_indices` is reused, so its partition is re-done in `ComputeAccelerations` right after). Mean/stddev sum in physical array order; all stats are deterministic for a given seed + build (see Determinism contract above).
- The user prefers terse code; add no comments unless asked. Verify `best_iter`/`worst_iter` semantics flip with `config_.minimize` (they did before).

## Current State

Merged features (through current branch):
- Header-only GSA library with `std::span` API
- Conan 2 integration: `nlohmann_json/3.11.2`, `xoshiro-cpp/1.1`
- JSON config: `LoadConfigFromString`, `LoadConfigFromJson` (in `src/gsa/json_io.hpp`)
- All public API in `gsa::` namespace; config changes via instance reassignment (no mutators)
- `conanfile.py` + `profiles/` (windows-msys2-gcc, linux-gcc, macos-clang) for portable builds
- `examples/main.cpp` demonstrating JSON config hot-reload (`r` key)
- `examples/demo.cpp` with 4 original API examples
- 11/11 CTest tests passing, `check-gsa` + `check-naming` clean
- Same-seed determinism verified across all configurations
