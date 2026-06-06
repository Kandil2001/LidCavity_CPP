# Lid-Driven Cavity Flow Solver in C++

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Python-post--processing-green.svg" alt="Python post-processing">
  <img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="MIT License">
  <a href="https://kandil2001.github.io/">
    <img src="https://img.shields.io/badge/Portfolio-kandil2001.github.io-2ea44f.svg" alt="Portfolio">
  </a>
</p>

A serial C++ implementation of the classical two-dimensional lid-driven cavity benchmark. This project is the C++ continuation of my MATLAB SIMPLE-based solver, using the same physical setup and parameter-study logic while exporting the results as CSV files for Python post-processing.

I built this version to turn the MATLAB learning code into a cleaner terminal-based CFD/HPC portfolio project. The current solver is intentionally a **serial C++ baseline**. It is not presented as a fake vectorized C++ solver. The goal is to have a reliable reference implementation before adding OpenMP, MPI, or CUDA versions later.

This repository is part of my wider lid-cavity implementation comparison, where the same benchmark is gradually implemented in MATLAB, C++, Python, OpenMP, MPI, CUDA, and OpenFOAM.

<p align="center">
  <img src="assets/figures/readme_overview.svg" alt="Overview of the C++ lid-driven cavity results" width="900">
</p>

## What is included

- Serial C++17 lid-driven cavity solver
- Structured collocated Cartesian grid
- Pseudo-transient pressure-correction workflow
- First-order upwind and central convection schemes
- Red-black Gauss-Seidel and red-black SOR pressure solvers
- Ghia et al. centreline velocity validation
- CSV export for fields, residual histories, and study summaries
- Python post-processing for contours, streamlines, residuals, validation plots, and study-level comparisons
- Bash scripts for smoke, single, quick, medium, and full studies

## Relation to the MATLAB version

The MATLAB project contains two MATLAB implementations: a loop-based version and a vectorized momentum-predictor version. This C++ repository currently contains one implementation:

```text
serial_cpp
```

The C++ solver follows the same benchmark idea, boundary conditions, Reynolds-number set, mesh set, convection schemes, pressure solvers, validation data, and study modes. The outputs are expected to match the MATLAB solution trend within numerical tolerance, but not bit-by-bit, because MATLAB and C++ use different operation ordering and update paths.

The full C++ study runs:

```text
3 meshes × 3 Reynolds numbers × 2 schemes × 2 pressure solvers × 1 implementation
= 36 simulations
```

The MATLAB full study runs 72 cases because it has two MATLAB implementations.

## Representative result

The case below uses `N = 128`, `Re = 1000`, central differencing, RBSOR pressure correction, and the serial C++ implementation.

| Flow field | Centreline validation |
|---|---|
| ![Streamlines](assets/figures/re1000_streamlines.svg) | ![Ghia u validation](assets/figures/re1000_ghia_u.svg) |
| ![Velocity magnitude](assets/figures/re1000_speed.svg) | ![Ghia v validation](assets/figures/re1000_ghia_v.svg) |

## Numerical approach

The solver advances the non-dimensional incompressible Navier-Stokes equations in pseudo-time. Each outer iteration predicts the velocity field, solves a pressure-correction Poisson equation, corrects velocity and pressure, applies boundary conditions, and records convergence diagnostics.

A detailed explanation is available in [docs/METHODOLOGY.md](docs/METHODOLOGY.md).

## Study observations

The compact full-study summary is included in:

```text
results/data/study_summary_full.csv
```

From the current run:

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

The Python script reads the C++ CSV files and generates MATLAB-style post-processing figures, including contours, streamlines, quiver plots, residual histories, Ghia validation curves, and study comparison plots.

## Repository layout

```text
src/           C++ solver
scripts/       build, run, plot, and clean scripts
postprocess/   Python plotting and result-summary tools
validation/    implemented inside the C++ solver through Ghia benchmark tables
assets/        selected figures for the README and documentation
docs/          methodology, running, validation, and result notes
results/       compact summary only; full generated output is ignored by Git
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

On Windows, I recommend WSL because the scripts are written for a Linux-style terminal.

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
