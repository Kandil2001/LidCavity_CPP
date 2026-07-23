#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Complete 36-case convergence study. RBGS and the largest grids are slower.
"$ROOT_DIR/bin/lid_cavity" --mode full --strict
