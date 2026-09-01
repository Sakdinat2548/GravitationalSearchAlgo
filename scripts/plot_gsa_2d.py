#!/usr/bin/env python3
import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a 2D gravitational particle view from GSA particle snapshots.")
    parser.add_argument("--input", "-i", required=True,
                        help="Path to gsa_particles_2d.csv")
    parser.add_argument("--output", "-o", default="gsa_gravity_2d.png",
                        help="Path to save the plot image")
    parser.add_argument("--title", default="GSA 2D Gravity Visualization",
                        help="Title shown on the plot")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise FileNotFoundError(f"CSV file not found: {input_path}")

    df = pd.read_csv(input_path)
    required = {"iteration", "agent", "x", "y"}
    missing = required.difference(df.columns)
    if missing:
        raise ValueError(f"CSV missing required columns: {sorted(missing)}")

    fig, ax = plt.subplots(figsize=(8, 8))
    frame_count = df["iteration"].nunique()
    if frame_count == 0:
        raise ValueError("No particle states were exported.")

    by_agent = df.groupby("agent")
    agent_paths = []
    best_positions = []
    for agent_id, group in by_agent:
        group = group.sort_values("iteration")
        x = group["x"].to_numpy()
        y = group["y"].to_numpy()
        agent_paths.append((x, y))
        best_positions.append((x[-1], y[-1]))

    # Show the global motion trail for each agent, and the final swarm centroid.
    for idx, (x, y) in enumerate(agent_paths):
        ax.plot(x, y, color="tab:blue", alpha=0.3, linewidth=0.8)
        ax.scatter([x[-1]], [y[-1]], s=20, color="tab:blue", alpha=0.8)

    final_x = [pt[0] for pt in best_positions]
    final_y = [pt[1] for pt in best_positions]
    if final_x:
        ax.scatter(final_x, final_y, s=25, c="tab:orange", edgecolors="black", linewidth=0.5)

    ax.set_title(args.title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_aspect("equal", adjustable="box")
    ax.grid(True, alpha=0.3)

    x_min = df["x"].min() - 0.5
    x_max = df["x"].max() + 0.5
    y_min = df["y"].min() - 0.5
    y_max = df["y"].max() + 0.5
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)

    plt.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output_path, dpi=200)
    print(f"Saved 2D gravity plot to {output_path}")


if __name__ == "__main__":
    main()
