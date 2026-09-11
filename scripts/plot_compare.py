"""Plot exports/bench/{gsa,meta,ga}_avg.csv as one 3-line graph.

Usage (from the repo root, .venv active):
    python scripts/plot_compare.py [bench_dir]

Writes compare.png next to the CSVs (semilogy, one line per algo).
"""
import csv
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read(path):
    with open(path, newline="") as f:
        rows = list(csv.DictReader(f))
    return [int(r["iter"]) for r in rows], \
        [float(r["avg_best"]) for r in rows]


def main():
    bench = Path(sys.argv[1]) if len(sys.argv) > 1 else \
        Path("exports/bench")
    fig, ax = plt.subplots()
    styles = {"gsa": "-", "meta": "--", "ga": "--"}
    for name in ("gsa", "meta", "ga"):
        iters, avg = read(bench / f"{name}_avg.csv")
        ax.semilogy(iters, avg, label=name, linestyle=styles[name])
    ax.set(xlabel="iteration", ylabel="average best-so-far",
           title="GSA vs simple metaheuristic vs genetic")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    fig.savefig(bench / "compare.png", dpi=100)
    plt.close(fig)
    print(f"wrote {bench / 'compare.png'}")


main()
