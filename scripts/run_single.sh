#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Representative README case from the MATLAB project:
# N=64, Re=100, central differencing, RBGS pressure solver, serial C++ implementation.
"$ROOT_DIR/bin/lid_cavity" --single --N 64 --Re 100 --scheme central --pressure RBGS --implementation serial_cpp
