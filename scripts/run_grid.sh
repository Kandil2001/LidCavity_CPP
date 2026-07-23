#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"
bash "$ROOT_DIR/scripts/build.sh"

# Re=100 central-difference grid sequence: N=16, 32, 64.
"$ROOT_DIR/bin/lid_cavity" --mode grid --strict
