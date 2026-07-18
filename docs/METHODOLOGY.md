# Methodology

This document explains the numerical workflow used by the completed C++ lid-driven cavity solver.

## Problem definition

The solver models the classical two-dimensional square lid-driven cavity. The domain is a unit square. The top wall moves with a nondimensional horizontal velocity of `U_lid = 1`, while the left, right, and bottom walls are stationary. No-slip velocity boundary conditions are applied on all walls.

The Reynolds number is controlled through the kinematic viscosity:

```text
nu = U_lid * L / Re
```

with `L = 1` and `U_lid = 1`.

## Numerical model

The code solves the incompressible Navier-Stokes equations in nondimensional form using a pseudo-transient pressure-correction workflow:

1. initialize velocity and pressure
2. apply lid and wall boundary conditions
3. predict the velocity field
4. solve the pressure-correction Poisson equation
5. correct velocity and pressure
6. record residuals and validation metrics
7. repeat until the iteration limit or stopping criteria are reached

## Spatial discretization

The domain is discretized on a structured Cartesian grid. The C++ version uses a collocated storage layout and manual indexing through a flat `std::vector<double>` container.

The study supports two convection schemes:

- `upwind`: more dissipative but more stable
- `central`: less dissipative and generally more accurate for the benchmark profiles

Diffusion terms are approximated with standard second-order finite differences.

## Pressure correction

The pressure-correction equation is solved iteratively using:

- `RBGS`: red-black Gauss-Seidel
- `RBSOR`: red-black Successive Over-Relaxation

The recorded study shows that RBSOR substantially reduces the average number of Poisson iterations compared with RBGS.

## Time-step logic

The solver uses a pseudo-time step based on convective and diffusive restrictions. This keeps the update conservative enough for the tested Reynolds numbers while allowing the same setup to run across several meshes.

## Validation

After each case, the code compares the computed centerline velocity profiles with the benchmark data from Ghia et al. The reported values are practical `L2` and `Linf` errors for comparing cases. They do not replace a formal verification or uncertainty study.

## Implementation notes

The code is a completed serial C++17 baseline focused on clarity and reproducibility rather than maximum performance. Accelerated implementations are developed separately in the broader work-in-progress solver-comparison repository so that numerical behavior, runtime, and scalability can be compared transparently.
