#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
rm -rf "$ROOT_DIR/bin" "$ROOT_DIR/results/data"/*.csv
mkdir -p "$ROOT_DIR/results/data"
echo "Cleaned bin/ and results/data/*.csv"
