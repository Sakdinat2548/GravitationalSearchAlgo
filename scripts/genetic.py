"""Real-valued genetic algorithm (workshop notes section II.2,
"Modification" variant, worked-example settings).

Roulette-wheel selection on fit = 1/(1+f) (minimization adapter;
algorithm math itself is verbatim), crossover method d (random blend
+ zero-mean noise, clamped to bounds), mutation method b, elitism
(keep best). Returns best-so-far, index 0 = initial population.
"""
import random


def run(fn, lower, upper, n_agents=30, max_iter=1000, pc=0.8, pm=0.0075,
        noise=0.01, seed=0):
    rng = random.Random(seed)
    width = [hi - lo for lo, hi in zip(lower, upper)]

    def clamp(x):
        return [min(hi, max(lo, v)) for v, lo, hi in zip(x, lower, upper)]

    pop = [clamp([rng.uniform(lo, hi) for lo, hi in zip(lower, upper)])
           for _ in range(n_agents)]
    fit = [fn(p) for p in pop]
    best = min(fit)
    elite = list(pop[fit.index(best)])
    trail = [best]

    def roulette(fits, total):
        if total <= 0:
            return rng.randrange(len(fits))
        r = rng.random() * total
        acc = 0.0
        for i, f in enumerate(fits):
            acc += f
            if acc >= r:
                return i
        return len(fits) - 1

    for _ in range(max_iter):
        fits = [1.0 / (1.0 + f) for f in fit]
        total = sum(fits)
        mating = [list(pop[roulette(fits, total)]) for _ in range(n_agents)]
        idx = [i for i in range(n_agents) if rng.random() < pc]
        rng.shuffle(idx)
        for a, b in zip(idx[::2], idx[1::2]):
            x, y = mating[a], mating[b]
            alpha = rng.random()
            w1 = [rng.uniform(-noise * w, noise * w) for w in width]
            w2 = [rng.uniform(-noise * w, noise * w) for w in width]
            mating[a] = clamp([alpha * u + (1 - alpha) * v + w
                               for u, v, w in zip(x, y, w1)])
            mating[b] = clamp([(1 - alpha) * u + alpha * v + w
                               for u, v, w in zip(x, y, w2)])
        for i in range(n_agents):
            if rng.random() < pm:
                a = rng.random()
                w = [rng.uniform(lo, hi) for lo, hi in zip(lower, upper)]
                mating[i] = [a * u + (1 - a) * v
                             for u, v in zip(mating[i], w)]
        pop, fit = mating, [fn(p) for p in mating]
        cur = min(fit)
        if cur < best:
            best = cur
            elite = list(pop[fit.index(cur)])
        worst = fit.index(max(fit))
        pop[worst], fit[worst] = list(elite), best
        trail.append(best)
    return trail
