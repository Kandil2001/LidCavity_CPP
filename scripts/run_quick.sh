#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Same reduced study as MATLAB main_quick.m:
# meshes [32,64], Re [100,400], schemes [upwind,central],
# pressure solvers [RBGS,RBSOR], one C++ implementation [serial_cpp].
"$ROOT_DIR/bin/lid_cavity" --mode quick
