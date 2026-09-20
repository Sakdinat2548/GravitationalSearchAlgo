"""Time C++ GSA (bench.exe) vs pure-Python GSA (gsa_pure.py) on identical workloads.

Usage (from the repo root, .venv active):
    python scripts/benchmark_gsa_timing.py [--runs 3 --iters 1000 ...]

Both sides run the same protocol: same dims/bounds/objective/population/
iters/seeds (trajectories still differ: Xoshiro vs Python RNG — this measures
speed only, not quality). Prints per-run means, totals, and the C++/Python
speedup; saves timing.json in a fresh timestamped
exports/timing_<yyyymmdd_hhmmss>/ folder (per-objective gsa_avg.csv lands in
subfolders next to it).
"""


def fresh_dir(base):
    # UTC, matching the std::chrono timestamps bench.exe generates.
    stamp = datetime.now(timezone.utc).strftime("timing_%Y%m%d_%H%M%S")
    outdir = Path(base) / stamp
    dup = 2
    while outdir.exists():
        outdir = Path(f"{base}/{stamp}_{dup}")
        dup += 1
    return outdir
import argparse
import json
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

import gsa_pure
from objective import FUNCTIONS


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--runs", type=int, default=3)
    p.add_argument("--iters", type=int, default=1000)
    p.add_argument("--agents", type=int, default=50)
    p.add_argument("--dims", type=int, default=30)
    p.add_argument("--lower", type=float, default=-30.0)
    p.add_argument("--upper", type=float, default=30.0)
    p.add_argument("--objectives", nargs="+", default=["sphere", "rosenbrock"],
                   choices=sorted(FUNCTIONS))
    p.add_argument("--seed-base", type=int, default=1000)
    p.add_argument("--g0", type=float, default=100.0)
    p.add_argument("--alpha", type=float, default=20.0)
    p.add_argument("--bench-exe", default="build/Release/bench.exe")
    a = p.parse_args()

    outdir = fresh_dir("exports")
    outdir.mkdir(parents=True, exist_ok=True)
    bench = Path(a.bench_exe)
    if not bench.exists():
        sys.exit(f"bench executable not found: {bench} (build it first)")

    report = {"protocol": vars(a), "results": {}}
    for name in a.objectives:
        fn = FUNCTIONS[name]
        lower = [a.lower] * a.dims
        upper = [a.upper] * a.dims

        t = time.perf_counter()
        subprocess.run([
            str(bench), str(a.runs), str(a.iters), str(a.agents),
            str(a.dims), str(a.lower), str(a.upper), str(a.seed_base),
            name, str(a.g0), str(a.alpha),
            str(outdir / name),
        ], check=True, capture_output=True)
        cpp = time.perf_counter() - t

        t = time.perf_counter()
        for r in range(a.runs):
            gsa_pure.run(fn, lower, upper, a.agents, a.iters,
                       a.g0, a.alpha, True, a.seed_base + r)
        py = time.perf_counter() - t

        report["results"][name] = {
            "cpp_total_s": cpp, "py_total_s": py,
            "cpp_per_run_s": cpp / a.runs, "py_per_run_s": py / a.runs,
            "speedup": py / cpp,
        }
        print(f"{name}: C++ {cpp / a.runs:.3f}s/run, "
              f"Python {py / a.runs:.3f}s/run, speedup {py / cpp:.0f}x",
              flush=True)

    (outdir / "timing.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"wrote {outdir / 'timing.json'}")


main()
