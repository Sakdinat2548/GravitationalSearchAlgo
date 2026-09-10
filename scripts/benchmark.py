"""Run both Python algos N times (distinct deterministic seeds) and
write averaged best-so-far CSVs.

Usage (from the repo root, .venv active):
    python scripts/benchmark.py [--runs 30 --iters 1000 --agents 30 ...]

Writes exports/bench/meta_avg.csv and ga_avg.csv
(header iter,avg_best,median_best,std_best,avg_mean; std is population std).
"""

import argparse
import csv
import statistics
from pathlib import Path

import genetic
import simple_metaheuristic
from objective import FUNCTIONS


def col_avg(trails):
    n = len(trails[0])
    return [sum(t[i] for t in trails) / len(trails) for i in range(n)]


def average(trails):
    n = len(trails[0])
    return (
        col_avg(trails),
        [statistics.median([t[i] for t in trails]) for i in range(n)],
        [statistics.pstdev([t[i] for t in trails]) for i in range(n)],
    )


def write(path, avg, med, std, mean):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="") as f:
        out = csv.writer(f)
        out.writerow(["iter", "avg_best", "median_best", "std_best",
                      "avg_mean"])
        for i, (a, m, s, u) in enumerate(zip(avg, med, std, mean)):
            out.writerow([i, f"{a:.17g}", f"{m:.17g}", f"{s:.17g}",
                          f"{u:.17g}"])


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--runs", type=int, default=30)
    p.add_argument("--iters", type=int, default=5000)
    p.add_argument("--agents", type=int, default=50)
    p.add_argument("--dims", type=int, default=2)
    p.add_argument("--lower", type=float, default=-30.0)
    p.add_argument("--upper", type=float, default=30.0)
    p.add_argument("--objective", default="rosenbrock", choices=FUNCTIONS)
    p.add_argument("--seed-base", type=int, default=1000)
    p.add_argument("--p-head", type=float, default=0.5)
    p.add_argument("--step", type=float, default=0.01)
    p.add_argument("--pc", type=float, default=0.8)
    p.add_argument("--pm", type=float, default=0.0075)
    p.add_argument("--noise", type=float, default=0.01)
    p.add_argument("--outdir", default="exports/bench")
    a = p.parse_args()

    fn = FUNCTIONS[a.objective]
    lower = [a.lower] * a.dims
    upper = [a.upper] * a.dims
    outdir = Path(a.outdir)

    meta_best, meta_mean = zip(*[
        simple_metaheuristic.run(
            fn, lower, upper, a.agents, a.iters, a.p_head, a.step, a.seed_base + i
        )
        for i in range(a.runs)
    ])
    avg, med, std = average(meta_best)
    write(outdir / "meta_avg.csv", avg, med, std, col_avg(meta_mean))
    print(f"meta: {a.runs} runs, final avg_best={avg[-1]:.6g}")

    ga_best, ga_mean = zip(*[
        genetic.run(
            fn, lower, upper, a.agents, a.iters, a.pc, a.pm, a.noise, a.seed_base + i
        )
        for i in range(a.runs)
    ])
    avg, med, std = average(ga_best)
    write(outdir / "ga_avg.csv", avg, med, std, col_avg(ga_mean))
    print(f"ga: {a.runs} runs, final avg_best={avg[-1]:.6g}")


main()
