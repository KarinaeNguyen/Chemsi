"""
VFEP Visualization Dashboard
---------------------------
Reads a VFEP_Sim CSV output (default: most recent CSV in ../data/outputs) and plots
key variables for quick debugging and demos.

Usage:
  python visualize_vfep.py
  python visualize_vfep.py --csv ../data/outputs/high_fidelity_ml.csv
  python visualize_vfep.py --csv path/to/file.csv --save dashboard.png
  python visualize_vfep.py --no-show --save dashboard.png
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Optional

import pandas as pd
import matplotlib.pyplot as plt


DEFAULT_OUTPUT_DIR = Path(__file__).resolve().parent.parent / "data" / "outputs"

COLUMN_ALIASES = {
    "time": ["time_s", "Time", "t", "time"],
    "temp": ["T_K", "Temp", "temperature_K", "temp_K", "temperature"],
    "hrr": ["HRR_kW", "hrr_kW", "HRR", "hrr"],
    "o2": ["O2_volpct", "o2_volpct", "O2"],
    "co2": ["CO2_volpct", "co2_volpct", "CO2"],
    "h2o": ["H2O_volpct", "h2o_volpct", "H2O"],
    "agent": ["agent_mdot_kgps", "agent_mdot", "mdot_agent"],
    "reward": ["reward", "Reward", "r"],
}


def find_col(df: pd.DataFrame, names: List[str]) -> Optional[str]:
    for n in names:
        if n in df.columns:
            return n
    return None


def pick_latest_csv() -> Optional[Path]:
    if not DEFAULT_OUTPUT_DIR.exists():
        return None
    files = sorted(DEFAULT_OUTPUT_DIR.glob("*.csv"), key=lambda p: p.stat().st_mtime, reverse=True)
    return files[0] if files else None


def main() -> None:
    parser = argparse.ArgumentParser("VFEP CSV visualizer")
    parser.add_argument("--csv", help="Path to VFEP CSV output")
    args = parser.parse_args()

    csv_path = Path(args.csv).resolve() if args.csv else pick_latest_csv()
    if not csv_path or not csv_path.exists():
        print("ERROR: No VFEP CSV file found.")
        print("Run VFEP_Sim first or pass --csv <file>")
        return

    df = pd.read_csv(csv_path)

    time_col = find_col(df, COLUMN_ALIASES["time"])
    if not time_col:
        print("ERROR: No time column found in CSV.")
        print("Columns:", list(df.columns))
        return

    temp_col = find_col(df, COLUMN_ALIASES["temp"])
    hrr_col = find_col(df, COLUMN_ALIASES["hrr"])
    o2_col = find_col(df, COLUMN_ALIASES["o2"])
    co2_col = find_col(df, COLUMN_ALIASES["co2"])
    h2o_col = find_col(df, COLUMN_ALIASES["h2o"])
    agent_col = find_col(df, COLUMN_ALIASES["agent"])
    reward_col = find_col(df, COLUMN_ALIASES["reward"])

    fig, axes = plt.subplots(4, 1, figsize=(12, 9), sharex=True)

    if temp_col:
        axes[0].plot(df[time_col], df[temp_col])
        axes[0].set_ylabel("Temp (K)")
        axes[0].set_title("Thermal Response")
        axes[0].grid(True)

    if hrr_col:
        axes[1].plot(df[time_col], df[hrr_col])
        axes[1].set_ylabel("HRR (kW)")
        axes[1].set_title("Heat Release Rate")
        axes[1].grid(True)

    gases_plotted = False
    if o2_col:
        axes[2].plot(df[time_col], df[o2_col], label="O2")
        gases_plotted = True
    if co2_col:
        axes[2].plot(df[time_col], df[co2_col], label="CO2")
        gases_plotted = True
    if h2o_col:
        axes[2].plot(df[time_col], df[h2o_col], label="H2O")
        gases_plotted = True

    if gases_plotted:
        axes[2].set_ylabel("Vol %")
        axes[2].set_title("Gas Composition")
        axes[2].legend()
        axes[2].grid(True)

    if agent_col:
        axes[3].plot(df[time_col], df[agent_col], label="Agent ṁ (kg/s)")
    if reward_col:
        axr = axes[3].twinx()
        axr.plot(df[time_col], df[reward_col], "--", label="Reward")

    axes[3].set_title("Suppression & Reward")
    axes[3].set_xlabel("Time (s)")
    axes[3].grid(True)

    fig.suptitle(f"VFEP Dashboard — {csv_path.name}", fontsize=14)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
