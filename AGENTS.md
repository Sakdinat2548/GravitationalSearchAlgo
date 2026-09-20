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

C++20 header-only Gravitational Search Algorithm (`src/gsa/gsa.hpp`, namespace `gsa`).
Conan 2 deps: `nlohmann_json/3.11.2`, `xoshiro-cpp/1.1` (both header-only;
`<xoshiro-cpp/XoshiroCpp.hpp>`), `gtest/1.14.0` (compiled once for tests). `GravitationalSearchAlgorithm` is templated on
the objective (any `std::invocable<std::span<const double>>` returning `double`, CTAD-deduced).

## Build / Verify (MSYS2 UCRT64 g++ on Windows)

```bash
conan install . --build=missing -pr:h=profiles/windows-msys2-gcc -pr:b=profiles/windows-msys2-gcc
cmake --preset conan-release
cmake --build --preset conan-release
cmake --build --preset conan-release --target check-gsa      # syntax, must be warning-free
cmake --build --preset conan-release --target check-naming   # clang-tidy Google-casing, must be warning-free
ctest --preset conan-release
```

- Profiles in `profiles/` (`windows-msys2-gcc`, `linux-gcc`, `macos-clang`). Never use bare
  `conan profile detect` — it mis-detects MSVC and defaults to cppstd=14, which rejects
  xoshiro-cpp (needs ≥17). `conan-release` is Conan-generated per machine
  (`CMakeUserPresets.json` include); the name is stable, the generator is not.
- Fallback without Conan (`default` preset, Ninja) needs system-installed deps
  (`nlohmann_json`, `xoshiro-cpp`, plus `gtest` with CMake config files):
  `cmake --preset default && cmake --build --preset default && ctest --preset default`.
- Outputs: `build/Release/{demo,main,bench}.exe`, `build/Release/tests/gsa_test.exe`.

## Tests

11 CTest entries from `tests/tasks/*.cpp` via GoogleTest (`tests/TEST.md` maps
suites to entries): `gsa_history`, `gsa_stats`, `gsa_determinism`, `gsa_modes`,
`gsa_thread_safety`, `gsa_median`, `gsa_convergence`, `gsa_validation`,
`gsa_json_io_config`, `gsa_json_io_bounds`, `gsa_snapshots`. Single suite:
`build/Release/tests/gsa_test.exe --gtest_filter=Determinism.*`. New tests need a
`--gtest_filter` CTest entry; `EXPECT_THROW` needs a named lambda for multi-arg
statements. Recurring invariants:
history size == `max_iter + 1`, monotonic `best_so_far` per mode, mean/median within
`[best_iter, worst_iter]` (reversed for maximize), `stddev >= 0` finite, same seed ⇒
identical, both `minimize` modes + odd agent count.

## API essentials

- `GsaConfig { n_agents, max_iter, g0, alpha, minimize, seed, snapshot_count }`.
  `seed 0` = `std::random_device`. `snapshot_count`: `0` = off, `1` = final only,
  max `max_iter + 1`; always includes iter 0 and `max_iter`.
- Instance is immutable — change config by assigning a fresh instance. Every `Optimize()`
  is an independent run (positions re-randomized, V/A zeroed); within a run
  `V(t+1) = rand·V(t) + A(t)`. `Optimize()` is `const` and thread-safe if the objective is.
- Ctors: `(lower, upper, func, cfg)`, `(dims, lo, hi, func, cfg)`,
  `(dims, lower_vec, upper_vec, func, cfg)`. Bounds are required, no defaults.
  JSON loader (`json_io.hpp`: `LoadConfigFromJson`/`LoadConfigFromString`, `Bounds` +
  `LoadBoundsFromJson`) accepts mixed scalar/array bounds when `dimensions` is given;
  C++ ctors do not mix — both scalar or both vectors.
- `GsaResult`: `best_val`, `best_pos`, `history` (`GsaIterationInfo`: `best_so_far`,
  `best_iter`, `worst_iter`, `mean_fitness`, `median_fitness`, `stddev_fitness`),
  flat snapshots (`snapshot_iters`, `snapshot_positions` snap→agent→dim, `snapshot_masses` /
  `snapshot_fitnesses` snap→agent).

## Conventions / gotchas

- C++20. Integer loops use `impl::Range(n)` / `impl::Range(a, b)` (`views::iota` wrapper,
  unsigned-only by constraint); keep `std::iota` (`ranges::iota` is C++23).
  `dimensions_` and dim counters are `size_t`; index agent rows via
  `AgentOffset(agent)` (`agent * dimensions_`). Internal helpers (`Range`,
  `RandUni`, `BetterFit`, json_io guards) live in `gsa::impl` — keep them there.
- Objective must return finite values — non-finite fitness throws
  `std::invalid_argument` naming the agent. CSV writers throw `std::runtime_error`
  on I/O failure and validate snapshot vector sizes.
- Per-`Optimize()` buffers only — never add instance state. Hot loop
  `ComputeAccelerations` (O(N²·kbest·dims)): keep hoisted row spans
  (`positions.subspan(AgentOffset(i), dimensions_)`), `nth_element` k-best, no per-iter alloc.
- Do not remove the `kMassFloor` (1e-6) in `ComputeMasses` — prevents frozen zero-mass
  agents / outlier-flattened swarm (verified vs Rashedi Table 4).
- Determinism: same seed + same build ⇒ identical (`gsa_determinism`). Cross-build ULP
  drift is fine; time a quick bench (e.g. 200 agents × 500 iters) after hot-loop edits.
- `best_iter`/`worst_iter` semantics flip with `minimize` — re-verify after stat changes.
- Terse code, no comments unless asked. clang-tidy enforces Google-casing with `lower_case`
  variables/members (`_` suffix); `check-naming` covers `src/gsa/*.hpp`.

## Viz / benchmarks (for runs, not library edits)

- `main.exe` exports `exports/run_<ts>/` (`history.csv` always, `snapshots.csv` if
  `snapshot_count > 0`, plus effective `config.json`). `python scripts/plot_gsa.py <run>/`
  (`.venv` Python) writes convergence/contour/anim GIFs; 2D contour background is
  Rosenbrock-hardcoded (`objective_2d` + `LEVELS` EDIT block) — update it for other objectives.
- Fairness contract for comparisons: same dims/bounds/objective/pop/iters, distinct
  deterministic seeds, minimize everywhere. `benchmark.py --run-bench` drives `bench.exe`
  + Python algos (N=30) into `exports/bench_<ts>/{gsa,meta,ga}_avg.csv`
  (`iter,avg_best,median_best,std_best,avg_mean`); `plot_compare.py` → `compare.png`.
  Objectives live in `scripts/objective.py` (mirror of C++ side).
