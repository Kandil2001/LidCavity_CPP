# Lid-Driven Cavity Flow Solver in C++

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Python-post--processing-green.svg" alt="Python post-processing">
  <img src="https://img.shields.io/badge/CFD-lid--driven--cavity-orange.svg" alt="CFD benchmark">
  <img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="MIT License">
  <a href="https://kandil2001.github.io/">
    <img src="https://img.shields.io/badge/Portfolio-kandil2001.github.io-2ea44f.svg" alt="Portfolio">
  </a>
</p>

A standalone C++17 implementation of the classical two-dimensional lid-driven cavity benchmark for incompressible flow. The project solves the cavity problem using a structured Cartesian grid, a pressure-correction workflow, and a small parameter study covering mesh size, Reynolds number, convection scheme, and pressure-solver choice.

The goal of this repository is to keep the numerical method readable while still making the project useful as a CFD/HPC portfolio baseline. The current implementation is intentionally a **serial C++ solver**. It is the reference version that future OpenMP, MPI, CUDA, or other accelerated implementations can be compared against.

<p align="center">
  <img src="assets/figures/readme_overview.svg" alt="Overview of the C++ lid-driven cavity results" width="900">
</p>

## What is included

- Serial C++17 lid-driven cavity solver
- Structured collocated Cartesian grid
- Pseudo-transient pressure-correction workflow
- First-order upwind and central-difference convection options
- Red-black Gauss-Seidel and red-black SOR pressure-correction solvers
- Validation against the Ghia et al. centreline velocity benchmark
- CSV export for final fields, residual histories, and study summaries
- Python post-processing for contours, streamlines, residuals, validation plots, and study comparisons
- Bash scripts for smoke, single, quick, medium, and full studies
- GitHub Actions smoke test

## Representative result

The case below uses `N = 128`, `Re = 1000`, central differencing, RBSOR pressure correction, and the serial C++ implementation.

| Flow field | Centreline validation |
|---|---|
| ![Streamlines](assets/figures/re1000_streamlines.svg) | ![Ghia u validation](assets/figures/re1000_ghia_u.svg) |
| ![Velocity magnitude](assets/figures/re1000_speed.svg) | ![Ghia v validation](assets/figures/re1000_ghia_v.svg) |

## Numerical approach

The solver advances the non-dimensional incompressible Navier-Stokes equations in pseudo-time. Each outer iteration predicts the velocity field, solves a pressure-correction Poisson equation, corrects velocity and pressure, reapplies boundary conditions, and records convergence diagnostics.

A detailed explanation is available in [docs/METHODOLOGY.md](docs/METHODOLOGY.md).

## Study design

The full study runs:

```text
3 meshes × 3 Reynolds numbers × 2 convection schemes × 2 pressure solvers × 1 serial implementation
= 36 simulations
```

The cases cover:

```text
meshes:             32, 64, 128
Reynolds numbers:   100, 400, 1000
schemes:            upwind, central
pressure solvers:   RBGS, RBSOR
implementation:     serial_cpp
```

The compact full-study summary is stored in:

```text
results/data/study_summary_full.csv
```

## Current results

From the current full-study run:

- `36` cases were executed.
- `22/36` cases passed the selected Ghia centreline-error threshold used in the code.
- Every case reached the configured maximum outer-iteration limit, so the results are useful for comparison but are not described as fully converged.
- The `Re = 100` cases showed the best agreement with the benchmark.
- RBSOR strongly reduced pressure-solver iterations and runtime compared with RBGS.

| Pressure solver | Cases | Mean runtime [s] | Total runtime [s] | Mean Poisson iterations |
|---|---:|---:|---:|---:|
| RBGS | 18 | 790.2 | 14224.3 | 1333.1 |
| RBSOR | 18 | 176.6 | 3178.9 | 264.5 |

The full study took approximately **4.83 hours** on the machine where the uploaded results were generated.

More detailed notes are available in [docs/RESULTS.md](docs/RESULTS.md).

![Pressure solver comparison](assets/figures/study_pressure_solver_iterations.svg)

## Run the project

On Linux, WSL, or the university cluster:

```bash
bash scripts/run_smoke_test.sh   # small compilation/output check
bash scripts/run_single.sh       # representative case
bash scripts/run_quick.sh        # reduced study
bash scripts/run_medium.sh       # medium study
bash scripts/run_full.sh         # full 36-case study
```

Generated CSV files are written to `results/data/`. Generated figures are written to `results/figures/`.

Detailed running instructions are in [docs/RUNNING.md](docs/RUNNING.md).

## Plot the results

After running the solver:

```bash
bash scripts/plot_results.sh
```

The Python script reads the C++ CSV files and generates post-processing figures, including contours, streamlines, quiver plots, residual histories, Ghia validation curves, and study comparison plots.

## Repository layout

```text
src/           C++ solver
scripts/       build, run, plot, and clean scripts
postprocess/   Python plotting and result-summary tools
assets/        selected figures for the README and documentation
docs/          methodology, running, validation, scope, and result notes
results/       compact summary only; full generated output is ignored by Git
.github/       GitHub Actions workflow
```

## Requirements

For the C++ solver:

```bash
g++ with C++17 support
```

For the Python post-processing:

```bash
python3 -m pip install -r requirements.txt
```

On Windows, WSL is recommended because the scripts are written for a Linux-style terminal.

## Output files

Each case can generate two CSV files:

```text
case_XXX_..._fields.csv
case_XXX_..._history.csv
```

The field file contains:

```text
i,j,x,y,u,v,p,speed,vorticity
```

The history file contains:

```text
iter,Ru,Rv,Rc_mass,Rc_div,dt,poisson_iters,poisson_relative_residual,poisson_converged
```

The study summary contains the case setup, runtime, residuals, pressure-solver statistics, and Ghia validation errors.

## Limitations

This is an educational and portfolio CFD solver, not a replacement for a production CFD package. The current version is serial only, uses a simple pressure-correction approach, and does not include multigrid acceleration or Rhie-Chow interpolation. The high-Reynolds-number cases are useful for comparison but still need more convergence tuning.

## Planned extensions

1. Improve convergence behaviour for `Re = 400` and `Re = 1000`.
2. Add an OpenMP implementation and compare speedup against `serial_cpp`.
3. Add a cleaner benchmark table for accuracy and runtime.
4. Add MPI and CUDA versions as separate implementations later.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.

## Author

Ahmed Kandil — [Portfolio](https://kandil2001.github.io/) · [LinkedIn](https://www.linkedin.com/in/ahmed-kandil03/)

Released under the [MIT License](LICENSE).
