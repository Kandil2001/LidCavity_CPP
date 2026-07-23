#include "lidcavity/operators.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace lidcavity {
namespace {

void validate_spacing(double dx, double dy) {
    if (!(dx > 0.0) || !(dy > 0.0)) {
        throw std::invalid_argument("Grid spacing must be positive");
    }
}

void validate_same_shape(const Field2D& a, const Field2D& b) {
    if (a.nx() != b.nx() || a.ny() != b.ny()) {
        throw std::invalid_argument("Field shapes must match");
    }
}

}  // namespace

VectorField2D forward_gradient(const Field2D& scalar, double dx, double dy) {
    validate_spacing(dx, dy);
    VectorField2D gradient{Field2D(scalar.ny(), scalar.nx(), 0.0),
                           Field2D(scalar.ny(), scalar.nx(), 0.0)};

    for (std::size_t i = 0; i < scalar.ny(); ++i) {
        for (std::size_t j = 0; j + 1 < scalar.nx(); ++j) {
            gradient.x(i, j) = (scalar(i, j + 1) - scalar(i, j)) / dx;
        }
    }

    for (std::size_t i = 0; i + 1 < scalar.ny(); ++i) {
        for (std::size_t j = 0; j < scalar.nx(); ++j) {
            gradient.y(i, j) = (scalar(i + 1, j) - scalar(i, j)) / dy;
        }
    }

    return gradient;
}

Field2D backward_divergence(const VectorField2D& vector, double dx, double dy) {
    validate_spacing(dx, dy);
    validate_same_shape(vector.x, vector.y);
    Field2D divergence(vector.x.ny(), vector.x.nx(), 0.0);

    for (std::size_t i = 1; i + 1 < vector.x.ny(); ++i) {
        for (std::size_t j = 1; j + 1 < vector.x.nx(); ++j) {
            divergence(i, j) = (vector.x(i, j) - vector.x(i, j - 1)) / dx
                             + (vector.y(i, j) - vector.y(i - 1, j)) / dy;
        }
    }

    return divergence;
}

Field2D compatible_laplacian(const Field2D& scalar, double dx, double dy) {
    return backward_divergence(forward_gradient(scalar, dx, dy), dx, dy);
}

Field2D five_point_laplacian(const Field2D& scalar, double dx, double dy) {
    validate_spacing(dx, dy);
    Field2D laplacian(scalar.ny(), scalar.nx(), 0.0);
    const double inv_dx2 = 1.0 / (dx * dx);
    const double inv_dy2 = 1.0 / (dy * dy);

    for (std::size_t i = 1; i + 1 < scalar.ny(); ++i) {
        for (std::size_t j = 1; j + 1 < scalar.nx(); ++j) {
            laplacian(i, j) = (scalar(i, j + 1) - 2.0 * scalar(i, j) + scalar(i, j - 1)) * inv_dx2
                            + (scalar(i + 1, j) - 2.0 * scalar(i, j) + scalar(i - 1, j)) * inv_dy2;
        }
    }

    return laplacian;
}

double linf_norm(const Field2D& field) {
    double norm = 0.0;
    for (const double value : field.values()) {
        norm = std::max(norm, std::abs(value));
    }
    return norm;
}

double l2_norm(const Field2D& field) {
    long double sum = 0.0L;
    for (const double value : field.values()) {
        sum += static_cast<long double>(value) * static_cast<long double>(value);
    }
    return std::sqrt(static_cast<double>(sum / static_cast<long double>(field.size())));
}

bool all_finite(const Field2D& field) {
    return std::all_of(field.values().begin(), field.values().end(), [](double value) {
        return std::isfinite(value);
    });
}

}  // namespace lidcavity
