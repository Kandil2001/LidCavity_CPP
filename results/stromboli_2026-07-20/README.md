# Stromboli smoke test — 20 July 2026

This directory archives the successful GCC 8.5.0 build and smoke execution performed on the Stromboli HPC cluster.

## Configuration

- mode: `smoke`
- grid: `N = 16`
- Reynolds number: `100`
- convection scheme: upwind
- pressure solver: RBGS
- outer iterations: `20`
- measured runtime: approximately `0.01 s`

## Interpretation

The case reached the configured maximum iteration count and was reported as `quality=needs_improvement`. The Ghia profile thresholds were not met:

```text
u L2 = 1.719e-1
v L2 = 1.298e-1
```

This does not indicate a failed smoke test. The case is intentionally too small and too short for numerical validation. It checks only that the project:

- compiles with the available C++17 toolchain
- links `std::filesystem` correctly on GCC 8
- starts and completes without crashing
- accepts the smoke-mode arguments
- writes the expected CSV output

Use the full study data and validation figures in the main repository for numerical interpretation.

## Main files

- `logs/smoke_test_retry.log` — successful build and execution log
- `data/study_summary_smoke.csv` — one-row smoke-study summary
- additional files under `data/` — generated smoke-case output, where retained
