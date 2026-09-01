#!/usr/bin/env python3
"""
Quick launcher for the GSA interactive visualizer.
Runs: streamlit run scripts/visualizer_interactive.py
"""

import subprocess
import sys
from pathlib import Path

if __name__ == "__main__":
    script = Path(__file__).parent / "scripts" / "visualizer_interactive.py"
    cmd = [sys.executable, "-m", "streamlit", "run", str(script)]
    print(f"🚀 Launching: {' '.join(cmd)}")
    print("📊 Opening browser to http://localhost:8501")
    subprocess.run(cmd)
