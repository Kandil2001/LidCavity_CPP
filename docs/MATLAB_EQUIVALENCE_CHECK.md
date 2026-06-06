# MATLAB equivalence check

This C++ project was built to reproduce the same lid-driven cavity study idea as the MATLAB solver.

## Same benchmark setup

The C++ version keeps the same core benchmark setup:

- unit square lid-driven cavity,
- non-dimensional lid velocity `U_lid = 1`,
- Reynolds numbers `100`, `400`, and `1000`,
- meshes `32`, `64`, and `128` for the full study,
- upwind and central convection options,
- RBGS and RBSOR pressure solvers,
- Ghia centreline validation.

## Main difference

The MATLAB repository has two MATLAB implementation labels: loop-based and vectorized. The C++ repository currently has only one real implementation:

```text
serial_cpp
```

This is intentional. The current C++ code should not be called vectorized unless a real SIMD, Eigen/xtensor/Armadillo, OpenMP, or GPU version is added later.

## Expected result comparison

The C++ and MATLAB results should show the same flow physics and similar validation trends. They are not expected to produce identical numbers at every iteration because the floating-point update order is different.
