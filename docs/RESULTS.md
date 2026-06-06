# Results

This page summarizes the uploaded full C++ serial study saved in:

```text
results/data/study_summary_full.csv
```

## Study size

The full C++ study contains 36 cases:

```text
3 meshes × 3 Reynolds numbers × 2 convection schemes × 2 pressure solvers × 1 C++ implementation
= 36 simulations
```

The implementation is intentionally labeled `serial_cpp`. It is not a vectorized, OpenMP, MPI, or CUDA solver yet.

## Main observations

- `22/36` cases passed the selected Ghia centreline validation threshold.
- All cases reached the configured maximum outer-iteration limit. The generated fields are useful for comparison, but the runs are not described as fully converged.
- The `Re = 100` cases gave the strongest agreement with the benchmark.
- Accuracy became harder at `Re = 400` and `Re = 1000`, which is expected for this simple educational pressure-correction solver.
- `RBSOR` was much faster than `RBGS` for the pressure-correction loop.

## Pressure solver comparison

| Pressure solver | Cases | Mean runtime [s] | Total runtime [s] | Mean Poisson iterations |
|---|---:|---:|---:|---:|
| RBGS | 18 | 790.2 | 14224.3 | 1333.1 |
| RBSOR | 18 | 176.6 | 3178.9 | 264.5 |


## Best validation case at each Reynolds number

| Re | Case | N | Scheme | Pressure solver | Ghia u L2 | Ghia v L2 | Runtime [s] |
|---:|---:|---:|---|---|---:|---:|---:|
| 100 | 27 | 128 | central | RBGS | 0.0031 | 0.0041 | 1934.2 |
| 400 | 32 | 128 | central | RBSOR | 0.0539 | 0.0652 | 527.6 |
| 1000 | 36 | 128 | central | RBSOR | 0.1102 | 0.1109 | 647.6 |


## Runtime

The full study runtime from the summary file is approximately **4.83 hours** on the machine where it was executed.

## Selected plots

![Ghia error summary](../assets/figures/study_ghia_error.svg)

![Pressure solver iterations](../assets/figures/study_pressure_solver_iterations.svg)

## Important interpretation

This is a learning and portfolio CFD code. The results show the correct cavity-flow structure and a working validation workflow, but the high-Reynolds-number cases still leave room for improvement.

Good next steps are:

1. tune convergence and under-relaxation for `Re = 400` and `Re = 1000`,
2. add an OpenMP implementation,
3. compare `serial_cpp` vs OpenMP runtime,
4. later add MPI or CUDA.
