#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Converged higher-Reynolds-number representative case.
"$ROOT_DIR/bin/lid_cavity" \
    --single \
    --N 32 \
    --Re 1000 \
    --scheme central \
    --pressure RBSOR \
    --tol-velocity 1e-7 \
    --strict
