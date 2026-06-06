#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Full C++ study using the same MATLAB meshes/Re/schemes/pressure solvers, with one serial_cpp implementation.
# This can take a long time depending on your CPU.
"$ROOT_DIR/bin/lid_cavity" --mode full
