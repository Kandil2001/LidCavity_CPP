#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"
rm -f "$ROOT_DIR/results/data/study_summary_single.csv"

"$ROOT_DIR/bin/lid_cavity" \
    --single \
    --N 32 \
    --Re 100 \
    --scheme upwind \
    --pressure RBSOR \
    --strict \
    --no-fields

python3 - "$ROOT_DIR/results/data/study_summary_single.csv" <<'PY'
import csv
import math
import sys

path = sys.argv[1]
rows = list(csv.DictReader(open(path, encoding="utf-8")))
if len(rows) != 1:
    raise SystemExit(f"Expected one canonical result, found {len(rows)}")
row = rows[0]
if row["Status"] != "converged":
    raise SystemExit(f"Canonical case did not converge: {row['Status']}")
if row["ValidationPass"] != "1":
    raise SystemExit("Canonical case failed the Ghia benchmark threshold")
if float(row["FinalVelocityLinf"]) > 1.0e-7:
    raise SystemExit("Velocity convergence regression")
if float(row["FinalRcDiv"]) > 1.0e-9:
    raise SystemExit("Divergence convergence regression")
if int(row["FailedPressureSolves"]) != 0:
    raise SystemExit("Pressure solver regression")
for key in ("Runtime_s", "Ghia_u_L2", "Ghia_v_L2"):
    if not math.isfinite(float(row[key])):
        raise SystemExit(f"Non-finite canonical metric: {key}")
print("Canonical staggered-grid regression passed.")
PY
