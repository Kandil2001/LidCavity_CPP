#!/usr/bin/env bash
set -euo pipefail

build_dir="${1:-build/phase2-verification}"
cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure
