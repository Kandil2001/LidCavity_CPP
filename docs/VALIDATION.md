# Validation

The solver is validated against the classical Ghia et al. lid-driven cavity benchmark data.

## What is compared

The code compares two centreline profiles:

- horizontal velocity `u` along the vertical centreline,
- vertical velocity `v` along the horizontal centreline.

For each case, the solver reports L2 and Linf errors against the benchmark values available for the tested Reynolds numbers.

## Current interpretation

The validation thresholds used in the code are practical comparison limits. They help classify which numerical setups are behaving reasonably, but they are not a full verification and validation study.

From the current uploaded full study:

- 22 out of 36 C++ cases passed the selected Ghia centreline-error threshold.
- The `Re = 100`, `N = 128`, central-difference cases showed the best agreement.
- The high-Reynolds-number cases show the expected flow structure, but still need convergence and parameter tuning.

## Why the C++ and MATLAB results are not exactly identical

The C++ solver follows the same physical and numerical setup as the MATLAB solver, but the floating-point update order is different. Therefore, the correct expectation is:

```text
MATLAB result ≈ C++ result
```

not:

```text
MATLAB result == C++ result bit by bit
```

Small differences in update order, residual history, and stopping behaviour are normal.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.
