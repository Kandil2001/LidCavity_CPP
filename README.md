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

This is a standalone C++ CFD project for the classical **2D lid-driven cavity** benchmark. I built it to have a clean serial C++ solver that is easy to run, easy to inspect, and useful as a baseline before adding faster versions later.

The project solves the incompressible cavity flow problem on a structured grid, exports the results as CSV files, and uses Python for post-processing and validation plots.

<p align="center">
  <img src="assets/figures/refined_grid_validation_summary.svg" alt="Refined-grid validation summary" width="900">
</p>

## Why this project

The lid-driven cavity is a simple geometry, but it is a very useful CFD benchmark. It shows the main ideas behind incompressible flow solvers: wall boundary conditions, vortex formation, pressure correction, residual tracking, and validation against reference data.

I wanted this repository to be more than just a single source file. The goal was to make it look and feel like a small engineering project:

- a clear C++ solver,
- repeatable run scripts,
- structured CSV output,
- Python post-processing,
- validation against Ghia et al.,
- selected figures and result summaries for GitHub.

The current version is intentionally a **serial C++ baseline**. It is not OpenMP, MPI, CUDA, or vectorized C++ yet. Those versions can be added later and compared against this one.

## What the solver does

- Solves the 2D incompressible lid-driven cavity problem
- Uses a unit square cavity with a moving top lid
- Runs Reynolds numbers `100`, `400`, and `1000`
- Supports grid sizes `32`, `64`, and `128`
- Includes `upwind` and `central` convection schemes
- Includes `RBGS` and `RBSOR` pressure-correction solvers
- Exports final fields and residual histories as CSV files
- Compares centreline velocities with the Ghia et al. benchmark
- Generates contours, streamlines, residual plots, and validation plots using Python

## Representative results

The strongest results to show are the refined-grid central/RBSOR cases. They combine the best validation behaviour with the faster pressure solver.

| Re | Best case | N | Scheme | Pressure solver | Ghia `u` L2 | Ghia `v` L2 |
|---:|---:|---:|---|---|---:|---:|
| 100 | 28 | 128 | central | RBSOR | 0.0031 | 0.0041 |
| 400 | 32 | 128 | central | RBSOR | 0.0539 | 0.0652 |
| 1000 | 36 | 128 | central | RBSOR | 0.1102 | 0.1109 |

The high-Reynolds-number case below is still useful visually because it shows the stronger recirculation structure at `Re = 1000`:

| Flow field | Centreline validation |
|---|---|
| ![Streamlines](assets/figures/re1000_streamlines.svg) | ![Ghia u validation](assets/figures/re1000_ghia_u.svg) |
| ![Velocity magnitude](assets/figures/re1000_speed.svg) | ![Ghia v validation](assets/figures/re1000_ghia_v.svg) |

## Numerical method

The solver uses a pseudo-transient pressure-correction workflow. In each outer iteration, the code:

1. applies the lid and wall boundary conditions,
2. predicts the velocity field,
3. solves the pressure-correction Poisson equation,
4. corrects velocity and pressure,
5. records residuals and validation metrics.

The full method is explained in [docs/METHODOLOGY.md](docs/METHODOLOGY.md).

## Study setup

The full study contains 36 simulations:

```text
3 meshes × 3 Reynolds numbers × 2 convection schemes × 2 pressure solvers × 1 serial implementation
= 36 simulations
```

The tested setup is:

```text
meshes:             32, 64, 128
Reynolds numbers:   100, 400, 1000
schemes:            upwind, central
pressure solvers:   RBGS, RBSOR
implementation:     serial_cpp
```

The compact full-study summary is included here:

```text
results/data/study_summary_full.csv
```

## Result highlights

The refined-grid cases give the clearest validation story. All `N = 128` cases passed the selected Ghia centreline validation thresholds, including the `Re = 1000` cases.

From the current full study:

- `36` simulations were executed.
- `22/36` cases passed the selected Ghia validation threshold.
- `12/12` cases on the `N = 128` grid passed the validation threshold.
- The best agreement was obtained with the `N = 128`, `Re = 100`, central-difference cases.
- At `Re = 400` and `Re = 1000`, the refined-grid central cases still captured the expected cavity-flow behaviour and passed the selected validation threshold.
- RBSOR gave almost the same validation error as RBGS while being much faster.

| Pressure solver | Cases | Mean runtime [s] | Total runtime [s] | Mean Poisson iterations |
|---|---:|---:|---:|---:|
| RBGS | 18 | 790.2 | 14224.3 | 1333.1 |
| RBSOR | 18 | 176.6 | 3178.9 | 264.5 |

In this study, RBSOR was about **4.5× faster** overall and used about **5× fewer pressure iterations** on average than RBGS.

The full study took approximately **4.83 hours** on the machine where the uploaded results were generated.

More details are available in [docs/RESULTS.md](docs/RESULTS.md).

![Pressure solver comparison](assets/figures/study_pressure_solver_iterations.svg)

## How to run

On Linux, WSL, or a university cluster:

```bash
bash scripts/run_smoke_test.sh   # small compilation/output check
bash scripts/run_single.sh       # representative case
bash scripts/run_quick.sh        # reduced study
bash scripts/run_medium.sh       # medium study
bash scripts/run_full.sh         # full 36-case study
```

Generated CSV files are written to:

```text
results/data/
```

Generated figures are written to:

```text
results/figures/
```

More detailed instructions are in [docs/RUNNING.md](docs/RUNNING.md).

## Plot the results

After running the solver:

```bash
bash scripts/plot_results.sh
```

The Python script reads the C++ CSV files and generates post-processing figures, including velocity contours, pressure contours, streamlines, vorticity contours, residual histories, Ghia validation curves, and study comparison plots.

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

This is an educational and portfolio CFD solver, not a replacement for a production CFD package. The current version is serial only and uses a simple pressure-correction approach. The refined-grid results are promising, but the code can still be improved with better convergence control, acceleration, and cleaner solver modularization.

## Planned extensions

1. Improve convergence control and stopping criteria.
2. Split the solver into smaller C++ modules.
3. Add an OpenMP implementation and compare it with `serial_cpp`.
4. Add benchmark tables for accuracy, runtime, and speedup.
5. Add MPI or CUDA versions later.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.

## Author

Ahmed Kandil — [Portfolio](https://kandil2001.github.io/) · [LinkedIn](https://www.linkedin.com/in/ahmed-kandil03/)

Released under the [MIT License](LICENSE).
