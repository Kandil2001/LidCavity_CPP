#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Four fast converged cases: N=24/32, Re=100, upwind/central, RBSOR.
"$ROOT_DIR/bin/lid_cavity" --mode quick --strict
