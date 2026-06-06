# Project scope

This repository is a standalone C++ CFD project for the two-dimensional lid-driven cavity benchmark.

## Purpose

The aim is to build a clear serial C++ baseline that can be used for:

- studying the pressure-correction workflow for incompressible flow,
- comparing numerical options such as upwind and central differencing,
- comparing pressure solvers such as RBGS and RBSOR,
- exporting reproducible CSV results,
- generating validation and post-processing figures with Python,
- extending the same codebase later with OpenMP, MPI, or CUDA.

## Current implementation

The current implementation is:

```text
serial_cpp
```

It is intentionally not described as vectorized, OpenMP, MPI, or GPU accelerated. Those should be added as separate implementations later so that runtime and accuracy comparisons stay honest.

## What this project is not

This is not a production CFD package. It is also not meant to replace OpenFOAM, ANSYS Fluent, STAR-CCM+, or other industrial CFD tools.

The project is an educational and portfolio-quality solver that prioritizes transparency, reproducibility, and a clean path toward future HPC versions.

## Recommended next development steps

1. Improve convergence behaviour at higher Reynolds numbers.
2. Split common solver utilities into smaller source files.
3. Add OpenMP parallel loops as the first accelerated version.
4. Add benchmark tables comparing serial and OpenMP performance.
5. Add MPI or CUDA only after the serial and OpenMP versions are stable.
