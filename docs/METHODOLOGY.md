# Methodology

This document explains the numerical workflow used by the C++ lid-driven cavity solver.

## Problem definition

The solver models the classical two-dimensional square lid-driven cavity. The domain is a unit square. The top wall moves with a non-dimensional horizontal velocity of `U_lid = 1`, while the left, right, and bottom walls are stationary. No-slip velocity boundary conditions are applied on all walls.

The Reynolds number is controlled through the kinematic viscosity:

```text
nu = U_lid * L / Re
```

with `L = 1` and `U_lid = 1`.

## Numerical model

The code solves the incompressible Navier-Stokes equations in non-dimensional form using a pseudo-transient pressure-correction workflow. The main goal is not to build a production CFD solver, but to expose the important parts of the algorithm clearly:

1. initialize velocity and pressure,
2. apply lid and wall boundary conditions,
3. predict the velocity field,
4. solve the pressure-correction Poisson equation,
5. correct velocity and pressure,
6. record residuals and validation metrics,
7. repeat until the iteration limit or stopping criteria are reached.

## Spatial discretization

The domain is discretized on a structured Cartesian grid. The current C++ version uses a collocated storage layout and manual indexing through a flat `std::vector<double>` container.

The study supports two convection schemes:

- `upwind`: more dissipative but more stable,
- `central`: less dissipative and usually more accurate for the benchmark profiles.

Diffusion terms are approximated with standard second-order finite differences.

## Pressure correction

The pressure-correction equation is solved iteratively using one of two red-black methods:

- `RBGS`: red-black Gauss-Seidel,
- `RBSOR`: red-black successive over-relaxation.

The current full-study output shows that RBSOR significantly reduces the average number of Poisson iterations compared with RBGS.

## Time-step logic

The solver uses a pseudo-time-step based on convective and diffusive restrictions. This keeps the update conservative enough for the tested Reynolds numbers while allowing the same setup to run across several meshes.

## Validation

After each case, the code compares the computed centreline velocity profiles against the benchmark data from Ghia et al. The reported values are practical L2 and Linf errors for comparison between cases. They are not meant to replace a formal verification study.

## Notes on equivalence with MATLAB

The C++ code follows the same benchmark and study setup as the MATLAB solver, but it is not a line-by-line translation. MATLAB vectorized array operations and C++ loop updates do not produce bitwise-identical floating-point histories. The expected comparison is therefore numerical agreement within tolerance, not exact matching of every iteration value.
