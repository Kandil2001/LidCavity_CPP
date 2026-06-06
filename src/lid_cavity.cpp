#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

constexpr double PI = 3.141592653589793238462643383279502884;

struct Matrix {
    int n{};
    std::vector<double> a;

    Matrix() = default;
    explicit Matrix(int n_, double value = 0.0) : n(n_), a(static_cast<size_t>(n_) * n_, value) {}

    double& operator()(int i, int j) { return a[static_cast<size_t>(i) * n + j]; }
    double operator()(int i, int j) const { return a[static_cast<size_t>(i) * n + j]; }
};

struct Config {
    double U_lid = 1.0;
    double L = 1.0;

    int maxIter = 4000;
    int maxIter_N128_bonus = 3000;
    int maxIter_Re1000_bonus = 3000;
    int maxIter_central_bonus = 1500;

    double tol_mass = 1e-7;
    double tol_divergence = 2e-3;
    double tol_velocity = 5e-7;
    double diverged_limit = 1e6;

    double cfl = 0.25;
    double dt_max = 0.0025;
    double dt_min = 1e-6;

    double alpha_u = 0.55;
    double alpha_p = 0.20;

    int poisson_maxIter = 2500;
    double poisson_tol_abs = 1e-8;
    double poisson_tol_rel = 1e-4;
    int poisson_check_every = 25;

    std::string sor_omega = "auto";
    double sor_omega_min = 1.15;
    double sor_omega_max = 1.90;

    std::vector<int> meshes = {32, 64, 128};
    std::vector<int> re_list = {100, 400, 1000};
    std::vector<std::string> schemes = {"upwind", "central"};
    std::vector<std::string> pressure_solvers = {"RBGS", "RBSOR"};
    std::vector<std::string> implementations = {"serial_cpp"};

    double validation_u_L2_limit_Re100 = 0.030;
    double validation_v_L2_limit_Re100 = 0.030;
    double validation_u_L2_limit_Re400 = 0.090;
    double validation_v_L2_limit_Re400 = 0.120;
    double validation_u_L2_limit_Re1000 = 0.160;
    double validation_v_L2_limit_Re1000 = 0.180;

    bool save_fields = true;
    std::string results_dir = "results";
    std::string data_dir = "results/data";
};

struct PoissonInfo {
    int iter = 0;
    bool converged = false;
    double final_true_residual = std::numeric_limits<double>::infinity();
    double final_relative_residual = std::numeric_limits<double>::infinity();
    double final_change = std::numeric_limits<double>::infinity();
    double omega = 1.0;
    std::vector<double> residual_history;
    std::vector<double> change_history;
};

struct Result {
    int N = 0;
    int Re = 0;
    std::string scheme;
    std::string pressure_solver;
    std::string implementation;

    std::vector<double> x, y;
    Matrix u, v, p, speed, vorticity;

    std::vector<double> Ru, Rv, Rc_mass, Rc_div, dt, poisson_relative_residual;
    std::vector<int> poisson_iters;
    std::vector<bool> poisson_converged;

    int iterations = 0;
    int localMaxIter = 0;
    double runtime = 0.0;
    std::string status = "maxIter";
    double final_Ru = 0.0;
    double final_Rv = 0.0;
    double final_Rc_mass = 0.0;
    double final_Rc_div = 0.0;
    double avg_poisson_iters = 0.0;
    double avg_poisson_relative_residual = 0.0;
    double pressure_saturation_ratio = 0.0;
    int stagnation_counter = 0;
};

struct GhiaData {
    int Re = 0;
    std::vector<double> y_u, u, x_v, v;
};

struct Metrics {
    bool available = false;
    bool pass = false;
    double u_L2 = std::numeric_limits<double>::quiet_NaN();
    double v_L2 = std::numeric_limits<double>::quiet_NaN();
    double u_Linf = std::numeric_limits<double>::quiet_NaN();
    double v_Linf = std::numeric_limits<double>::quiet_NaN();
    double u_limit = std::numeric_limits<double>::quiet_NaN();
    double v_limit = std::numeric_limits<double>::quiet_NaN();
};

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

static std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    return s;
}

static std::string normalize_implementation(std::string s) {
    s = lower(s);
    // MATLAB has two implementations: vectorized and loop. This C++ package
    // intentionally contains one honest serial loop kernel. For convenience,
    // MATLAB labels are accepted as aliases, but CSV output uses serial_cpp.
    if (s == "serial" || s == "serial_cpp" || s == "cpp" || s == "loop" || s == "vectorized") {
        return "serial_cpp";
    }
    throw std::runtime_error("Unknown implementation: " + s + " (use serial_cpp)");
}

static double max_abs(const Matrix& m) {
    double r = 0.0;
    for (double v : m.a) r = std::max(r, std::abs(v));
    return r;
}

static double mean_all(const Matrix& m) {
    if (m.a.empty()) return 0.0;
    return std::accumulate(m.a.begin(), m.a.end(), 0.0) / static_cast<double>(m.a.size());
}

