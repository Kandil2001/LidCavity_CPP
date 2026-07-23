#pragma once

#include <cstddef>
#include <string>

#include "lidcavity/field.hpp"

namespace lidcavity {

enum class PoissonMethod { rbgs, rbsor };

struct PoissonOptions {
    PoissonMethod method = PoissonMethod::rbsor;
    std::size_t max_iterations = 20000;
    double absolute_tolerance = 1e-12;
    double relative_tolerance = 1e-10;
    double omega = 1.7;
    std::size_t check_every = 10;
};

struct PoissonResult {
    Field2D solution;
    std::size_t iterations = 0;
    bool converged = false;
    double absolute_residual = 0.0;
    double relative_residual = 0.0;
};

[[nodiscard]] PoissonResult solve_poisson_dirichlet(
    const Field2D& rhs,
    double dx,
    double dy,
    const PoissonOptions& options = {}
);

[[nodiscard]] double poisson_residual_linf(
    const Field2D& solution,
    const Field2D& rhs,
    double dx,
    double dy
);

[[nodiscard]] std::string to_string(PoissonMethod method);

}  // namespace lidcavity
