# Phase 2: Verification and Convergence

Phase 2 is now integrated into the standalone C++ lid-driven-cavity solver.

## Main production change

The earlier collocated pressure-correction implementation has been replaced in the production path by a staggered Marker-and-Cell solver. Pressure is stored at cell centers, horizontal velocity on vertical faces, and vertical velocity on horizontal faces.

This gives the solver a compatible pressure-gradient, velocity-correction, and divergence arrangement and removes the pressure-velocity compatibility problem that prevented the original study from reaching strict iterative convergence.

## Convergence contract

A cavity run reports `converged` only when all of the following hold:

- dimensionless velocity-update `Linf` residual is below tolerance;
- dimensionless divergence `Linf` residual is below tolerance;
- dimensionless divergence `L2` residual is below tolerance;
- global boundary mass imbalance is below tolerance;
- the pressure equation converged for the current outer iteration;
- all conditions remain satisfied for the configured number of consecutive iterations;
- all fields and metrics remain finite.

The explicit solver statuses are:

```text
converged
max_iterations
pressure_not_converged
stagnated
diverged
non_finite
```

`max_iterations` is never treated as convergence.

## Verification tests

### Operator compatibility

The verification library checks analytical gradient, divergence, and Laplacian fields, including the discrete compatibility relation:

```text
D(G(phi)) = L(phi)
```

### Manufactured Poisson solution

The independent Poisson verification uses:

```text
phi(x,y) = sin(pi x) sin(pi y)
laplacian(phi) = -2 pi^2 sin(pi x) sin(pi y)
```

The tests cover:

- `17x17`, `33x33`, and `65x65` grids;
- approximately second-order error reduction;
- RBGS/RBSOR agreement;
- true equation-residual reduction.

### Convergence-state logic

The convergence tracker is tested for:

- minimum-iteration protection;
- consecutive-pass requirements;
- pressure-solver failure handling;
- non-finite residual handling;
- maximum-iteration termination.

### Production regression

CTest now runs the actual production executable for:

```text
N = 32
Re = 100
scheme = upwind
pressure solver = RBSOR
```

The test uses `--strict` and fails unless the case converges.

## Verified run sets

The following sets were exercised during Phase 2 development:

- canonical `N=32`, `Re=100`, upwind, RBSOR;
- six `N=32` cases using `Re=100`, `400`, and `1000`, with upwind and central schemes;
- `N=16`, `32`, and `64` at `Re=100`, central, RBSOR.

All of these cases reached the configured iterative convergence criteria and passed their selected Ghia centerline thresholds.

## Running the checks

```bash
bash scripts/run_phase2_verification.sh
```

Or directly:

```bash
cmake -S . -B build/phase2-verification -DCMAKE_BUILD_TYPE=Release
cmake --build build/phase2-verification --parallel
ctest --test-dir build/phase2-verification --output-on-failure
```

## Running the solver

```bash
bash scripts/run_single.sh
bash scripts/run_medium.sh
bash scripts/run_grid.sh
```

The complete 36-case configuration remains available through:

```bash
bash scripts/run_full.sh
```

That run contains slower RBGS and `N=128` cases and is intended for a workstation or HPC node.
