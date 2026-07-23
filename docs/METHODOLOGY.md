# Numerical Methodology

## Problem definition

The solver models the classical two-dimensional incompressible lid-driven cavity in a unit square. The top wall moves with nondimensional velocity `U = 1`; the other walls are stationary. The Reynolds number controls the kinematic viscosity:

```text
nu = U * L / Re
```

with `U = 1` and `L = 1`.

## Staggered-grid arrangement

The production Phase 2 solver uses a Marker-and-Cell staggered grid:

- pressure is stored at cell centers;
- horizontal velocity is stored on vertical cell faces;
- vertical velocity is stored on horizontal cell faces.

This arrangement avoids the pressure checkerboarding problem of the earlier collocated prototype and makes the pressure gradient, velocity correction, and discrete divergence naturally compatible.

## Projection workflow

Each outer pseudo-time iteration performs:

1. calculate a stable pseudo-time step from convection and diffusion limits;
2. predict face velocities from convection, diffusion, and the current pressure field;
3. calculate cell-centered divergence of the predicted velocity;
4. solve the pressure-correction Poisson equation;
5. correct face velocities with pressure-correction gradients;
6. update and normalize pressure;
7. calculate velocity-update, divergence, mass-balance, and pressure metrics;
8. update the convergence state.

The discrete projection is arranged so that the divergence and pressure-gradient operators compose into the same Laplacian used in the Poisson solve.

## Momentum discretization

The code supports:

- first-order upwind convection;
- second-order central convection;
- second-order central diffusion.

Tangential no-slip wall conditions are imposed through ghost values. Normal wall velocities are fixed directly on the staggered boundary faces.

## Pressure Poisson equation

The pressure-correction equation is solved with:

- red-black Gauss-Seidel (`RBGS`);
- red-black successive over-relaxation (`RBSOR`).

Homogeneous normal pressure-gradient conditions are represented by the boundary stencil. The right-hand side is projected to zero mean, and the pressure field is normalized to remove the constant null space.

Pressure convergence is measured with a true equation residual. An outer case cannot report `converged` while the pressure solve is failing.

## Convergence definition

The solver records separate dimensionless quantities:

- velocity-update `Linf` residual;
- divergence `Linf` residual;
- divergence `L2` residual;
- global boundary mass imbalance;
- pressure-Poisson relative residual.

A case reports `converged` only after all configured criteria pass for a required number of consecutive outer iterations and after a minimum iteration count.

Terminal states are:

```text
converged
max_iterations
pressure_not_converged
stagnated
diverged
non_finite
```

## Continuation

Parameter-study modes reuse a converged solution at a lower Reynolds number as the initial state for the next Reynolds number when the grid, convection scheme, and pressure solver are unchanged. This improves the stability and efficiency of the `Re=400` and `Re=1000` cases.

## Verification

The repository includes three levels of numerical checking:

### Operator verification

Analytical fields test the gradient, divergence, and Laplacian operators and their discrete compatibility.

### Poisson verification

A manufactured Poisson problem is solved on successively refined grids. The tests check error reduction and agreement between RBGS and RBSOR.

### Production regression

CTest runs the canonical cavity case:

```text
N = 32
Re = 100
scheme = upwind
pressure solver = RBSOR
```

The regression fails unless the executable reports convergence.

## Ghia comparison

After each supported Reynolds-number case, the cell-centered solution is interpolated onto the vertical and horizontal centerlines and compared with Ghia et al. The output includes `L2` and `Linf` errors for `u(y)` and `v(x)`.

This is a reference benchmark comparison. It is not experimental validation.
