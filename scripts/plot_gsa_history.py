#!/usr/bin/env python3
import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot GSA convergence history from a CSV exported by the C++ runner.")
    parser.add_argument("--input", "-i", required=True,
                        help="Path to gsa_history.csv")
    parser.add_argument("--output", "-o", default="gsa_convergence.png",
                        help="Path to save the plot image")
    parser.add_argument("--title", default="GSA Convergence",
                        help="Title shown on the plot")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise FileNotFoundError(f"CSV file not found: {input_path}")

    df = pd.read_csv(input_path)
    required = {
        "iteration",
        "best_so_far",
        "best_iter",
        "worst_iter",
        "mean_fitness",
        "median_fitness",
        "stddev_fitness",
    }
    missing = required.difference(df.columns)
    if missing:
        raise ValueError(f"CSV missing required columns: {sorted(missing)}")

    plt.figure(figsize=(10, 6))
    plt.plot(df["iteration"], df["best_so_far"], label="best_so_far", color="tab:blue", linewidth=2)
    plt.plot(df["iteration"], df["mean_fitness"], label="mean_fitness", color="tab:orange", linestyle="--", linewidth=1.5)
    plt.title(args.title)
    plt.xlabel("Iteration")
    plt.ylabel("Fitness")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output_path, dpi=200)
    print(f"Saved plot to {output_path}")


if __name__ == "__main__":
    main()
