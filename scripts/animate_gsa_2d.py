#!/usr/bin/env python3
import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a frame-by-frame 2D animation of GSA particle movement and mass growth.")
    parser.add_argument("--input", "-i", required=True,
                        help="Path to gsa_particles_2d.csv")
    parser.add_argument("--output", "-o", default="gsa_gravity_2d.gif",
                        help="Path to save the animation")
    parser.add_argument("--fps", type=int, default=12,
                        help="Frames per second for the animation")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise FileNotFoundError(f"CSV file not found: {input_path}")

    df = pd.read_csv(input_path)
    required = {"iteration", "agent", "x", "y", "mass", "fitness"}
    missing = required.difference(df.columns)
    if missing:
        raise ValueError(f"CSV missing required columns: {sorted(missing)}")

    fig, ax = plt.subplots(figsize=(8, 8))
    x_min = df["x"].min() - 0.5
    x_max = df["x"].max() + 0.5
    y_min = df["y"].min() - 0.5
    y_max = df["y"].max() + 0.5
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_aspect("equal", adjustable="box")
    ax.set_title("GSA 2D Gravity Evolution")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid(True, alpha=0.3)

    frames = sorted(df["iteration"].unique().tolist())
    agents = sorted(df["agent"].unique().tolist())
    scatters = []
    trails = []
    best_marker = ax.scatter(np.empty((0, 2)), np.empty((0, 2)), s=60,
                            color="tab:orange", edgecolors="black",
                            linewidth=0.5, zorder=5)

    for _ in agents:
        line, = ax.plot([], [], color="tab:blue", alpha=0.45, linewidth=1.0)
        trails.append(line)
        scat, = ax.plot([], [], linestyle="", marker="o", markersize=7,
                        color="tab:blue", alpha=0.9)
        scatters.append(scat)

    def init():
        for line in trails:
            line.set_data([], [])
        for scat in scatters:
            scat.set_data([], [])
        best_marker.set_offsets(np.empty((0, 2)))
        return trails + scatters + [best_marker]

    def update(frame_idx):
        frame = frames[frame_idx]
        frame_df = df[df["iteration"] == frame].copy()
        max_mass = frame_df["mass"].max()
        if pd.isna(max_mass):
            max_mass = 1.0

        for agent_id, line in zip(agents, trails):
            history = df[(df["agent"] == agent_id) & (df["iteration"] <= frame)]
            if history.empty:
                line.set_data([], [])
                continue
            line.set_data(history["x"].to_numpy(), history["y"].to_numpy())

        for agent_id, scat in zip(agents, scatters):
            row = frame_df[frame_df["agent"] == agent_id]
            if row.empty:
                scat.set_data([], [])
                continue
            mass = float(row["mass"].iloc[0])
            fitness = float(row["fitness"].iloc[0])
            x = float(row["x"].iloc[0])
            y = float(row["y"].iloc[0])

            alpha = 0.3 + 0.7 * (mass / max_mass if max_mass > 0 else 0.0)
            color = "tab:red" if agent_id == 0 else "tab:blue"
            scat.set_color(color)
            scat.set_alpha(alpha)
            scat.set_markersize(4 + 10 * (mass / max_mass if max_mass > 0 else 0.0))
            scat.set_data([x], [y])

        # highlight the best particle in the current frame
        best_row = frame_df.loc[[frame_df["fitness"].idxmin()]]
        if not best_row.empty:
            best_marker.set_offsets(best_row[["x", "y"]].to_numpy())
        else:
            best_marker.set_offsets(np.empty((0, 2)))
        return trails + scatters + [best_marker]

    anim = FuncAnimation(fig, update, frames=range(len(frames)), init_func=init,
                         interval=1000 / args.fps, blit=False, repeat=False)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    anim.save(output_path, writer="pillow", fps=args.fps)
    print(f"Saved GSA animation to {output_path}")


if __name__ == "__main__":
    main()
