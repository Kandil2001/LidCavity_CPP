#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT_DIR/bin"

g++ -std=c++17 -O3 -march=native -Wall -Wextra -pedantic \
    "$ROOT_DIR/src/lid_cavity.cpp" \
    -o "$ROOT_DIR/bin/lid_cavity"

echo "Built: $ROOT_DIR/bin/lid_cavity"
