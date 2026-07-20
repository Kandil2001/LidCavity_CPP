#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
BIN_DIR="$ROOT_DIR/bin"
OBJECT="$BIN_DIR/lid_cavity.o"
TARGET="$BIN_DIR/lid_cavity"

mkdir -p "$BIN_DIR"

"$CXX" -std=c++17 -O3 -march=native -Wall -Wextra -pedantic \
    -c "$ROOT_DIR/src/lid_cavity.cpp" \
    -o "$OBJECT"

if ! "$CXX" "$OBJECT" -o "$TARGET"; then
    echo "Initial link failed; retrying with -lstdc++fs for older GCC toolchains."
    "$CXX" "$OBJECT" -o "$TARGET" -lstdc++fs
fi

rm -f "$OBJECT"
echo "Built: $TARGET"
