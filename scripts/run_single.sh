#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Representative README case:
# N=128, Re=1000, central differencing, RBSOR pressure solver, serial C++ implementation.
"$ROOT_DIR/bin/lid_cavity" --single --N 128 --Re 1000 --scheme central --pressure RBSOR --implementation serial_cpp
