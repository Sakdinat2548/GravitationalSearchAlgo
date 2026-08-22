# Gravitational Search Algorithm

C++20 implementation of the Gravitational Search Algorithm (GSA, Rashedi et al.).

## Build

Requires [CMake](https://cmake.org) (3.20+), [Ninja](https://ninja-build.org), and [Conan 2](https://conan.io) (for dependencies). The `conan-release` preset configures a MinGW Makefiles, Release build in `build/Release` with `-O3 -DNDEBUG` (MSYS2 UCRT64 g++).

### Dependencies

Conan 2 manages all external dependencies:
- `nlohmann_json/3.11.2` — JSON configuration support
- `xoshiro-cpp/1.1` — Xoshiro256++ PRNG

No vendored dependencies; everything comes from Conan Center.

### Configure (one time, or after editing `CMakeLists.txt`)

```bash
# 1. Install Conan dependencies (uses msys2-gcc profile from repo)
conan install . --build=missing -pr:h=msys2-gcc -pr:b=msys2-gcc

# 2. Configure with Conan toolchain
cmake --preset conan-release
```

This generates the build files under `build/Release` with the Conan toolchain (provides `nlohmann_json` and `xoshiro-cpp` targets).

### Build

```bash
cmake --build --preset conan-release
```

Produces:
- `build/Release/demo.exe` — 4-example API demo (prints `best_val` + position)
- `build/Release/main.exe` — JSON config + runtime mutation examples
- `build/Release/tests/gsa_test.exe` — GSA invariants + determinism test harness

### Run

```bash
./build/Release/demo.exe          # 4-example API demo
./build/Release/main.exe          # JSON config + runtime mutation demo
./build/Release/tests/gsa_test.exe  # Full test suite (8 tests)
```

### Quick Syntax Check (warning-free sources)

```bash
cmake --build --preset conan-release --target check-gsa
```

### Naming Lint (Google-casing, see `.clang-tidy`)

```bash
cmake --build --preset conan-release --target check-naming
```

### Test (GSA Invariants + Determinism)

```bash
ctest --preset conan-release
```

Runs 8 CTest tests: `gsa_history`, `gsa_stats`, `gsa_determinism`, `gsa_modes`, `gsa_thread_safety`, `gsa_median`, `gsa_convergence`, `gsa_validation`. See `tests/` for sources. Run the binary directly to see per-test output, or pass a single test name to run only that one:

```bash
./build/Release/tests/gsa_test.exe
./build/Release/tests/gsa_test.exe determinism
```

---

### Alternative: `default` Preset (Ninja, No Conan)

If you already have `nlohmann_json` and `xoshiro-cpp` installed system-wide:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

This uses a Ninja Release build in `build/` without the Conan toolchain. You must ensure both dependencies are findable via `find_package`.

### Manual Build (No CMake)

The library is header-only. Compile directly with g++ (O3, C++20):

```bash
# Using conan-installed dependencies (get include paths from conan output or compile_commands.json)
g++.exe -O3 -std=c++20 \
    -Iinclude \
    -I<path_to_nlohmann_json_include> \
    -I<path_to_xoshiro_cpp_include> \
    src/demo.cpp -o demo.exe
```

After running `conan install`, the include paths are available in `build/Release/generators/conan_toolchain.cmake` or `build/Release/compile_commands.json`.

## Usage

Header-only library in `include/gsa/gsa.hpp` (`include/gsa/stats.hpp` for fitness statistics). Primary entry point: `GravitationalSearchAlgorithm`, templated on the objective callable (any invocable of `std::span<const double>` returning `double`; deduced via CTAD).

### Basic Example: Scalar Bounds

```cpp
#include "gsa/gsa.hpp"

double sphere(std::span<const double> x) {
    double s = 0.0; for (double v : x) s += v*v; return s;
}

GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere);
auto best = gsa.Optimize();
std::cout << "best value: " << best.best_val << "\n";
```

### Per-Dimension Bounds and Custom Config

```cpp
GravitationalSearchAlgorithm gsa2(
    {-10.0, 0.0, -1.0},
    {10.0, 50.0, 1.0},
    sphere,
    {.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0, .minimize = true}
);
auto best2 = gsa2.Optimize();
```

### Lambda Objective

```cpp
GravitationalSearchAlgorithm gsa3(2, -65.53, 65.53,
    [](std::span<const double> x){ return std::sin(x[0]) + std::cos(x[1]); },
    {.n_agents = 40, .max_iter = 500}
);
auto res3 = gsa3.Optimize();
```

### Accessing Results

```cpp
GsaResult res = gsa.Optimize();
std::vector<double> position = res.best_pos;           // best point found
std::vector<GsaIterationInfo> history = res.history;  // per-iteration stats
double last_mean = res.history.back().mean_fitness;   // mean fitness, last iter
```

Each `GsaIterationInfo` records: `best_so_far`, `best_iter`, `worst_iter`, `mean_fitness`, `median_fitness`, `stddev_fitness`.

### JSON Configuration

Load configuration from a JSON file:

```cpp
#include "gsa/gsa.hpp"
#include "gsa/config.hpp"

GsaConfig cfg = gsa::LoadConfigFromFile("config.json");
GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere, cfg);
auto best = gsa.Optimize();
```

Example `config.json`:
```json
{
  "n_agents": 40,
  "max_iter": 500,
  "g0": 10.0,
  "alpha": 20.0,
  "minimize": true,
  "seed": 12345
}
```

Or from a JSON string:

```cpp
#include "gsa/config.hpp"

const char* json = R"({
    "n_agents": 40,
    "max_iter": 500,
    "g0": 10.0,
    "alpha": 20.0
})";

GsaConfig cfg = gsa::LoadConfigFromString(json);
GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere, cfg);
```

### Runtime Configuration Mutation

Modify config on the fly between runs:

```cpp
GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere);

// First run with defaults
auto res1 = gsa.Optimize();

// Mutate config for second run
gsa.Config().SetG0(5.0).SetAlpha(5.0).SetNAgents(20);
auto res2 = gsa.Optimize();  // uses modified config
```

Available setters (chainable): `SetNAgents()`, `SetMaxIter()`, `SetG0()`, `SetAlpha()`, `SetMinimize()`, `SetSeed()`.

### Configuration Tuning

- `n_agents` (size_t): number of agents/particles. Default: `40`.
- `max_iter` (size_t): number of iterations. Default: `500`.
- `g0` (double): initial gravitational constant. Default: `100.0`.
- `alpha` (double): decay rate for `G(t) = g0 * exp(-alpha * t)`. Default: `20`.
- `minimize` (bool): `true` to minimize, `false` to maximize. Default: `true`.
- `seed` (uint64_t): RNG seed for reproducible runs; `0` uses `std::random_device`. Default: `0`.

**Tip:** `g0` is scale-dependent — tune it to your bounds. If the solver stalls early (all agents collapse onto the initial best), forces are too strong: try *decreasing* `g0` or *increasing* `alpha`. If it barely moves, forces are too weak: *increase* `g0` or *decrease* `alpha`. For a `[-5, 5]` domain `g0 = 10.0` converges cleanly, while the default `g0 = 100.0` overshoots and stalls.

## Tests

`tests/` holds a self-contained framework (`test_framework.hpp` registers named tests; `test_common.hpp` shares objectives/invariant helpers). Individual tests in `tests/tasks/`, one `*.cpp` per test, registered as separate CTest entries:

- `test_history.cpp` — history size `== max_iter + 1`, monotonic `best_so_far`, mean/median within `[best_iter, worst_iter]`, finite non-negative `stddev` (both modes).
- `test_stats.cpp` — history stats finite and in-range across objectives.
- `test_determinism.cpp` — same-seed bit-identical results on sphere/rosenbrock/Shekel.
- `test_modes.cpp` — `minimize` true/false and odd agent count (median).
- `test_thread_safety.cpp` — 8 concurrent `Optimize()` calls on one instance are bit-identical.
- `test_median.cpp` — direct unit test of `ComputeFitnessStats`: exact median `(n-1)/2` for even/odd populations in both modes, plus scrambled even array.
- `test_convergence.cpp` — fixed-seed sphere run converges (`best_val < 0.01`; uses `g0 = 10.0`).
- `test_validation.cpp` — constructor rejects empty/mismatched bounds, zero agents, zero iterations, `lower > upper` with `std::invalid_argument`.

Build manually with:

```bash
g++.exe -O3 -std=c++20 -Iinclude \
    -I<path_to_nlohmann_json_include> \
    -I<path_to_xoshiro_cpp_include> \
    tests/test_main.cpp tests/tasks/test_history.cpp \
    tests/tasks/test_stats.cpp tests/tasks/test_determinism.cpp \
    tests/tasks/test_modes.cpp tests/tasks/test_thread_safety.cpp \
    tests/tasks/test_median.cpp tests/tasks/test_convergence.cpp \
    tests/tasks/test_validation.cpp -o gsa_test.exe
```

Run (or use `ctest --preset conan-release`):

```bash
./gsa_test.exe
./gsa_test.exe determinism
```