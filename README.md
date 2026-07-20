# Lid-Driven Cavity Flow Solver in C++

<p align="center">
  <img src="https://img.shields.io/badge/Status-Completed-brightgreen.svg" alt="Completed">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Build-GCC%208%2B-brightgreen.svg" alt="GCC 8 and newer">
  <img src="https://img.shields.io/badge/Python-post--processing-green.svg" alt="Python post-processing">
  <a href="https://github.com/Kandil2001/LidCavity_CPP/actions/workflows/ci.yml">
    <img src="https://github.com/Kandil2001/LidCavity_CPP/actions/workflows/ci.yml/badge.svg" alt="C++ build and smoke test">
  </a>
  <img src="https://img.shields.io/badge/License-MIT-lightgrey.svg" alt="MIT License">
  <a href="https://kandil2001.github.io/projects/lid-cavity-cpp.html">
    <img src="https://img.shields.io/badge/Portfolio-Case%20Study-2ea44f.svg" alt="Portfolio case study">
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
- red-black Gauss-Seidel and red-black SOR pressure solvers
- CSV output for fields, residuals, and study summaries
- Python scripts for plotting fields, residuals, validation, and runtime summaries
- GitHub Actions build, smoke execution, and output verification
- portable filesystem linking for modern compilers and GCC 8 HPC nodes

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

The solver advances the nondimensional incompressible Navier-Stokes equations in pseudo-time. Each outer iteration predicts velocity, solves the pressure-correction equation, corrects velocity and pressure, reapplies wall boundary conditions, and records residual information.

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

The build script compiles the source to an object file, links normally on current compilers, and retries with `-lstdc++fs` only when an older GCC toolchain requires it. The same command therefore works on modern Linux systems and on GCC 8-based cluster nodes.

Generate the plots with:

```bash
bash scripts/plot_results.sh
```

Outputs are written to:

```text
results/data/       CSV output files
results/figures/    generated plots
```

## Stromboli smoke test — 20 July 2026

The repository was compiled and executed successfully on the Stromboli HPC cluster with GCC 8.5.0 after adding the portable filesystem-link fallback.

The smoke configuration was intentionally tiny:

| Setting | Value |
|---|---:|
| Grid | `N = 16` |
| Reynolds number | `100` |
| Convection scheme | upwind |
| Pressure solver | RBGS |
| Outer iterations | `20` |
| Runtime | approximately `0.01 s` |

The smoke run reached the configured `maxIter` limit and did **not** meet the Ghia validation thresholds. That is expected for this deliberately short case. Its purpose is only to verify compilation, execution, argument handling, and CSV output—not numerical convergence.

The archived log and generated smoke-test data are stored in [`results/stromboli_2026-07-20`](results/stromboli_2026-07-20).

## Continuous integration

The GitHub Actions workflow runs the existing `scripts/run_smoke_test.sh` path, then verifies:

- the C++ executable was created
- the smoke-study summary contains exactly one `N = 16`, `Re = 100` case
- at least one convergence-history CSV was generated

This is a fast build-and-execution check. It does not claim that the full 36-case study is rerun or numerically validated on every commit.

## Repository structure

```text
src/                                   C++ solver
scripts/                               build, run, plot, and clean scripts
postprocess/                           Python plotting scripts
assets/                                selected README figures
docs/                                  methodology, running notes, validation, and results
results/data/                          full-study CSV output
results/figures/                       full-study generated plots
results/stromboli_2026-07-20/          archived HPC smoke test
.github/                               build-and-smoke GitHub Actions workflow
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

The natural next research step is to build a stricter verification and convergence protocol around these documented limitations.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.

## Author

Ahmed Kandil — [Portfolio](https://kandil2001.github.io/) · [LinkedIn](https://www.linkedin.com/in/ahmed-kandil03/) · [ORCID](https://orcid.org/0009-0007-2724-4565)

Released under the [MIT License](LICENSE).
