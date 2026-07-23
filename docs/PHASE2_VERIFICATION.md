# Phase 2: Verification and Convergence Contract

This phase adds a verification layer around the standalone C++ lid-driven-cavity project before the existing production solver is numerically changed.

## Why this is needed

The original 36-case study completed its configured executions, but those runs reached their maximum outer-iteration limits. Execution completion, benchmark-profile agreement, and iterative convergence are different results and must remain separate.

## Convergence contract

A future cavity run may report `converged` only when all of the following hold:

- the velocity-update Linf residual is below its dimensionless tolerance;
- the local divergence Linf residual is below its dimensionless tolerance;
- the local divergence L2 residual is below its dimensionless tolerance;
- the integrated global mass imbalance is below its tolerance;
- the pressure equation converged for the current outer iteration;
- all conditions remain satisfied for a configured number of consecutive iterations;
- all fields and metrics are finite.

The explicit solver statuses are:

- `running`
- `converged`
- `max_iterations`
- `pressure_not_converged`
- `stagnated`
- `diverged`
- `non_finite`

The `max_iterations` status must never be interpreted as convergence.

## Verification tests added

### Operator compatibility

The verification library uses a forward pressure gradient and backward divergence pair. In the interior, their composition is checked against the standard five-point Laplacian:

```text
D(G(phi)) = L(phi)
```

The test also checks constant-field gradients, zero-field divergence, and non-finite-value detection.

### Manufactured Poisson solution

The Poisson verification problem uses

```text
phi(x,y) = sin(pi x) sin(pi y)
```

with homogeneous Dirichlet boundaries and

```text
laplacian(phi) = -2 pi^2 sin(pi x) sin(pi y).
```

The tests verify:

- convergence on 17x17, 33x33, and 65x65 grids;
- approximately second-order spatial convergence;
- agreement between RBGS and RBSOR solutions;
- true equation-residual reduction.

### Convergence-state logic

The convergence tracker is tested independently for:

- minimum-iteration protection;
- consecutive-pass requirements;
- repeated pressure-solver failures;
- non-finite residuals;
- maximum-iteration termination.

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

## Next integration step

The verification library intentionally does not silently change the existing full-study results. The next step is to integrate the convergence tracker and dimensionless residual definitions into `src/lid_cavity.cpp`, then tune one canonical case (`N=32`, `Re=100`, upwind, RBSOR) before regenerating the 36-case study.
