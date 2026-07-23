#include <cstdlib>
#include <iostream>
#include <limits>

#include "lidcavity/convergence.hpp"

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

    ConvergenceCriteria criteria;
    criteria.minimum_iterations = 5;
    criteria.consecutive_passes = 3;
    criteria.maximum_consecutive_pressure_failures = 2;

    ConvergenceTracker tracker(criteria);
    ConvergenceMetrics passing;
    passing.velocity_update_linf = criteria.velocity_update_linf * 0.5;
    passing.divergence_linf = criteria.divergence_linf * 0.5;
    passing.divergence_l2 = criteria.divergence_l2 * 0.5;
    passing.global_mass_imbalance = criteria.global_mass_imbalance * 0.5;

    for (std::size_t iteration = 1; iteration <= 6; ++iteration) {
        require(tracker.update(iteration, passing) == SolverStatus::running,
                "tracker waits for minimum iterations and consecutive passes");
    }
    require(tracker.update(7, passing) == SolverStatus::converged,
            "tracker converges after the required consecutive passes");

    ConvergenceTracker pressure_failure(criteria);
    ConvergenceMetrics failed_pressure = passing;
    failed_pressure.pressure_converged = false;
    require(pressure_failure.update(1, failed_pressure) == SolverStatus::running,
            "one pressure failure does not stop immediately");
    require(pressure_failure.update(2, failed_pressure) == SolverStatus::pressure_not_converged,
            "repeated pressure failures are reported explicitly");

    ConvergenceTracker non_finite(criteria);
    ConvergenceMetrics invalid = passing;
    invalid.velocity_update_linf = std::numeric_limits<double>::quiet_NaN();
    require(non_finite.update(1, invalid) == SolverStatus::non_finite,
            "non-finite residuals are reported explicitly");

    ConvergenceTracker maxed(criteria);
    require(maxed.finish_at_max_iterations() == SolverStatus::max_iterations,
            "maximum-iteration termination is distinct from convergence");

    std::cout << "Convergence-contract tests passed.\n";
    return EXIT_SUCCESS;
}
