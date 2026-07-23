#pragma once

#include <cstddef>
#include <string>

namespace lidcavity {

enum class SolverStatus {
    running,
    converged,
    max_iterations,
    pressure_not_converged,
    stagnated,
    diverged,
    non_finite
};

struct ConvergenceCriteria {
    double velocity_update_linf = 5e-7;
    double divergence_linf = 2e-3;
    double divergence_l2 = 5e-4;
    double global_mass_imbalance = 1e-8;
    std::size_t minimum_iterations = 100;
    std::size_t consecutive_passes = 10;
    std::size_t maximum_consecutive_pressure_failures = 3;
    double diverged_limit = 1e6;
};

struct ConvergenceMetrics {
    double velocity_update_linf = 0.0;
    double divergence_linf = 0.0;
    double divergence_l2 = 0.0;
    double global_mass_imbalance = 0.0;
    bool pressure_converged = true;
    bool finite = true;
};

class ConvergenceTracker {
public:
    explicit ConvergenceTracker(ConvergenceCriteria criteria = {});

    SolverStatus update(std::size_t iteration, const ConvergenceMetrics& metrics);
    SolverStatus finish_at_max_iterations();

    [[nodiscard]] SolverStatus status() const noexcept { return status_; }
    [[nodiscard]] std::size_t consecutive_passes() const noexcept { return consecutive_passes_; }
    [[nodiscard]] std::size_t consecutive_pressure_failures() const noexcept {
        return consecutive_pressure_failures_;
    }

private:
    ConvergenceCriteria criteria_;
    SolverStatus status_ = SolverStatus::running;
    std::size_t consecutive_passes_ = 0;
    std::size_t consecutive_pressure_failures_ = 0;
};

[[nodiscard]] std::string to_string(SolverStatus status);

}  // namespace lidcavity
