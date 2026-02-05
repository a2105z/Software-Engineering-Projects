#!/usr/bin/env python3
"""
Plot SmartSignal log output. Reads logs/run.csv and shows lamp states over time.
Usage: python scripts/visualize_log.py [path/to/run.csv]
"""

import sys
import csv
from pathlib import Path

def main():
    if len(sys.argv) > 1:
        log_path = sys.argv[1]
    else:
        log_path = "logs/run.csv"
    if not Path(log_path).exists():
        log_path = Path(__file__).parent.parent / "logs" / "run.csv"
    if not Path(log_path).exists():
        print("No log file found. Run SmartSignal first to generate logs/run.csv")
        sys.exit(1)

    times = []
    ns_vals = []
    ew_vals = []
    siren_conf = []

    LAMP_TO_VAL = {"OFF": 0, "RED": 1, "YELLOW": 2, "GREEN": 3}

    with open(log_path, newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            times.append(float(row["time_ms"]) / 1000.0)
            ns_vals.append(LAMP_TO_VAL.get(row["ns"], 0))
            ew_vals.append(LAMP_TO_VAL.get(row["ew"], 0))
            siren_conf.append(float(row["siren_conf"]))

    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed. pip install matplotlib to plot.")
        sys.exit(1)

    fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, figsize=(10, 6))
    ax1.step(times, ns_vals, where="post", label="NS", color="green", linewidth=2)
    ax1.step(times, ew_vals, where="post", label="EW", color="orange", linewidth=2)
    ax1.set_ylabel("Lamp (0=OFF,1=RED,2=YELLOW,3=GREEN)")
    ax1.set_ylim(-0.2, 3.5)
    ax1.legend(loc="upper right")
    ax1.set_title("SmartSignal - Traffic Light State Over Time")
    ax1.grid(True, alpha=0.3)

    ax2.plot(times, siren_conf, label="Siren confidence", color="red", alpha=0.8)
    ax2.set_ylabel("Siren confidence")
    ax2.set_xlabel("Time (s)")
    ax2.set_ylim(-0.05, 1.05)
    ax2.legend(loc="upper right")
    ax2.grid(True, alpha=0.3)

    plt.tight_layout()
    out_path = Path(log_path).parent / "run_plot.png"
    plt.savefig(out_path, dpi=100)
    print(f"Saved plot to {out_path}")
    plt.show()

if __name__ == "__main__":
    main()