static bool all_finite(const Matrix& m) {
    for (double x : m.a) {
        if (!std::isfinite(x)) return false;
    }
    return true;
}

static void apply_lid_bc(Matrix& u, Matrix& v, double U_lid) {
    const int N = u.n;
    for (int j = 0; j < N; ++j) {
        u(0, j) = 0.0;         // bottom wall
        u(N - 1, j) = U_lid;   // moving lid
    }
    for (int i = 0; i < N; ++i) {
        u(i, 0) = 0.0;         // left wall; also sets top-left corner to zero
        u(i, N - 1) = 0.0;     // right wall; also sets top-right corner to zero
    }

    for (int j = 0; j < N; ++j) {
        v(0, j) = 0.0;
        v(N - 1, j) = 0.0;
    }
    for (int i = 0; i < N; ++i) {
        v(i, 0) = 0.0;
        v(i, N - 1) = 0.0;
    }
}

static void apply_pressure_bc(Matrix& p) {
    const int N = p.n;
    for (int i = 0; i < N; ++i) {
        p(i, 0) = p(i, 1);
        p(i, N - 1) = p(i, N - 2);
    }
    for (int j = 0; j < N; ++j) {
        p(0, j) = p(1, j);
        p(N - 1, j) = p(N - 2, j);
    }
    p(0, 0) = 0.0;
}

static double compute_dt(const Matrix& u, const Matrix& v, double dx, double dy, double nu, const Config& cfg) {
    const double max_vel = std::max({max_abs(u), max_abs(v), cfg.U_lid, 1e-12});
    const double h = std::min(dx, dy);
    const double dt_conv = cfg.cfl * h / max_vel;
    const double dt_diff = (nu > 0.0) ? 0.25 * h * h / nu : std::numeric_limits<double>::infinity();
    return std::min({dt_conv, dt_diff, cfg.dt_max});
}

static Matrix divergence_field(const Matrix& u, const Matrix& v, double dx, double dy) {
    const int N = u.n;
    Matrix div(N, 0.0);
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            div(i, j) = (u(i, j + 1) - u(i, j - 1)) / (2.0 * dx)
                      + (v(i + 1, j) - v(i - 1, j)) / (2.0 * dy);
        }
    }
    return div;
}

static Matrix compute_vorticity(const Matrix& u, const Matrix& v, double dx, double dy) {
    const int N = u.n;
    Matrix omega(N, 0.0);
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            omega(i, j) = (v(i, j + 1) - v(i, j - 1)) / (2.0 * dx)
                        - (u(i + 1, j) - u(i - 1, j)) / (2.0 * dy);
        }
    }
    return omega;
}

static std::tuple<Matrix, Matrix, double> momentum_predictor(
    const Matrix& u, const Matrix& v, const Matrix& p,
    int Re, const std::string& scheme_in, const Config& cfg
) {
    const int N = u.n;
    const double dx = cfg.L / static_cast<double>(N - 1);
    const double dy = dx;
    const double nu = cfg.U_lid * cfg.L / static_cast<double>(Re);
    const double dt = compute_dt(u, v, dx, dy, nu, cfg);
    const std::string scheme = lower(scheme_in);

    Matrix u_star = u;
    Matrix v_star = v;

    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            const double uC = u(i, j);
            const double vC = v(i, j);

            const double lap_u = (u(i, j + 1) - 2.0 * u(i, j) + u(i, j - 1)) / (dx * dx)
                               + (u(i + 1, j) - 2.0 * u(i, j) + u(i - 1, j)) / (dy * dy);
            const double lap_v = (v(i, j + 1) - 2.0 * v(i, j) + v(i, j - 1)) / (dx * dx)
                               + (v(i + 1, j) - 2.0 * v(i, j) + v(i - 1, j)) / (dy * dy);

            double du_dx, du_dy, dv_dx, dv_dy;

            if (scheme == "central") {
                du_dx = (u(i, j + 1) - u(i, j - 1)) / (2.0 * dx);
                du_dy = (u(i + 1, j) - u(i - 1, j)) / (2.0 * dy);
                dv_dx = (v(i, j + 1) - v(i, j - 1)) / (2.0 * dx);
                dv_dy = (v(i + 1, j) - v(i - 1, j)) / (2.0 * dy);
            } else if (scheme == "upwind") {
                if (uC >= 0.0) {
                    du_dx = (u(i, j) - u(i, j - 1)) / dx;
                    dv_dx = (v(i, j) - v(i, j - 1)) / dx;
                } else {
                    du_dx = (u(i, j + 1) - u(i, j)) / dx;
                    dv_dx = (v(i, j + 1) - v(i, j)) / dx;
                }

                if (vC >= 0.0) {
                    du_dy = (u(i, j) - u(i - 1, j)) / dy;
                    dv_dy = (v(i, j) - v(i - 1, j)) / dy;
                } else {
                    du_dy = (u(i + 1, j) - u(i, j)) / dy;
                    dv_dy = (v(i + 1, j) - v(i, j)) / dy;
                }
            } else {
                throw std::runtime_error("Unknown convection scheme: " + scheme_in);
            }

            const double conv_u = uC * du_dx + vC * du_dy;
            const double conv_v = uC * dv_dx + vC * dv_dy;
            const double dp_dx = (p(i, j + 1) - p(i, j - 1)) / (2.0 * dx);
            const double dp_dy = (p(i + 1, j) - p(i - 1, j)) / (2.0 * dy);

            const double u_pred = u(i, j) + dt * (-conv_u - dp_dx + nu * lap_u);
            const double v_pred = v(i, j) + dt * (-conv_v - dp_dy + nu * lap_v);

            u_star(i, j) = (1.0 - cfg.alpha_u) * u(i, j) + cfg.alpha_u * u_pred;
            v_star(i, j) = (1.0 - cfg.alpha_u) * v(i, j) + cfg.alpha_u * v_pred;
        }
    }

    apply_lid_bc(u_star, v_star, cfg.U_lid);
    return {u_star, v_star, dt};
}

