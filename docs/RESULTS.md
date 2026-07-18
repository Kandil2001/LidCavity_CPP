# Results

This page summarizes the completed serial C++ study saved in:

```text
results/data/study_summary_full.csv
```

## Study size

The study contains 36 configured cases:

```text
3 meshes × 3 Reynolds numbers × 2 convection schemes × 2 pressure solvers × 1 serial C++ implementation
= 36 simulations
```

The implementation is labeled:

```text
serial_cpp
```

It serves as the completed serial baseline for the broader work-in-progress multi-language comparison.

## Main result story

The solver behaves better on the refined grid. This is expected for the lid-driven cavity benchmark because centerline velocity profiles and corner-vortex behavior become more sensitive at higher Reynolds numbers.

From the completed study:

- `36` simulations executed
- `22/36` cases met the selected Ghia centerline validation threshold
- `12/12` refined-grid cases with `N = 128` met the selected validation threshold
- the `N = 128` central-difference cases produced the best profile agreement
- RBSOR produced similar validation errors to RBGS with substantially lower runtime
- all full-study cases reached the configured maximum outer-iteration limit

Therefore, execution completion, validation-threshold status, and residual convergence must be interpreted separately.

A useful interpretation is:

> The coarse-grid cases are useful for speed and sensitivity checks, while the refined-grid cases provide the strongest profile agreement. The current results are not a formal verification or time-to-convergence study.

## Best validation case at each Reynolds number

| Re | Case | N | Scheme | Pressure solver | Ghia u L2 | Ghia v L2 | Runtime [s] |
|---:|---:|---:|---|---|---:|---:|---:|
| 100 | 28 | 128 | central | RBSOR | 0.0031 | 0.0041 | 441.7 |
| 400 | 32 | 128 | central | RBSOR | 0.0539 | 0.0652 | 527.6 |
| 1000 | 36 | 128 | central | RBSOR | 0.1102 | 0.1109 | 647.6 |

These cases combine the strongest recorded profile agreement with the faster pressure solver.

## Pressure solver comparison

| Pressure solver | Cases | Mean runtime [s] | Total runtime [s] | Mean Poisson iterations |
|---|---:|---:|---:|---:|
| RBGS | 18 | 790.2 | 14224.3 | 1333.1 |
| RBSOR | 18 | 176.6 | 3178.9 | 264.5 |

RBSOR was about **4.5× faster** overall and used about **5× fewer pressure iterations** on average than RBGS for the recorded configuration.

This makes RBSOR the better default choice for this solver configuration.

## Mesh refinement effect

The profile agreement improves with mesh refinement. At `Re = 1000` with the central scheme and RBSOR:

| N | Case | Ghia u L2 | Ghia v L2 | Validation threshold |
|---:|---:|---:|---:|---|
| 32 | 12 | 0.2091 | 0.3051 | did not meet |
| 64 | 24 | 0.1810 | 0.2428 | did not meet |
| 128 | 36 | 0.1102 | 0.1109 | met |

The recorded error decreases as the grid becomes finer, which is the expected direction for this setup.

## Runtime

The complete 36-case configuration took approximately **4.83 hours** on the machine used to generate the uploaded results.

The runtime values describe the configured executions. Because every full-study case reached the maximum outer-iteration limit, they should not be interpreted as time-to-convergence measurements.

## Selected plots

![Ghia error summary](../assets/figures/study_ghia_error.svg)

![Pressure solver iterations](../assets/figures/study_pressure_solver_iterations.svg)

## Completed scope

The repository records a completed serial implementation and parameter study. Documented limitations remain:

1. convergence stopping criteria require improvement
2. high-Reynolds-number cases require additional tuning
3. the C++ code could be divided into smaller modules
4. the broader comparison project is adding OpenMP, MPI, CUDA, and other implementations

These limitations define follow-up research rather than making the recorded study incomplete.
