# Phase 2 Local Test Record

The following regression sets were run during the Phase 2 integration work.

## Canonical strict case

```text
N = 32
Re = 100
scheme = upwind
pressure = RBSOR
status = converged
iterations = 2086
velocity-update Linf = 8.95e-09
divergence Linf = 5.16e-13
Ghia u L2 = 1.13e-02
Ghia v L2 = 8.92e-03
```

## Medium study

All six `N=32` RBSOR cases converged:

| Re | Scheme | Iterations | Ghia u L2 | Ghia v L2 |
|---:|---|---:|---:|---:|
| 100 | upwind | 1686 | 0.0113 | 0.0089 |
| 400 | upwind | 2430 | 0.0784 | 0.1020 |
| 1000 | upwind | 4627 | 0.1427 | 0.1956 |
| 100 | central | 1974 | 0.0037 | 0.0029 |
| 400 | central | 3924 | 0.0355 | 0.0439 |
| 1000 | central | 9454 | 0.0842 | 0.0976 |

## Grid sequence

The `Re=100`, central, RBSOR sequence converged:

| N | Iterations | Ghia u L2 | Ghia v L2 |
|---:|---:|---:|---:|
| 16 | 1967 | 0.0205 | 0.0160 |
| 32 | 1974 | 0.0037 | 0.0029 |
| 64 | 3172 | 0.0013 | 0.0037 |

These timings and iteration counts are configuration- and hardware-dependent. They are recorded as development evidence, while GitHub Actions provides the repeatable acceptance checks.
