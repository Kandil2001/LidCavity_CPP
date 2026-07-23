#include "lidcavity/convergence.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace lidcavity {

ConvergenceTracker::ConvergenceTracker(ConvergenceCriteria criteria) : criteria_(criteria) {
    if (criteria_.consecutive_passes == 0) {
        throw std::invalid_argument("consecutive_passes must be positive");
    }
    if (criteria_.maximum_consecutive_pressure_failures == 0) {
        throw std::invalid_argument("maximum_consecutive_pressure_failures must be positive");
    }
}

SolverStatus ConvergenceTracker::update(std::size_t iteration, const ConvergenceMetrics& metrics) {
    if (status_ != SolverStatus::running) {
        return status_;
    }

    if (!metrics.finite
        || !std::isfinite(metrics.velocity_update_linf)
        || !std::isfinite(metrics.divergence_linf)
        || !std::isfinite(metrics.divergence_l2)
        || !std::isfinite(metrics.global_mass_imbalance)) {
        status_ = SolverStatus::non_finite;
        return status_;
    }

    if (std::max({metrics.velocity_update_linf,
                  metrics.divergence_linf,
                  metrics.divergence_l2,
                  metrics.global_mass_imbalance}) > criteria_.diverged_limit) {
        status_ = SolverStatus::diverged;
        return status_;
    }

    if (metrics.pressure_converged) {
        consecutive_pressure_failures_ = 0;
    } else {
        ++consecutive_pressure_failures_;
        consecutive_passes_ = 0;
        if (consecutive_pressure_failures_ >= criteria_.maximum_consecutive_pressure_failures) {
            status_ = SolverStatus::pressure_not_converged;
        }
        return status_;
    }

    const bool passed =
        metrics.velocity_update_linf <= criteria_.velocity_update_linf
        && metrics.divergence_linf <= criteria_.divergence_linf
        && metrics.divergence_l2 <= criteria_.divergence_l2
        && metrics.global_mass_imbalance <= criteria_.global_mass_imbalance;

    if (iteration < criteria_.minimum_iterations || !passed) {
        consecutive_passes_ = 0;
        return status_;
    }

    ++consecutive_passes_;
    if (consecutive_passes_ >= criteria_.consecutive_passes) {
        status_ = SolverStatus::converged;
    }
    return status_;
}

SolverStatus ConvergenceTracker::finish_at_max_iterations() {
    if (status_ == SolverStatus::running) {
        status_ = SolverStatus::max_iterations;
    }
    return status_;
}

std::string to_string(SolverStatus status) {
    switch (status) {
        case SolverStatus::running: return "running";
        case SolverStatus::converged: return "converged";
        case SolverStatus::max_iterations: return "max_iterations";
        case SolverStatus::pressure_not_converged: return "pressure_not_converged";
        case SolverStatus::stagnated: return "stagnated";
        case SolverStatus::diverged: return "diverged";
        case SolverStatus::non_finite: return "non_finite";
    }
    return "unknown";
}

}  // namespace lidcavity
