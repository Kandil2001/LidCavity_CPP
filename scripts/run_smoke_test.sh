#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Very small test only to check compilation and file output.
"$ROOT_DIR/bin/lid_cavity" --mode smoke --no-fields
