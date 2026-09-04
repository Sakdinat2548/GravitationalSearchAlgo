"""Visualize a GSA run folder: convergence PNG, contour+scatter PNGs,
animated GIF.

Usage (from the repo root, .venv active):
    python scripts/plot_gsa.py exports/run_<yyyymmdd_hhmmss>

Writes convergence.png, contour_first.png, contour_last.png and
anim.gif next to the CSVs. Requires matplotlib + pillow.
"""

# ================= EDIT THIS IF YOU CHANGE objective.hpp =================
# Mirror of examples/objective.hpp for dims = 2. Used ONLY for the contour
# background; scatter, convergence and animation are purely data-driven.
def objective_2d(x, y):
    return 100.0 * (y - x**2) ** 2 + (x - 1.0) ** 2


LEVELS = [0.5, 2, 10, 50, 200, 1000]
# ==========================================================================

import csv
import json
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation, PillowWriter
from matplotlib.colors import LogNorm


def read_csv(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def as_list(v, dims, default):
    if isinstance(v, list):
        return [float(x) for x in v]
    return [float(v if v is not None else default)] * dims


def draw_contour(fig, ax, lo, hi):
    n = 300
    xs = np.linspace(lo[0], hi[0], n)
    ys = np.linspace(lo[1], hi[1], n)
    x, y = np.meshgrid(xs, ys)
    z = np.maximum(objective_2d(x, y), 1e-9)
    mesh = ax.pcolormesh(x, y, z, cmap="viridis", shading="gouraud",
                         norm=LogNorm())
    levels = [c for c in LEVELS if c < z.max()] or 8
    ax.contour(x, y, z, levels=levels, colors="white", linewidths=0.4,
               alpha=0.6)
    fig.colorbar(mesh, ax=ax, label="f")


def dot_sizes(mass):
    m = np.asarray(mass, dtype=float)
    peak = m.max() if m.max() > 0 else 1.0
    return 10 + 200 * (m / peak)


def scatter_points(ax, rows):
    x = np.array([float(r["x1"]) for r in rows])
    y = np.array([float(r["x2"]) for r in rows])
    return ax.scatter(x, y, s=dot_sizes([float(r["mass"]) for r in rows]),
                      c="red", edgecolors="black", linewidths=0.5,
                      alpha=0.9, label="agents", zorder=3)


def contour_frame(by_iter, k, lo, hi, title):
    fig, ax = plt.subplots()
    draw_contour(fig, ax, lo, hi)
    scatter_points(ax, by_iter[k])
    ax.set(xlim=(lo[0], hi[0]), ylim=(lo[1], hi[1]), xlabel="x1",
           ylabel="x2", title=title)
    ax.set_aspect("equal")
    ax.grid(True, alpha=0.3)
    ax.legend()
    return fig


def main():
    if len(sys.argv) < 2 or not (Path(sys.argv[1]) / "history.csv").exists():
        sys.exit("usage: plot_gsa.py RUN_DIR  (folder with history.csv)")
    run = Path(sys.argv[1])

    hist = read_csv(run / "history.csv")
    best = np.array([float(r["best_so_far"]) for r in hist])
    fig, ax = plt.subplots()
    ax.semilogy(np.arange(len(hist)), best)
    ax.set(xlabel="iteration", ylabel="best_so_far", title="convergence")
    ax.grid(True, which="both", alpha=0.3)
    fig.savefig(run / "convergence.png", dpi=100)
    plt.close(fig)

    snap_path = run / "snapshots.csv"
    if not snap_path.exists():
        print("no snapshots.csv; wrote convergence.png")
        return
    snaps = read_csv(snap_path)
    cfg = json.loads((run / "config.json").read_text()) \
        if (run / "config.json").exists() else {}
    dims = int(cfg.get("dimensions", 2))
    if dims < 2:
        print("dims < 2; wrote convergence.png")
        return
    lo = as_list(cfg.get("lower"), dims, -2.048)
    hi = as_list(cfg.get("upper"), dims, 2.048)

    by_iter = {}
    for r in snaps:
        by_iter.setdefault(int(r["iter"]), []).append(r)
    frames = sorted(by_iter)

    contour_frame(by_iter, frames[0], lo, hi,
                  f"iter {frames[0]}").savefig(run / "contour_first.png",
                                               dpi=100)
    plt.close("all")
    contour_frame(by_iter, frames[-1], lo, hi,
                  f"iter {frames[-1]}").savefig(run / "contour_last.png",
                                                dpi=100)
    plt.close("all")

    fig, ax = plt.subplots()
    if dims == 2:
        draw_contour(fig, ax, lo, hi)
    scat = scatter_points(ax, by_iter[frames[0]])
    ax.set(xlim=(lo[0], hi[0]), ylim=(lo[1], hi[1]), xlabel="x1",
           ylabel="x2")
    ax.set_aspect("equal")
    ax.grid(True, alpha=0.3)
    ax.legend()

    def update(k):
        rows = by_iter[k]
        scat.set_offsets(np.c_[ [float(r["x1"]) for r in rows],
                                [float(r["x2"]) for r in rows]])
        scat.set_sizes(dot_sizes([float(r["mass"]) for r in rows]))
        ax.set_title(f"iter {k}")
        return (scat,)

    FuncAnimation(fig, update, frames=frames).save(
        run / "anim.gif", writer=PillowWriter(fps=5))
    plt.close(fig)
    print(f"wrote convergence.png, contour_first/last.png, anim.gif in {run}")


main()
