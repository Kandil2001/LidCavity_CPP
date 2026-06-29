#!/usr/bin/env python3
"""
Create mesh-comparison validation plots for the C++ lid-driven cavity study.

This script is meant for the full-study output. It compares the centreline
profiles from several meshes against the Ghia et al. data on the same figure.
That is usually more useful for validation than showing only one mesh.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from plot_cpp_results import GHIA_DATA, load_field_grid, parse_case_name, sorted_csv_files


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def save(path_without_suffix: Path) -> None:
    ensure_dir(path_without_suffix.parent)
    plt.tight_layout()
    plt.savefig(path_without_suffix.with_suffix(".png"), dpi=220, bbox_inches="tight")
    plt.close()


def select_cases(data_dir: Path, scheme: str, pressure: str) -> Dict[int, List[Path]]:
    cases: Dict[int, List[Path]] = {}

    for field_csv in sorted_csv_files(data_dir, "_fields"):
        info = parse_case_name(field_csv)
        re_value = info.get("Re")
        if not isinstance(re_value, int) or re_value not in GHIA_DATA:
            continue
        if str(info.get("scheme", "")).lower() != scheme.lower():
            continue
        if str(info.get("pressure", "")).lower() != pressure.lower():
            continue
        cases.setdefault(re_value, []).append(field_csv)

    for re_value in cases:
        cases[re_value] = sorted(cases[re_value], key=lambda path: int(parse_case_name(path).get("N", 0)))

    return cases


def plot_mesh_validation(data_dir: Path, fig_dir: Path, scheme: str, pressure: str) -> int:
    cases = select_cases(data_dir, scheme, pressure)
    created = 0

    for re_value, files in cases.items():
        if len(files) < 2:
            continue

        ghia = GHIA_DATA[re_value]

        plt.figure(figsize=(7.2, 5.6))
        for field_csv in files:
            info = parse_case_name(field_csv)
            x, y, fields = load_field_grid(field_csv)
            mid_j = int(np.argmin(np.abs(x - 0.5)))
            plt.plot(fields["u"][:, mid_j], y, linewidth=1.4, label=f"N={info.get('N')}")
        plt.plot(ghia["u"], ghia["y_u"], "ko", markersize=4, label="Ghia et al.")
        plt.grid(True, alpha=0.30)
        plt.xlabel("u velocity at x = 0.5")
        plt.ylabel("y")
        plt.legend(loc="best")
        plt.title(f"u centreline mesh validation, Re={re_value}")
        save(fig_dir / f"mesh_validation_u_Re{re_value}_{scheme}_{pressure}")
        created += 1

        plt.figure(figsize=(7.2, 5.6))
        for field_csv in files:
            info = parse_case_name(field_csv)
            x, y, fields = load_field_grid(field_csv)
            mid_i = int(np.argmin(np.abs(y - 0.5)))
            plt.plot(x, fields["v"][mid_i, :], linewidth=1.4, label=f"N={info.get('N')}")
        plt.plot(ghia["x_v"], ghia["v"], "ko", markersize=4, label="Ghia et al.")
        plt.grid(True, alpha=0.30)
        plt.xlabel("x")
        plt.ylabel("v velocity at y = 0.5")
        plt.legend(loc="best")
        plt.title(f"v centreline mesh validation, Re={re_value}")
        save(fig_dir / f"mesh_validation_v_Re{re_value}_{scheme}_{pressure}")
        created += 1

    return created


def main() -> int:
    parser = argparse.ArgumentParser(description="Create mesh-comparison validation plots.")
    parser.add_argument("--data-dir", default="results/data")
    parser.add_argument("--fig-dir", default="results/figures")
    parser.add_argument("--scheme", default="central")
    parser.add_argument("--pressure", default="RBSOR")
    args = parser.parse_args()

    data_dir = Path(args.data_dir)
    fig_dir = Path(args.fig_dir)

    if not data_dir.exists():
        print(f"No data directory found: {data_dir}")
        return 0

    created = plot_mesh_validation(data_dir, fig_dir, args.scheme, args.pressure)
    if created:
        print(f"Created {created} mesh-validation plots in {fig_dir}")
    else:
        print("No mesh-validation plots created. Run the full study first.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