static double poisson_true_residual(const Matrix& phi, const Matrix& rhs, double dx, double dy) {
    const int N = phi.n;
    double res = 0.0;
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            const double lap = (phi(i, j + 1) - 2.0 * phi(i, j) + phi(i, j - 1)) / (dx * dx)
                             + (phi(i + 1, j) - 2.0 * phi(i, j) + phi(i - 1, j)) / (dy * dy);
            res = std::max(res, std::abs(lap - rhs(i, j)));
        }
    }
    return res;
}

static std::tuple<Matrix, PoissonInfo> pressure_poisson(
    const Matrix& rhs, double dx, double dy, const std::string& solver_type_in, const Config& cfg
) {
    const int N = rhs.n;
    Matrix phi(N, 0.0);
    Matrix rhs2 = rhs;
    const std::string solver_type = upper(solver_type_in);

    double interior_sum = 0.0;
    int count = 0;
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            interior_sum += rhs2(i, j);
            ++count;
        }
    }
    const double interior_mean = (count > 0) ? interior_sum / static_cast<double>(count) : 0.0;
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            rhs2(i, j) -= interior_mean;
        }
    }

    const double den = 2.0 * (dx * dx + dy * dy);
    double omega;
    if (lower(cfg.sor_omega) == "auto") {
        omega = 2.0 / (1.0 + std::sin(PI / static_cast<double>(N - 1)));
        omega = std::min(std::max(omega, cfg.sor_omega_min), cfg.sor_omega_max);
    } else {
        omega = std::stod(cfg.sor_omega);
    }

    double rhs_norm = 1.0;
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            rhs_norm = std::max(rhs_norm, std::abs(rhs2(i, j)));
        }
    }

    PoissonInfo info;
    info.omega = omega;
    info.residual_history.assign(static_cast<size_t>(cfg.poisson_maxIter), 0.0);
    info.change_history.assign(static_cast<size_t>(cfg.poisson_maxIter), 0.0);

    bool converged = false;
    double final_res = std::numeric_limits<double>::infinity();
    double final_change = std::numeric_limits<double>::infinity();
    int it = 0;

    for (it = 1; it <= cfg.poisson_maxIter; ++it) {
        Matrix phi_old = phi;

        if (solver_type == "JACOBI") {
            Matrix phi_new = phi;
            for (int i = 1; i < N - 1; ++i) {
                for (int j = 1; j < N - 1; ++j) {
                    phi_new(i, j) = ((phi(i + 1, j) + phi(i - 1, j)) * dy * dy
                                   + (phi(i, j + 1) + phi(i, j - 1)) * dx * dx
                                   - rhs2(i, j) * dx * dx * dy * dy) / den;
                }
            }
            phi = std::move(phi_new);
            apply_pressure_bc(phi);
        } else if (solver_type == "RBGS" || solver_type == "RBSOR") {
            for (int color = 0; color <= 1; ++color) {
                for (int i = 1; i < N - 1; ++i) {
                    for (int j = 1; j < N - 1; ++j) {
                        const bool is_red = ((i + 1) + (j + 1)) % 2 == 0; // MATLAB i+j parity
                        if ((color == 0 && !is_red) || (color == 1 && is_red)) continue;

                        const double candidate = ((phi(i + 1, j) + phi(i - 1, j)) * dy * dy
                                                + (phi(i, j + 1) + phi(i, j - 1)) * dx * dx
                                                - rhs2(i, j) * dx * dx * dy * dy) / den;
                        if (solver_type == "RBSOR") {
                            phi(i, j) = (1.0 - omega) * phi(i, j) + omega * candidate;
                        } else {
                            phi(i, j) = candidate;
                        }
                    }
                }
                apply_pressure_bc(phi);
            }
        } else {
            throw std::runtime_error("Unknown pressure solver: " + solver_type_in);
        }

        final_change = 0.0;
        for (size_t k = 0; k < phi.a.size(); ++k) {
            final_change = std::max(final_change, std::abs(phi.a[k] - phi_old.a[k]));
        }
        info.change_history[static_cast<size_t>(it - 1)] = final_change;

        if ((it % cfg.poisson_check_every) == 0 || it == 1 || it == cfg.poisson_maxIter) {
            final_res = poisson_true_residual(phi, rhs2, dx, dy);
            const double rel_res = final_res / rhs_norm;
            info.residual_history[static_cast<size_t>(it - 1)] = rel_res;
            if (final_res < cfg.poisson_tol_abs || rel_res < cfg.poisson_tol_rel) {
                converged = true;
                break;
            }
        }
    }

    if (it > cfg.poisson_maxIter) {
        it = cfg.poisson_maxIter;
    }

    if (!converged) {
        final_res = poisson_true_residual(phi, rhs2, dx, dy);
    }

    info.iter = it;
    info.converged = converged;
    info.final_true_residual = final_res;
    info.final_relative_residual = final_res / rhs_norm;
    info.final_change = final_change;
    info.residual_history.resize(static_cast<size_t>(it));
    info.change_history.resize(static_cast<size_t>(it));
    return {phi, info};
}

