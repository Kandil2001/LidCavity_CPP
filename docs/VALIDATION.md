# Validation

The solver output is compared with the classical Ghia et al. lid-driven cavity benchmark data.

## What is compared

The code compares two centerline profiles:

- horizontal velocity `u` along the vertical centerline
- vertical velocity `v` along the horizontal centerline

For each case, the solver reports `L2` and `Linf` errors against the benchmark values available for the tested Reynolds numbers.

## Interpretation

The validation thresholds used in the code are practical comparison limits. They classify numerical setups by profile agreement, but they do not constitute a complete verification and validation study.

From the completed study:

- 22 of 36 cases met the selected Ghia centerline-error threshold
- the `Re = 100`, `N = 128`, central-difference cases showed the strongest agreement
- the high-Reynolds-number cases show the expected flow structure but require additional convergence tuning

## Important distinction

The validation results must be read together with the residual history and termination status.

All cases in the full study reached the configured maximum outer-iteration limit. Therefore:

- the cases executed successfully
- some cases met the selected Ghia profile-error threshold
- execution completion does not prove residual convergence
- the high-Reynolds-number results should not be presented as fully converged final benchmark solutions

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.
