# Lid-Driven Cavity Flow Solver in C++

<p align="center">
  <img src="https://img.shields.io/badge/Status-Completed-brightgreen.svg" alt="Completed">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Python-post--processing-green.svg" alt="Python post-processing">
  <img src="https://img.shields.io/badge/License-MIT-lightgrey.svg" alt="MIT License">
  <a href="https://kandil2001.github.io/">
    <img src="https://img.shields.io/badge/Portfolio-kandil2001.github.io-2ea44f.svg" alt="Portfolio">
  </a>
</p>

A completed C++17 implementation and parameter study of the two-dimensional lid-driven cavity benchmark.

This repository is the serial C++ component of a larger CFD comparison project. It solves the same benchmark used by the MATLAB and multi-language implementations so that numerical behavior, result quality, code structure, and runtime can be compared consistently.

The repository is an educational solver and benchmark study, not a production CFD package.

## What the code does

The solver runs the incompressible lid-driven cavity problem on a structured Cartesian grid. The top wall moves, the other walls are fixed, and the flow develops the characteristic cavity recirculation.

Implemented features:

- serial C++17 solver
- collocated Cartesian grid
- pseudo-transient pressure-correction method
- upwind and central convection schemes
- red-black Gauss-Seidel pressure solver
- red-black SOR pressure solver
- CSV output for fields, residuals, and study summaries
- Python scripts for plotting fields, residuals, validation, and runtime summaries

The completed study contains 36 configured cases:

```text
3 meshes × 3 Reynolds numbers × 2 schemes × 2 pressure solvers
```

## Representative result

The case shown below uses:

```text
N = 128
Re = 1000
scheme = central
pressure solver = RBSOR
implementation = serial_cpp
```

| Streamlines | Velocity magnitude |
|---|---|
| ![Streamlines](assets/figures/re1000_streamlines.svg) | ![Velocity magnitude](assets/figures/re1000_speed.svg) |

## Validation

The numerical profiles are compared with the classical Ghia et al. lid-driven cavity data:

- `u(y)` on the vertical centerline `x = 0.5`
- `v(x)` on the horizontal centerline `y = 0.5`

| Ghia u comparison | Ghia v comparison |
|---|---|
| ![Ghia u validation](assets/figures/re1000_ghia_u.svg) | ![Ghia v validation](assets/figures/re1000_ghia_v.svg) |

For each case, the code reports `L2` and `Linf` errors against the benchmark points.

The refined-grid central + RBSOR cases are:

| Re | Case | N | Scheme | Pressure solver | Ghia `u` L2 | Ghia `v` L2 | Runtime [s] |
|---:|---:|---:|---|---|---:|---:|---:|
| 100 | 28 | 128 | central | RBSOR | 0.0031 | 0.0041 | 441.7 |
| 400 | 32 | 128 | central | RBSOR | 0.0539 | 0.0652 | 527.6 |
| 1000 | 36 | 128 | central | RBSOR | 0.1102 | 0.1109 | 647.6 |

Study observations:

- all 36 configured cases executed
- 22 cases met the selected Ghia error thresholds
- all 12 cases with `N = 128` met those thresholds
- central differencing produced the best refined-grid agreement
- RBSOR produced similar validation errors to RBGS with lower pressure-solver cost

![Ghia error summary](assets/figures/study_ghia_error.svg)

![Pressure solver comparison](assets/figures/study_pressure_solver_iterations.svg)

The selected Ghia limits are comparison thresholds, not a formal verification or grid-independence study.

## Convergence interpretation

All full-study cases reached the configured maximum outer-iteration limit. Therefore:

- an executed case is not automatically a converged case
- the reported runtime is the cost of the configured run
- the Ghia error thresholds describe profile agreement, not residual convergence
- the high-Reynolds-number cases require additional convergence tuning

This distinction is important when comparing the results with other solver implementations.

## Numerical workflow

The solver advances the nondimensional incompressible Navier-Stokes equations in pseudo-time.

Each outer iteration:

1. predicts velocity
2. solves the pressure-correction equation
3. corrects velocity and pressure
4. reapplies wall boundary conditions
5. records residual information

More details are available in [`docs/METHODOLOGY.md`](docs/METHODOLOGY.md).

## Running the project

On Linux, WSL, or a cluster:

```bash
bash scripts/run_smoke_test.sh   # compile and run a tiny check
bash scripts/run_single.sh       # N=128, Re=1000 example
bash scripts/run_quick.sh        # reduced study
bash scripts/run_medium.sh       # medium study
bash scripts/run_full.sh         # complete 36-case configuration
```

Generate the plots with:

```bash
bash scripts/plot_results.sh
```

Outputs are written to:

```text
results/data/       CSV output files
results/figures/    generated plots
```

## Repository structure

```text
src/           C++ solver
scripts/       build, run, plot, and clean scripts
postprocess/   Python plotting scripts
assets/        selected figures used in the README
docs/          methodology, running notes, validation, and results
results/       generated outputs
.github/       smoke-test workflow
```

## Requirements

Solver:

```text
g++ with C++17 support
```

Post-processing:

```bash
python3 -m pip install -r requirements.txt
```

WSL is recommended on Windows because the scripts use a Linux-style terminal workflow.

## Scope and limitations

This completed project records the implemented solver and study as they were configured. Known numerical limitations include:

- collocated grid without Rhie-Chow interpolation
- no multigrid pressure solver
- configured maximum-iteration termination in the full study
- high-Reynolds-number cases that need stronger convergence control
- no formal grid-convergence or uncertainty study

The natural next research step is not to relabel the project as incomplete, but to build a new, stricter benchmark protocol around these documented limitations.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.

## Author

Ahmed Kandil — [Portfolio](https://kandil2001.github.io/) · [LinkedIn](https://www.linkedin.com/in/ahmed-kandil03/) · [ORCID](https://orcid.org/0009-0007-2724-4565)

Released under the [MIT License](LICENSE).
