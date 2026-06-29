# Lid-Driven Cavity Flow Solver in C++

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Python-post--processing-green.svg" alt="Python post-processing">
  <img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="MIT License">
  <a href="https://kandil2001.github.io/">
    <img src="https://img.shields.io/badge/Portfolio-kandil2001.github.io-2ea44f.svg" alt="Portfolio">
  </a>
</p>

This is my C++17 implementation of the 2D lid-driven cavity flow problem.

I made it as the serial C++ version of a larger CFD comparison project. The idea is simple: solve the same benchmark problem in different languages and implementations, then compare the results, runtime, and later the parallel speedup.

This repository is not meant to be a perfect CFD package. It is a clean baseline that I can build on before moving to OpenMP, MPI, CUDA, and OpenFOAM versions.

## What the code does

The solver runs the incompressible lid-driven cavity problem on a structured Cartesian grid. The top wall moves, the other walls are fixed, and the flow develops the usual cavity recirculation.

Implemented features:

- serial C++17 solver,
- collocated Cartesian grid,
- pseudo-transient pressure-correction method,
- upwind and central convection schemes,
- red-black Gauss-Seidel pressure solver,
- red-black SOR pressure solver,
- CSV output for fields, residuals, and study summaries,
- Python scripts for plotting fields, residuals, validation, and runtime summaries.

The full study runs 36 cases:

```text
3 meshes x 3 Reynolds numbers x 2 schemes x 2 pressure solvers
```

## Example result

The case shown below is:

```text
N = 128
Re = 1000
scheme = central
pressure solver = RBSOR
implementation = serial_cpp
```

I use this case in the README because the main recirculation region is clear.

| Streamlines | Velocity magnitude |
|---|---|
| ![Streamlines](assets/figures/re1000_streamlines.svg) | ![Velocity magnitude](assets/figures/re1000_speed.svg) |

## Validation

The validation is done against the classical Ghia et al. lid-driven cavity data.

I compare:

- `u(y)` on the vertical centreline `x = 0.5`,
- `v(x)` on the horizontal centreline `y = 0.5`.

For the README, the validation plots are shown side by side.

| Ghia u validation | Ghia v validation |
|---|---|
| ![Ghia u validation](assets/figures/re1000_ghia_u.svg) | ![Ghia v validation](assets/figures/re1000_ghia_v.svg) |

For each case, the code reports `L2` and `Linf` errors against the benchmark points.

The best refined-grid central + RBSOR cases are:

| Re | Case | N | Scheme | Pressure solver | Ghia `u` L2 | Ghia `v` L2 | Runtime [s] |
|---:|---:|---:|---|---|---:|---:|---:|
| 100 | 28 | 128 | central | RBSOR | 0.0031 | 0.0041 | 441.7 |
| 400 | 32 | 128 | central | RBSOR | 0.0539 | 0.0652 | 527.6 |
| 1000 | 36 | 128 | central | RBSOR | 0.1102 | 0.1109 | 647.6 |

From the full study:

- 36 cases were run,
- 22 cases passed the selected Ghia error limits,
- all 12 cases with `N = 128` passed,
- central differencing worked best on the refined grid,
- RBSOR gave nearly the same validation error as RBGS, but with much lower pressure-solver cost.

![Ghia error summary](assets/figures/study_ghia_error.svg)

![Pressure solver comparison](assets/figures/study_pressure_solver_iterations.svg)

The validation limits are practical limits for comparing the study cases. I do not treat them as a full verification study.

## How it works

The solver advances the non-dimensional incompressible Navier-Stokes equations in pseudo-time.

Each outer iteration does roughly this:

1. predict velocity,
2. solve the pressure-correction equation,
3. correct velocity and pressure,
4. apply the wall boundary conditions again,
5. save residual information.

More details are in [`docs/METHODOLOGY.md`](docs/METHODOLOGY.md).

## Running

On Linux, WSL, or a cluster:

```bash
bash scripts/run_smoke_test.sh   # compile and run a tiny check
bash scripts/run_single.sh       # N=128, Re=1000 example case
bash scripts/run_quick.sh        # smaller study
bash scripts/run_medium.sh       # medium study
bash scripts/run_full.sh         # full 36-case study
```

After running cases, generate the plots with:

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

For the solver:

```bash
g++ with C++17 support
```

For plotting:

```bash
python3 -m pip install -r requirements.txt
```

WSL is recommended on Windows because the scripts are written for a Linux-style terminal.

## Notes and limitations

This is still an educational CFD solver, so there are some things I would not overclaim:

- the grid is collocated,
- there is no Rhie-Chow interpolation yet,
- the pressure solver does not use multigrid,
- all full-study cases reached the configured maximum outer-iteration limit,
- the high-Re cases still need better convergence tuning.

So the current results are useful for comparing setups and building the benchmark series, but the solver still needs numerical improvement.

## Next steps

- improve convergence control,
- improve the validation plots for all Reynolds numbers,
- add clearer mesh-convergence plots,
- split the C++ code into smaller files,
- add OpenMP and compare with this serial version,
- add MPI and CUDA versions later,
- compare this with the Python, C, MATLAB, and OpenFOAM versions.

## Reference

Ghia, U., Ghia, K. N., & Shin, C. T. (1982). *High-Re solutions for incompressible flow using the Navier-Stokes equations and a multigrid method*. Journal of Computational Physics, 48(3), 387-411.

## Author

Ahmed Kandil - [Portfolio](https://kandil2001.github.io/) - [LinkedIn](https://www.linkedin.com/in/ahmed-kandil03/)

Released under the [MIT License](LICENSE).