static std::tuple<double, double, double, double> velocity_residuals(
    const Matrix& u, const Matrix& v, const Matrix& u_old, const Matrix& v_old,
    double dx, double dy, double U, double L
) {
    double Ru = 0.0;
    double Rv = 0.0;
    for (size_t k = 0; k < u.a.size(); ++k) {
        Ru = std::max(Ru, std::abs(u.a[k] - u_old.a[k]));
        Rv = std::max(Rv, std::abs(v.a[k] - v_old.a[k]));
    }

    Matrix div = divergence_field(u, v, dx, dy);
    const double Rc_div = max_abs(div);
    const double scale = std::max(U * L, std::numeric_limits<double>::epsilon());
    const double Rc_mass = Rc_div * dx * dy / scale;
    return {Ru, Rv, Rc_mass, Rc_div};
}

static Result solve_lid_cavity(int N, int Re, std::string scheme, std::string pressure_solver, std::string implementation, const Config& cfg) {
    scheme = lower(scheme);
    pressure_solver = upper(pressure_solver);
    implementation = normalize_implementation(implementation);

    const double dx = cfg.L / static_cast<double>(N - 1);
    const double dy = dx;

    int localMaxIter = cfg.maxIter;
    if (N >= 128) localMaxIter += cfg.maxIter_N128_bonus;
    if (Re >= 1000) localMaxIter += cfg.maxIter_Re1000_bonus;
    if (scheme == "central") localMaxIter += cfg.maxIter_central_bonus;

    Matrix u(N, 0.0), v(N, 0.0), p(N, 0.0);
    apply_lid_bc(u, v, cfg.U_lid);

    Result result;
    result.N = N;
    result.Re = Re;
    result.scheme = scheme;
    result.pressure_solver = pressure_solver;
    result.implementation = implementation;
    result.localMaxIter = localMaxIter;

    result.Ru.reserve(localMaxIter);
    result.Rv.reserve(localMaxIter);
    result.Rc_mass.reserve(localMaxIter);
    result.Rc_div.reserve(localMaxIter);
    result.dt.reserve(localMaxIter);
    result.poisson_iters.reserve(localMaxIter);
    result.poisson_relative_residual.reserve(localMaxIter);
    result.poisson_converged.reserve(localMaxIter);

    std::string status = "maxIter";
    int stagnation_counter = 0;
    double prev_mass = std::numeric_limits<double>::infinity();
    int iter = 0;

    const auto t0 = std::chrono::steady_clock::now();

    for (iter = 1; iter <= localMaxIter; ++iter) {
        Matrix u_old = u;
        Matrix v_old = v;

        auto [u_star, v_star, dt] = momentum_predictor(u, v, p, Re, scheme, cfg);
        dt = std::max(dt, cfg.dt_min);

        Matrix div_star = divergence_field(u_star, v_star, dx, dy);
        Matrix rhs(N, 0.0);
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                rhs(i, j) = div_star(i, j) / dt;
            }
        }

        auto [p_prime, pinfo] = pressure_poisson(rhs, dx, dy, pressure_solver, cfg);

        u = u_star;
        v = v_star;
        for (int i = 1; i < N - 1; ++i) {
            for (int j = 1; j < N - 1; ++j) {
                const double dpdx = (p_prime(i, j + 1) - p_prime(i, j - 1)) / (2.0 * dx);
                const double dpdy = (p_prime(i + 1, j) - p_prime(i - 1, j)) / (2.0 * dy);
                u(i, j) = u_star(i, j) - dt * dpdx;
                v(i, j) = v_star(i, j) - dt * dpdy;
            }
        }

        for (size_t k = 0; k < p.a.size(); ++k) {
            p.a[k] += cfg.alpha_p * p_prime.a[k];
        }
        const double p_mean = mean_all(p);
        for (double& val : p.a) val -= p_mean;

        apply_lid_bc(u, v, cfg.U_lid);

        auto [Ru, Rv, Rc_mass, Rc_div] = velocity_residuals(u, v, u_old, v_old, dx, dy, cfg.U_lid, cfg.L);
        result.Ru.push_back(Ru);
        result.Rv.push_back(Rv);
        result.Rc_mass.push_back(Rc_mass);
        result.Rc_div.push_back(Rc_div);
        result.dt.push_back(dt);
        result.poisson_iters.push_back(pinfo.iter);
        result.poisson_relative_residual.push_back(pinfo.final_relative_residual);
        result.poisson_converged.push_back(pinfo.converged);

        if (!all_finite(u) || !all_finite(v) || !all_finite(p) || std::max({Ru, Rv, Rc_div}) > cfg.diverged_limit) {
            status = "diverged";
            break;
        }

        if (Rc_mass > 0.995 * prev_mass) {
            ++stagnation_counter;
        } else {
            stagnation_counter = 0;
        }
        prev_mass = Rc_mass;

        if (Rc_mass < cfg.tol_mass && std::max(Ru, Rv) < cfg.tol_velocity) {
            status = "converged";
            break;
        }
    }

    if (iter > localMaxIter) {
        iter = localMaxIter;
    }

    const auto t1 = std::chrono::steady_clock::now();
    result.runtime = std::chrono::duration<double>(t1 - t0).count();

    result.iterations = iter;
    result.status = status;
    result.stagnation_counter = stagnation_counter;

    result.x.resize(N);
    result.y.resize(N);
    for (int k = 0; k < N; ++k) {
        result.x[k] = static_cast<double>(k) * cfg.L / static_cast<double>(N - 1);
        result.y[k] = result.x[k];
    }

    result.u = std::move(u);
    result.v = std::move(v);
    result.p = std::move(p);
    result.speed = Matrix(N, 0.0);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            result.speed(i, j) = std::sqrt(result.u(i, j) * result.u(i, j) + result.v(i, j) * result.v(i, j));
        }
    }
    result.vorticity = compute_vorticity(result.u, result.v, dx, dy);

    result.final_Ru = result.Ru.empty() ? 0.0 : result.Ru.back();
    result.final_Rv = result.Rv.empty() ? 0.0 : result.Rv.back();
    result.final_Rc_mass = result.Rc_mass.empty() ? 0.0 : result.Rc_mass.back();
    result.final_Rc_div = result.Rc_div.empty() ? 0.0 : result.Rc_div.back();

    if (!result.poisson_iters.empty()) {
        result.avg_poisson_iters = std::accumulate(result.poisson_iters.begin(), result.poisson_iters.end(), 0.0)
                                 / static_cast<double>(result.poisson_iters.size());
        result.avg_poisson_relative_residual = std::accumulate(result.poisson_relative_residual.begin(), result.poisson_relative_residual.end(), 0.0)
                                            / static_cast<double>(result.poisson_relative_residual.size());
        int saturated = 0;
        for (int pi : result.poisson_iters) if (pi >= cfg.poisson_maxIter) ++saturated;
        result.pressure_saturation_ratio = static_cast<double>(saturated) / static_cast<double>(result.poisson_iters.size());
    }

    return result;
}

