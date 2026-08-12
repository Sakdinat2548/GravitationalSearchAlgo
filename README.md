# Gravitational Search Algorithm

This project implements a Gravitational Search Algorithm (GSA) in C++.

## Build

Requires [CMake](https://cmake.org) (3.20+) and [Ninja](https://ninja-build.org), both on `PATH`.
The `default` preset configures a Ninja, Release build in `build/` with
`-O3 -DNDEBUG` (MSYS2 UCRT64 g++).

### Configure (one time, or after editing `CMakeLists.txt`)

```bash
cmake --preset default
```

This generates the Ninja build files under `build/`.

### Build

```bash
cmake --build --preset default
```

Produces `build/main.exe` (API demo) and `build/gsa_test.exe` (invariant/determinism test harness).

### Run

```bash
./build/main.exe          # 4-example API demo (prints best_val + position)
./build/gsa_test.exe      # GSA invariants + same-seed determinism checks
```

### Quick syntax check (warning-free sources)

```bash
cmake --build --preset default --target check-gsa
```

### Test (GSA invariants + determinism)

```bash
ctest --preset default
```

Runs 4 CTest tests (`gsa_history`, `gsa_stats`, `gsa_determinism`, `gsa_modes`); see
`test/` for the sources. Run the binary directly to see per-test output, or pass a
single test name to run only that one:

```bash
./build/gsa_test.exe
./build/gsa_test.exe determinism
```

### Manual build (no CMake)

Alternatively, compile directly with g++ (O3, C++20, include the implementation source file):

```bash
g++.exe -O3 -std=c++20 main.cpp gsa.cpp -o main.exe
```

## Usage

This repository exposes a small C++ API in `gsa.hpp` / `gsa.cpp`.
The primary entry point is the class `GravitationalSearchAlgorithm`.

Build then run the provided `main.cpp`, or use the class directly in your code. See
`test/` below for the invariant/determinism test harness.

Example: simple usage with scalar (equal) bounds

```cpp
#include "gsa.hpp"

double sphere(std::span<const double> x) {
	double s = 0.0; for (double v : x) s += v*v; return s;
}

GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere);
auto best = gsa.optimize();
std::cout << "best value: " << best.best_val << "\n";
```

Example: per-dimension bounds and custom configuration

```cpp
GravitationalSearchAlgorithm gsa2(
	{-10.0, 0.0, -1.0},
	{10.0, 50.0, 1.0},
	sphere,
	{.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0, .minimize = true}
);
auto best2 = gsa2.optimize();
```

Example: using a lambda objective

```cpp
GravitationalSearchAlgorithm gsa3(2, -65.53, 65.53,
	[](std::span<const double> x){ return std::sin(x[0]) + std::cos(x[1]); },
	{.n_agents = 40, .max_iter = 500}
);
auto res3 = gsa3.optimize();
```
   
And accessing the result fields:

```cpp
GsaResult res = gsa.optimize();
std::vector<double> position = res.best_pos;           // best point found
std::vector<GsaIterationInfo> history = res.history;  // per-iteration stats
double last_mean = res.history.back().mean_fitness;   // mean fitness, last iter
```

Each `GsaIterationInfo` records for its iteration: `best_so_far`,
`best_iter`, `worst_iter`, `mean_fitness`, `median_fitness`, and `stddev_fitness`.

API notes
- `GsaConfig` controls `n_agents`, `max_iter`, `g0`, `alpha`, `minimize`, and `seed`.
- `optimize()` returns a `GsaResult` with `best_val`, `best_pos`, and `history`.
- `optimize()` is `const` and thread-safe: concurrent calls on the same instance are fine (your objective function must be thread-safe). Each call is an independent run.

Configuration tuning

- `n_agents` (int): number of agents/particles. Default: `40`.
- `max_iter` (int): number of iterations. Default: `500`.
- `g0` (double): initial gravitational constant.
Default: `100.0`.
- `alpha` (double): decay rate for `G(t) = g0 * exp(-alpha * t)`. Default: `20`.
- `minimize` (bool): set `true` to minimize objective, `false` to maximize. Default: `true`.
- `seed` (uint64_t): RNG seed for reproducible runs; `0` (default) derives a seed from `std::random_device`.

**Tip:** If the solver converges too quickly or gets stuck, try increasing `g0` or
decreasing `alpha` to encourage more exploration early in the run; to make the
search more conservative, decrease `g0` or increase `alpha`.

## Tests

`test/` holds a small self-contained framework (`test_framework.hpp` registers and runs
named tests; `test_common.hpp` shares objectives and invariant helpers). Each `*.cpp`
defines one test, and CMake registers each as its own CTest entry:

- `test/test_history.cpp` — history size `== max_iter + 1`, monotonic `best_so_far`,
  mean/median within `[best_iter, worst_iter]`, finite non-negative `stddev` (both modes).
- `test/test_stats.cpp` — history stats finite and in-range across objectives.
- `test/test_determinism.cpp` — same-seed bit-identical results on sphere/rosenbrock/Shekel.
- `test/test_modes.cpp` — `minimize` true/false and odd agent count (median).

Build manually with:

```bash
g++.exe -O3 -std=c++20 test/test_main.cpp test/test_history.cpp test/test_stats.cpp \
    test/test_determinism.cpp test/test_modes.cpp gsa.cpp -o gsa_test.exe
```

Run (or use `ctest --preset default`):

```bash
./gsa_test.exe
```
