# Gravitational Search Algorithm

This project implements a Gravitational Search Algorithm (GSA) in C++.

## Build

Compile the program with g++ using O3, flto and include the implementation source file:

```bash
g++.exe -O3 -march=native -flto -std=c++20 main.cpp gsa.cpp -o main.exe
```

Run the executable:

```bash
./main.exe
```

## Usage

This repository exposes a small C++ API in `gsa.hpp` / `gsa.cpp`.
The primary entry point is the class `GravitationalSearchAlgorithm`.

Build then run the provided `main.cpp` examples, or use the class directly in your code.

Example: simple usage with scalar (equal) bounds

```cpp
#include "gsa.hpp"

double sphere(const std::vector<double>& x) {
	double s = 0.0; for (double v : x) s += v*v; return s;
}

GravitationalSearchAlgorithm gsa(3, -5.0, 5.0, sphere);
auto [best_val, best_pos] = gsa.optimize();
```

Example: per-dimension bounds and custom configuration

```cpp
GravitationalSearchAlgorithm gsa2(
	std::vector<double>{-10.0, 0.0, -1.0},
	std::vector<double>{10.0, 50.0, 1.0},
	sphere,
	{.n_agents = 50, .max_iter = 1000, .g0 = 10.0, .alpha = 10.0, .minimize = true}
);
auto [val2, pos2] = gsa2.optimize();
```

Example: using a lambda objective

```cpp
GravitationalSearchAlgorithm gsa3(2, -65.53, 65.53,
	[](const std::vector<double>& x){ return std::sin(x[0]) + std::cos(x[1]); },
	{.n_agents = 40, .max_iter = 500}
);
auto [v, p] = gsa3.optimize();
```

API notes
- `GsaConfig` controls `n_agents`, `max_iter`, `g0`, `alpha`, and whether to `minimize`.
- The implementation is single-threaded; if you call `optimize()` multiple times concurrently, protect the instance or create separate instances.

Configuration tuning

- `n_agents` (int): number of agents/particles. Default: `40`.
- `max_iter` (int): number of iterations. Default: `500`.
- `g0` (double): initial gravitational constant.
Default: `100.0`.
- `alpha` (double): decay rate for `G(t) = g0 * exp(-alpha * t)`. Default: `20`.
- `minimize` (bool): set `true` to minimize objective, `false` to maximize. Default: `true`.

**Tip:** If the solver converges too quickly or gets stuck, try increasing `g0` or
decreasing `alpha` to encourage more exploration early in the run; to make the
search more conservative, decrease `g0` or increase `alpha`.

## Examples folder

An example program is provided in `examples/simple_sphere.cpp` demonstrating usage and two presets. Build it with:

```bash
g++.exe -O3 -march=native -flto -std=c++20 examples/simple_sphere.cpp gsa.cpp -o examples/simple_sphere.exe
```

Run:

```bash
./examples/simple_sphere.exe
```