static GhiaData ghia_data(int Re) {
    GhiaData d;
    d.Re = Re;
    if (Re == 100) {
        d.y_u = {1.0000,0.9766,0.9688,0.9609,0.9531,0.8516,0.7344,0.6172,0.5000,0.4531,0.2813,0.1719,0.1016,0.0703,0.0625,0.0547,0.0000};
        d.u   = {1.0000,0.84123,0.78871,0.73722,0.68717,0.23151,0.00332,-0.13641,-0.20581,-0.21090,-0.15662,-0.10150,-0.06434,-0.04775,-0.04192,-0.03717,0.0000};
        d.x_v = {1.0000,0.9688,0.9609,0.9531,0.9453,0.9063,0.8594,0.8047,0.5000,0.2344,0.2266,0.1563,0.0938,0.0781,0.0703,0.0625,0.0000};
        d.v   = {0.0000,-0.05906,-0.07391,-0.08864,-0.10313,-0.16914,-0.22445,-0.24533,0.05454,0.17527,0.17507,0.16077,0.12317,0.10890,0.10091,0.09233,0.0000};
    } else if (Re == 400) {
        d.y_u = {1.0000,0.9766,0.9688,0.9609,0.9531,0.8516,0.7344,0.6172,0.5000,0.4531,0.2813,0.1719,0.1016,0.0703,0.0625,0.0547,0.0000};
        d.u   = {1.0000,0.75837,0.68439,0.61756,0.55892,0.29093,0.16256,0.02135,-0.11477,-0.17119,-0.32726,-0.24299,-0.14612,-0.10338,-0.09266,-0.08186,0.0000};
        d.x_v = {1.0000,0.9688,0.9609,0.9531,0.9453,0.9063,0.8594,0.8047,0.5000,0.2344,0.2266,0.1563,0.0938,0.0781,0.0703,0.0625,0.0000};
        d.v   = {0.0000,-0.12146,-0.15663,-0.19254,-0.22847,-0.23827,-0.44993,-0.38598,0.05186,0.30174,0.30203,0.28124,0.22965,0.20920,0.19713,0.18360,0.0000};
    } else if (Re == 1000) {
        d.y_u = {1.0000,0.9766,0.9688,0.9609,0.9531,0.8516,0.7344,0.6172,0.5000,0.4531,0.2813,0.1719,0.1016,0.0703,0.0625,0.0547,0.0000};
        d.u   = {1.0000,0.65928,0.57492,0.51117,0.46604,0.33304,0.18719,0.05702,-0.06080,-0.10648,-0.27805,-0.38289,-0.29730,-0.22220,-0.20196,-0.18109,0.0000};
        d.x_v = {1.0000,0.9688,0.9609,0.9531,0.9453,0.9063,0.8594,0.8047,0.5000,0.2344,0.2266,0.1563,0.0938,0.0781,0.0703,0.0625,0.0000};
        d.v   = {0.0000,-0.21388,-0.27669,-0.33714,-0.39188,-0.51550,-0.42665,-0.31966,0.02526,0.32235,0.33075,0.37095,0.32627,0.30353,0.29012,0.27485,0.0000};
    }
    return d;
}

