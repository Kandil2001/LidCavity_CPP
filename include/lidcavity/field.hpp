#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace lidcavity {

class Field2D {
public:
    Field2D() = default;
    Field2D(std::size_t ny, std::size_t nx, double value = 0.0)
        : ny_(ny), nx_(nx), values_(ny * nx, value) {
        if (ny < 2 || nx < 2) {
            throw std::invalid_argument("Field2D requires at least 2x2 points");
        }
    }

    [[nodiscard]] std::size_t nx() const noexcept { return nx_; }
    [[nodiscard]] std::size_t ny() const noexcept { return ny_; }
    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }

    double& operator()(std::size_t i, std::size_t j) {
        return values_.at(i * nx_ + j);
    }

    double operator()(std::size_t i, std::size_t j) const {
        return values_.at(i * nx_ + j);
    }

    [[nodiscard]] const std::vector<double>& values() const noexcept { return values_; }
    [[nodiscard]] std::vector<double>& values() noexcept { return values_; }

private:
    std::size_t ny_ = 0;
    std::size_t nx_ = 0;
    std::vector<double> values_;
};

}  // namespace lidcavity
