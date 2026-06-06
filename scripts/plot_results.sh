#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

if ! python3 - <<'PY' >/dev/null 2>&1
import numpy, pandas, matplotlib
PY
then
  echo "Installing Python plotting requirements..."
  python3 -m pip install -r requirements.txt
fi

if [ "$#" -eq 0 ]; then
  set -- --all-cases --summaries all
fi

python3 postprocess/plot_cpp_results.py "$@"
