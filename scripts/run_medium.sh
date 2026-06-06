#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Medium C++ study matching MATLAB meshes/Re choices:
# meshes [32,64], Re [100,400,1000].
"$ROOT_DIR/bin/lid_cavity" --mode medium
