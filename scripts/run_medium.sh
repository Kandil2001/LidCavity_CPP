#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build.sh"

# Six converged N=32 cases at Re=100, 400, and 1000 using upwind and central schemes.
"$ROOT_DIR/bin/lid_cavity" --mode medium --strict
