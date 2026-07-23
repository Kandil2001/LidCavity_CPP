#pragma once

#include "lidcavity/field.hpp"

namespace lidcavity {

struct VectorField2D {
    Field2D x;
    Field2D y;
};

[[nodiscard]] VectorField2D forward_gradient(const Field2D& scalar, double dx, double dy);
[[nodiscard]] Field2D backward_divergence(const VectorField2D& vector, double dx, double dy);
[[nodiscard]] Field2D compatible_laplacian(const Field2D& scalar, double dx, double dy);
[[nodiscard]] Field2D five_point_laplacian(const Field2D& scalar, double dx, double dy);

[[nodiscard]] double linf_norm(const Field2D& field);
[[nodiscard]] double l2_norm(const Field2D& field);
[[nodiscard]] bool all_finite(const Field2D& field);

}  // namespace lidcavity