static double interp_linear(const std::vector<double>& x, const std::vector<double>& y, double q) {
    if (x.empty()) return std::numeric_limits<double>::quiet_NaN();
    if (q <= x.front()) return y.front();
    if (q >= x.back()) return y.back();
    const auto it = std::upper_bound(x.begin(), x.end(), q);
    const size_t k = static_cast<size_t>(std::distance(x.begin(), it));
    const double x0 = x[k - 1], x1 = x[k];
    const double y0 = y[k - 1], y1 = y[k];
    const double t = (q - x0) / (x1 - x0);
    return y0 + t * (y1 - y0);
}

static Metrics validate_against_ghia(const Result& result, const Config& cfg) {
    Metrics m;
    GhiaData d = ghia_data(result.Re);
    if (d.y_u.empty()) return m;
    m.available = true;

    const int N = result.N;
    const int mid = static_cast<int>(std::round((N + 1) / 2.0)) - 1; // MATLAB round((N+1)/2)
    std::vector<double> u_center(N), v_center(N);
    for (int i = 0; i < N; ++i) u_center[i] = result.u(i, mid);
    for (int j = 0; j < N; ++j) v_center[j] = result.v(mid, j);

    double sum_u2 = 0.0, sum_v2 = 0.0;
    double linf_u = 0.0, linf_v = 0.0;
    for (size_t k = 0; k < d.y_u.size(); ++k) {
        const double un = interp_linear(result.y, u_center, d.y_u[k]);
        const double e = un - d.u[k];
        sum_u2 += e * e;
        linf_u = std::max(linf_u, std::abs(e));
    }
    for (size_t k = 0; k < d.x_v.size(); ++k) {
        const double vn = interp_linear(result.x, v_center, d.x_v[k]);
        const double e = vn - d.v[k];
        sum_v2 += e * e;
        linf_v = std::max(linf_v, std::abs(e));
    }

    m.u_L2 = std::sqrt(sum_u2 / static_cast<double>(d.y_u.size()));
    m.v_L2 = std::sqrt(sum_v2 / static_cast<double>(d.x_v.size()));
    m.u_Linf = linf_u;
    m.v_Linf = linf_v;

    if (result.Re == 100) {
        m.u_limit = cfg.validation_u_L2_limit_Re100;
        m.v_limit = cfg.validation_v_L2_limit_Re100;
    } else if (result.Re == 400) {
        m.u_limit = cfg.validation_u_L2_limit_Re400;
        m.v_limit = cfg.validation_v_L2_limit_Re400;
    } else if (result.Re == 1000) {
        m.u_limit = cfg.validation_u_L2_limit_Re1000;
        m.v_limit = cfg.validation_v_L2_limit_Re1000;
    } else {
        m.u_limit = std::numeric_limits<double>::infinity();
        m.v_limit = std::numeric_limits<double>::infinity();
    }
    m.pass = (m.u_L2 <= m.u_limit && m.v_L2 <= m.v_limit);
    return m;
}

static std::string quality_label(const Result& result, const Metrics& metrics) {
    if (result.status == "converged" && metrics.available && metrics.pass) return "converged_validated";
    if (result.status == "converged" && metrics.available && !metrics.pass) return "converged_not_validated";
    if (result.status == "converged" && !metrics.available) return "converged_no_benchmark";
    if (result.status != "converged" && metrics.available && metrics.pass) return "validated_but_not_converged";
    return "needs_improvement";
}

static void write_field_csv(const Result& r, const std::string& case_name, const Config& cfg) {
    fs::create_directories(cfg.data_dir);
    const fs::path path = fs::path(cfg.data_dir) / (case_name + "_fields.csv");
    std::ofstream out(path);
    out << std::setprecision(12);
    out << "i,j,x,y,u,v,p,speed,vorticity\n";
    for (int i = 0; i < r.N; ++i) {
        for (int j = 0; j < r.N; ++j) {
            out << i << ',' << j << ',' << r.x[j] << ',' << r.y[i] << ','
                << r.u(i, j) << ',' << r.v(i, j) << ',' << r.p(i, j) << ','
                << r.speed(i, j) << ',' << r.vorticity(i, j) << '\n';
        }
    }
}

