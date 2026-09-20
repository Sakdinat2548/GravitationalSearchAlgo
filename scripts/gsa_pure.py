"""Pure-Python Gravitational Search Algorithm (timing baseline only).

Faithful plain-Python port of src/gsa/gsa.hpp: uniform init, best tracking,
mass normalization with the 1e-6 floor, k-best schedule (N -> 1 linearly),
O(N^2 * kbest * dims) accelerations with per-pair rand(), V = rand*V + A,
clamp to bounds. Stdlib only.

Not used for solution quality: Python's RNG differs from Xoshiro, so
trajectories differ from the C++ build by design. This exists to measure the
C++ speedup on an identical workload (same dims/agents/iters/objective).
Returns the best-so-far trail, index 0 = initial population.
"""
import math
import random

K_MASS_FLOOR = 1e-6
K_EPSILON = 1e-12


def run(fn, lower, upper, n_agents=50, max_iter=500, g0=100.0, alpha=20.0,
        minimize=True, seed=0):
    rng = random.Random(seed)
    dims = len(lower)
    pos = [[rng.uniform(lo, hi) for lo, hi in zip(lower, upper)]
           for _ in range(n_agents)]
    vel = [[0.0] * dims for _ in range(n_agents)]
    acc = [[0.0] * dims for _ in range(n_agents)]

    def better(a, b):
        return a < b if minimize else a > b

    global_best = math.inf if minimize else -math.inf
    trail = []
    for k in range(1, max_iter + 1):
        fit = [fn(p) for p in pos]
        for f in fit:
            if better(f, global_best):
                global_best = f
        trail.append(global_best)

        fmin, fmax = min(fit), max(fit)
        inv_diff = 1.0 / max(fmax - fmin, K_EPSILON)
        if minimize:
            mass = [max((fmax - f) * inv_diff, K_MASS_FLOOR) for f in fit]
        else:
            mass = [max((f - fmin) * inv_diff, K_MASS_FLOOR) for f in fit]
        total = max(sum(mass), K_EPSILON)
        mass = [m / total for m in mass]

        progress = k / max_iter
        kbest = max(1, min(n_agents,
                           int(n_agents - progress * (n_agents - 1))))
        order = sorted(range(n_agents), key=lambda i: fit[i],
                       reverse=not minimize)[:kbest]

        grav = g0 * math.exp(-alpha * k / max_iter)
        for i in range(n_agents):
            xi, mi = pos[i], mass[i]
            force = [0.0] * dims
            gmi = grav * mi
            for j in order:
                if i == j:
                    continue
                xj = pos[j]
                r2 = sum((a - b) ** 2 for a, b in zip(xi, xj))
                mag = gmi * mass[j] / (math.sqrt(r2) + K_EPSILON)
                for d in range(dims):
                    force[d] += rng.random() * mag * (xj[d] - xi[d])
            inv_m = 1.0 / (mi + K_EPSILON)
            for d in range(dims):
                acc[i][d] = force[d] * inv_m

        for i in range(n_agents):
            for d in range(dims):
                vel[i][d] = rng.random() * vel[i][d] + acc[i][d]
                v = pos[i][d] + vel[i][d]
                pos[i][d] = min(upper[d], max(lower[d], v))
    return trail
