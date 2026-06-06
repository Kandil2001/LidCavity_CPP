# Validation

The solver is validated against the classical Ghia et al. lid-driven cavity benchmark data.

## What is compared

The code compares two centreline profiles:

- horizontal velocity `u` along the vertical centreline,
- vertical velocity `v` along the horizontal centreline.

For each case, the solver reports L2 and Linf errors against the benchmark values available for the tested Reynolds numbers.

## Current interpretation

The validation thresholds used in the code are practical comparison limits. They help classify which numerical setups are behaving reasonably, but they are not a full verification and validation study.

From the current full study:

- 22 out of 36 cases passed the selected Ghia centreline-error threshold.
- The `Re = 100`, `N = 128`, central-difference cases showed the best agreement.
- The high-Reynolds-number cases show the expected flow structure, but still need convergence and parameter tuning.

## Important notes

The validation results should be read together with the residual history and maximum-iteration status. In the current run, all cases reached the configured maximum outer-iteration limit. This means the data is useful for comparing numerical setups, but the high-Re cases should not be presented as fully converged final benchmark results.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.