static void write_history_csv(const Result& r, const std::string& case_name, const Config& cfg) {
    fs::create_directories(cfg.data_dir);
    const fs::path path = fs::path(cfg.data_dir) / (case_name + "_history.csv");
    std::ofstream out(path);
    out << std::setprecision(12);
    out << "iter,Ru,Rv,Rc_mass,Rc_div,dt,poisson_iters,poisson_relative_residual,poisson_converged\n";
    for (size_t k = 0; k < r.Ru.size(); ++k) {
        out << (k + 1) << ',' << r.Ru[k] << ',' << r.Rv[k] << ',' << r.Rc_mass[k] << ',' << r.Rc_div[k]
            << ',' << r.dt[k] << ',' << r.poisson_iters[k] << ',' << r.poisson_relative_residual[k]
            << ',' << (r.poisson_converged[k] ? 1 : 0) << '\n';
    }
}

static void write_summary_header(std::ofstream& out) {
    out << "CaseID,Implementation,N,Re,Scheme,PressureSolver,Status,Quality,Iterations,LocalMaxIter,"
        << "FinalRu,FinalRv,FinalRcMass,FinalRcDiv,Runtime_s,AvgPoissonIterations,"
        << "AvgPoissonRelResidual,PressureSaturationRatio,HasGhia,ValidationPass,"
        << "Ghia_u_L2,Ghia_v_L2,Ghia_u_Linf,Ghia_v_Linf,Ghia_u_L2_Limit,Ghia_v_L2_Limit\n";
}

static void write_summary_row(std::ofstream& out, int case_id, const Result& r, const Metrics& m, const std::string& quality) {
    out << std::setprecision(12);
    out << case_id << ',' << r.implementation << ',' << r.N << ',' << r.Re << ',' << r.scheme << ',' << r.pressure_solver << ','
        << r.status << ',' << quality << ',' << r.iterations << ',' << r.localMaxIter << ','
        << r.final_Ru << ',' << r.final_Rv << ',' << r.final_Rc_mass << ',' << r.final_Rc_div << ',' << r.runtime << ','
        << r.avg_poisson_iters << ',' << r.avg_poisson_relative_residual << ',' << r.pressure_saturation_ratio << ','
        << (m.available ? 1 : 0) << ',' << (m.pass ? 1 : 0) << ','
        << m.u_L2 << ',' << m.v_L2 << ',' << m.u_Linf << ',' << m.v_Linf << ',' << m.u_limit << ',' << m.v_limit << '\n';
}

static void configure_mode(Config& cfg, const std::string& mode) {
    const std::string m = lower(mode);
    if (m == "quick") {
        cfg.meshes = {32, 64};
        cfg.re_list = {100, 400};
        cfg.maxIter = 2000;
        cfg.maxIter_N128_bonus = 0;
        cfg.maxIter_Re1000_bonus = 0;
        cfg.maxIter_central_bonus = 500;
        cfg.poisson_maxIter = 1200;
    } else if (m == "medium") {
        cfg.meshes = {32, 64};
        cfg.re_list = {100, 400, 1000};
        cfg.maxIter = 3500;
        cfg.maxIter_N128_bonus = 0;
        cfg.poisson_maxIter = 1800;
    } else if (m == "full") {
        // defaults, equivalent to main.m
    } else if (m == "single") {
        cfg.meshes = {64};
        cfg.re_list = {100};
        cfg.schemes = {"central"};
        cfg.pressure_solvers = {"RBGS"};
        cfg.implementations = {"serial_cpp"};
    } else if (m == "smoke") {
        cfg.meshes = {16};
        cfg.re_list = {100};
        cfg.schemes = {"upwind"};
        cfg.pressure_solvers = {"RBGS"};
        cfg.implementations = {"serial_cpp"};
        cfg.maxIter = 20;
        cfg.maxIter_N128_bonus = 0;
        cfg.maxIter_Re1000_bonus = 0;
        cfg.maxIter_central_bonus = 0;
        cfg.poisson_maxIter = 50;
    } else {
        throw std::runtime_error("Unknown mode: " + mode + " (use quick, medium, full, single, or smoke)");
    }
}

static void print_usage(const char* exe) {
    std::cout << "Usage:\n"
              << "  " << exe << " --mode quick|medium|full|single|smoke\n"
              << "  " << exe << " --single --N 64 --Re 100 --scheme central --pressure RBGS --implementation serial_cpp\n\n"
              << "Options:\n"
              << "  --no-fields              Do not write full field CSV files\n"
              << "  --maxIter VALUE          Override base outer iterations\n"
              << "  --poisson-maxIter VALUE  Override pressure solver iterations\n"
              << "\nImplementation note: this C++ package has one serial_cpp solver. MATLAB labels loop/vectorized are accepted as aliases only.\n";
}

