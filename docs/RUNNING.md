# Running the C++ solver

The project is intended to be run from a Linux-style terminal. On Windows, WSL is recommended.

## Build only

```bash
bash scripts/build.sh
```

This creates the executable:

```text
bin/lid_cavity
```

## Smoke test

Run this first after cloning the repository:

```bash
bash scripts/run_smoke_test.sh
```

The smoke test uses a very small setup. It is only meant to check that the code compiles and writes output files.

## Representative case

```bash
bash scripts/run_single.sh
```

This runs:

```text
N = 64
Re = 100
scheme = central
pressure solver = RBGS
implementation = serial_cpp
```

## Quick study

```bash
bash scripts/run_quick.sh
```

The quick study is useful after editing the solver because it runs a reduced set of meshes and Reynolds numbers.

## Medium study

```bash
bash scripts/run_medium.sh
```

The medium study includes more Reynolds-number coverage but avoids the largest mesh.

## Full study

```bash
bash scripts/run_full.sh
```

The full C++ study runs 36 cases:

```text
3 meshes × 3 Reynolds numbers × 2 schemes × 2 pressure solvers × 1 serial C++ implementation
```

This can take several hours depending on the CPU.

## Plotting

After running one or more cases:

```bash
bash scripts/plot_results.sh
```

The plotting script reads the CSV files in `results/data/` and writes PNG figures to `results/figures/`.

## Cleaning generated files

```bash
bash scripts/clean.sh
```

This removes the compiled binary and generated CSV files.

## Output folders

```text
results/data/       CSV outputs
results/figures/    generated PNG figures
```

The repository keeps only the compact full-study summary and selected README figures. Full generated case files are ignored by Git because they can become large.
