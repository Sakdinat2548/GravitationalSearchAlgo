"""Simple loaded-coin metaheuristic (workshop notes section I.2).

Per iteration: find the best agent; each agent copies it with
probability p_head, then perturbs every dimension by uniform
±step*width (clamped to bounds); greedy accept on <=.
Returns (best-so-far, mean-fitness) lists, index 0 = initial population
(matches GSA history row-index = iteration).
"""
import random


def run(fn, lower, upper, n_agents=30, max_iter=1000, p_head=0.5,
        step=0.01, seed=0):
    rng = random.Random(seed)
    width = [hi - lo for lo, hi in zip(lower, upper)]
    pop = [[rng.uniform(lo, hi) for lo, hi in zip(lower, upper)]
           for _ in range(n_agents)]
    fit = [fn(p) for p in pop]
    best = min(fit)
    trail = [best]
    means = [sum(fit) / len(fit)]
    for _ in range(max_iter):
        champion = pop[fit.index(best)]
        for i in range(n_agents):
            x = list(champion) if rng.random() < p_head else list(pop[i])
            x = [min(hi, max(lo, v + rng.uniform(-step * w, step * w)))
                 for v, lo, hi, w in zip(x, lower, upper, width)]
            fx = fn(x)
            if fx <= fit[i]:
                pop[i], fit[i] = x, fx
                best = min(best, fx)
        trail.append(best)
        means.append(sum(fit) / len(fit))
    return trail, means