int main(int argc, char** argv) {
    Config cfg;
    std::string mode = "quick";
    bool explicit_single = false;
    int single_N = 64;
    int single_Re = 100;
    std::string single_scheme = "central";
    std::string single_pressure = "RBGS";
    std::string single_implementation = "serial_cpp";

    try {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            auto require_value = [&](const std::string& name) -> std::string {
                if (i + 1 >= argc) throw std::runtime_error("Missing value for " + name);
                return argv[++i];
            };

            if (arg == "--help" || arg == "-h") {
                print_usage(argv[0]);
                return 0;
            } else if (arg == "--mode") {
                mode = require_value(arg);
            } else if (arg == "--single") {
                explicit_single = true;
                mode = "single";
            } else if (arg == "--N") {
                single_N = std::stoi(require_value(arg));
                explicit_single = true;
                mode = "single";
            } else if (arg == "--Re") {
                single_Re = std::stoi(require_value(arg));
                explicit_single = true;
                mode = "single";
            } else if (arg == "--scheme") {
                single_scheme = require_value(arg);
                explicit_single = true;
                mode = "single";
            } else if (arg == "--pressure") {
                single_pressure = require_value(arg);
                explicit_single = true;
                mode = "single";
            } else if (arg == "--implementation") {
                single_implementation = require_value(arg);
                explicit_single = true;
                mode = "single";
            } else if (arg == "--no-fields") {
                cfg.save_fields = false;
            } else if (arg == "--maxIter") {
                cfg.maxIter = std::stoi(require_value(arg));
            } else if (arg == "--poisson-maxIter") {
                cfg.poisson_maxIter = std::stoi(require_value(arg));
            } else {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        configure_mode(cfg, mode);
        if (explicit_single) {
            cfg.meshes = {single_N};
            cfg.re_list = {single_Re};
            cfg.schemes = {lower(single_scheme)};
            cfg.pressure_solvers = {upper(single_pressure)};
            cfg.implementations = {normalize_implementation(single_implementation)};
        }

        fs::create_directories(cfg.data_dir);
        const fs::path summary_path = fs::path(cfg.data_dir) / ("study_summary_" + lower(mode) + ".csv");
        std::ofstream summary(summary_path);
        write_summary_header(summary);

        const int nCases = static_cast<int>(cfg.meshes.size() * cfg.re_list.size() * cfg.schemes.size()
                         * cfg.pressure_solvers.size() * cfg.implementations.size());
        std::cout << "\nLID-DRIVEN CAVITY C++ SOLVER\n";
        std::cout << "Mode: " << mode << "\n";
        std::cout << "Total simulations: " << nCases << "\n";
        std::cout << "Summary: " << summary_path.string() << "\n\n";

        int case_id = 0;
        for (int N : cfg.meshes) {
            for (int Re : cfg.re_list) {
                for (const auto& scheme : cfg.schemes) {
                    for (const auto& pressure_solver : cfg.pressure_solvers) {
                        for (const auto& implementation : cfg.implementations) {
                            ++case_id;
                            std::ostringstream name;
                            name << "case_" << std::setw(3) << std::setfill('0') << case_id << std::setfill(' ')
                                 << "_N" << N << "_Re" << Re << "_" << lower(scheme)
                                 << "_" << upper(pressure_solver) << "_" << lower(implementation);
                            const std::string case_name = name.str();

                            std::cout << "[" << std::setw(3) << std::setfill('0') << case_id << std::setfill(' ')
                                      << "] N=" << N << " Re=" << Re << " Scheme=" << scheme
                                      << " Pressure=" << pressure_solver << " Implementation=" << implementation << "\n";

                            Result r = solve_lid_cavity(N, Re, scheme, pressure_solver, implementation, cfg);
                            Metrics metrics = validate_against_ghia(r, cfg);
                            const std::string quality = quality_label(r, metrics);
                            write_summary_row(summary, case_id, r, metrics, quality);
                            summary.flush();
                            write_history_csv(r, case_name, cfg);
                            if (cfg.save_fields) write_field_csv(r, case_name, cfg);

                            std::cout << "      status=" << r.status << " quality=" << quality
                                      << " iter=" << r.iterations << "/" << r.localMaxIter
                                      << " Rc_mass=" << std::scientific << std::setprecision(3) << r.final_Rc_mass
                                      << " Rc_div=" << r.final_Rc_div
                                      << " runtime=" << std::fixed << std::setprecision(2) << r.runtime << "s"
                                      << " avgPiter=" << std::setprecision(1) << r.avg_poisson_iters
                                      << " pSat=" << std::setprecision(2) << r.pressure_saturation_ratio << "\n";
                            if (metrics.available) {
                                std::cout << "      Ghia L2: u=" << std::scientific << std::setprecision(3) << metrics.u_L2
                                          << "(limit " << metrics.u_limit << "), v=" << metrics.v_L2
                                          << "(limit " << metrics.v_limit << "), pass=" << (metrics.pass ? 1 : 0) << "\n";
                            }
                            std::cout << std::defaultfloat;
                        }
                    }
                }
            }
        }

        std::cout << "\nFinished. CSV outputs are in " << cfg.data_dir << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        print_usage(argv[0]);
        return 1;
    }
}
