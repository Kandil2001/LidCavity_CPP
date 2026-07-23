#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "lidcavity/operators.hpp"

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

int main() {
    using namespace lidcavity;
    constexpr std::size_t n = 33;
    const double h = 1.0 / static_cast<double>(n - 1);

    Field2D constant(n, n, 3.25);
    const auto constant_gradient = forward_gradient(constant, h, h);
    require(linf_norm(constant_gradient.x) < 1e-14, "constant scalar has zero x-gradient");
    require(linf_norm(constant_gradient.y) < 1e-14, "constant scalar has zero y-gradient");

    Field2D scalar(n, n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            const double x = static_cast<double>(j) * h;
            const double y = static_cast<double>(i) * h;
            scalar(i, j) = std::sin(2.0 * x) + 0.5 * y * y + x * y;
        }
    }

    const Field2D compatible = compatible_laplacian(scalar, h, h);
    const Field2D five_point = five_point_laplacian(scalar, h, h);
    double mismatch = 0.0;
    for (std::size_t i = 1; i + 1 < n; ++i) {
        for (std::size_t j = 1; j + 1 < n; ++j) {
            mismatch = std::max(mismatch, std::abs(compatible(i, j) - five_point(i, j)));
        }
    }
    require(mismatch < 1e-11, "D(G(phi)) matches the five-point Laplacian in the interior");

    VectorField2D zero{Field2D(n, n, 0.0), Field2D(n, n, 0.0)};
    require(linf_norm(backward_divergence(zero, h, h)) < 1e-14, "zero velocity has zero divergence");

    scalar(5, 5) = std::numeric_limits<double>::quiet_NaN();
    require(!all_finite(scalar), "non-finite values are detected");

    std::cout << "Operator verification passed; compatibility mismatch=" << mismatch << '\n';
    return EXIT_SUCCESS;
}
