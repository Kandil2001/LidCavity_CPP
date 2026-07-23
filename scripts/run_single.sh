#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"
bash "$ROOT_DIR/scripts/build.sh"

# Canonical Phase 2 regression case. It should finish with status=converged.
"$ROOT_DIR/bin/lid_cavity" \
    --single \
    --N 32 \
    --Re 100 \
    --scheme upwind \
    --pressure RBSOR \
    --strict
