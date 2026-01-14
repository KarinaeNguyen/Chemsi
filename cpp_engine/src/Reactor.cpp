// src/Reactor.cpp
#include "Reactor.h"

#include "Constants.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace vfep {

namespace {
constexpr double kTiny      = 1e-15;
constexpr double kMinTemp_K = 1.0;     // hard floor
constexpr double kMaxTemp_K = 5000.0;  // safety cap

static bool isFinite(double x) {
    return std::isfinite(x);
}

static bool isFinitePositive(double x) {
    return std::isfinite(x) && x > 0.0;
}
} // namespace

Reactor::Reactor(std::vector<Species> sp, ChemistryIndex idx, CombustionModel model)
: cfg_(),
  sp_(std::move(sp)),
  idx_(idx),
  model_(model),
  chemistry_(sp_, idx_, model_),
  n_mol_(sp_.size(), 0.0),
  gasIdx_(),
  T_K_(cfg_.T_amb_K) {

    // Cache gas indices
    gasIdx_.reserve(sp_.size());
    for (int i = 0; i < static_cast<int>(sp_.size()); ++i) {
        if (sp_[i].phase == Phase::Gas) {
            gasIdx_.push_back(i);
        }
    }

    if (!isFinite(T_K_)) {
        T_K_ = 295.15;
    }
}

double Reactor::totalGasMoles() const noexcept {
    double sum = 0.0;
    for (int i : gasIdx_) {
        if (i < 0 || i >= static_cast<int>(n_mol_.size())) continue;
        const double ni = n_mol_[i];
        if (ni > 0.0 && isFinite(ni)) sum += ni;
    }
    return sum;
}

double Reactor::mixtureCp_J_per_K() const noexcept {
    // Cp_mix [J/K] = sum_i (n_i [mol] * cp_i [J/mol-K]) over gas species
    double Cp = 0.0;
    for (int i : gasIdx_) {
        if (i < 0 || i >= static_cast<int>(n_mol_.size())) continue;
        const double ni = n_mol_[i];
        if (!(ni > 0.0) || !isFinite(ni)) continue;

        const double cpi = sp_[i].cp_J_per_molK;
        if (!isFinite(cpi) || cpi <= 0.0) continue;

        Cp += ni * cpi;
    }
    return Cp;
}

double Reactor::gasMoleFraction(int i) const noexcept {
    if (i < 0 || i >= static_cast<int>(n_mol_.size())) return 0.0;
    if (sp_[i].phase != Phase::Gas) return 0.0;

    const double nTot = totalGasMoles();
    if (!(nTot > kTiny) || !isFinite(nTot)) return 0.0;

    const double ni = n_mol_[i];
    if (!(ni > 0.0) || !isFinite(ni)) return 0.0;

    const double y = ni / nTot;
    if (!isFinite(y)) return 0.0;
    return std::clamp(y, 0.0, 1.0);
}

void Reactor::addMoles(int i, double dn) noexcept {
    if (i < 0 || i >= static_cast<int>(n_mol_.size())) return;
    if (!isFinite(dn)) return;

    const double newVal = n_mol_[i] + dn;
    if (!isFinite(newVal) || newVal <= 0.0) {
        n_mol_[i] = 0.0;
    } else {
        n_mol_[i] = newVal;
    }
}

double Reactor::heatLoss_W() const noexcept {
    const double T    = T_K_;
    const double Tamb = cfg_.T_amb_K;

    if (!isFinite(T) || !isFinite(Tamb)) return 0.0;

    const double A = cfg_.area_m2;
    if (!isFinitePositive(A)) return 0.0;

    // Convection: Q = h A (T - Tamb)
    const double h = cfg_.h_W_m2K;
    double Qconv = 0.0;
    if (isFinitePositive(h)) {
        Qconv = h * A * (T - Tamb);
    }

    // Radiation: Q = eps sigma A (T^4 - Tamb^4)
    const double eps = std::clamp(cfg_.emissivity, 0.0, 1.0);
    double Qrad = 0.0;
    if (eps > 0.0) {
        const double T4  = std::pow(std::max(kMinTemp_K, T),    4.0);
        const double Ta4 = std::pow(std::max(kMinTemp_K, Tamb), 4.0);
        Qrad = eps * sigmaSB * A * (T4 - Ta4);
    }

    const double Qloss = Qconv + Qrad;
    if (!isFinite(Qloss)) return 0.0;

    // Positive means heat leaving the reactor (removal).
    return Qloss;
}

void Reactor::step(double dt,
                   double inhibitor_kg_per_m3,
                   double externalCooling_W,
                   double& outCombustionHRR_W) noexcept {
    // Backward-compatible wrapper: full coupling.
    step(dt, inhibitor_kg_per_m3, externalCooling_W, 1.0, outCombustionHRR_W, 0.0);
}

void Reactor::step(double dt,
                   double inhibitor_kg_per_m3,
                   double externalCooling_W,
                   double combustionHeatMultiplier_0_1,
                   double& outCombustionHRR_W) noexcept {
    step(dt, inhibitor_kg_per_m3, externalCooling_W, combustionHeatMultiplier_0_1, outCombustionHRR_W, 0.0);
}

void Reactor::step(double dt,
                   double inhibitor_kg_per_m3,
                   double externalCooling_W,
                   double combustionHeatMultiplier_0_1,
                   double& outCombustionHRR_W,
                   double ignitionTempFloor_K) noexcept {
    outCombustionHRR_W = 0.0;

    if (!isFinitePositive(dt)) return;

    // Ensure temperature is usable
    if (!isFinite(T_K_)) {
        T_K_ = isFinite(cfg_.T_amb_K) ? cfg_.T_amb_K : 295.15;
    }

    // Enforce n_mol_ size invariant defensively
    if (n_mol_.size() != sp_.size()) {
        n_mol_.assign(sp_.size(), 0.0);
        for (double& ni : n_mol_) {
            if (!isFinite(ni) || ni < 0.0) ni = 0.0;
        }
    }

    // -------------
    // Combustion chemistry: consumes FUEL + O2 -> CO2 + H2O and returns heat release rate.
    // This is the single source of truth for combustion HRR used by the Simulation gate.
    // -------------
    {
        const ReactionResult rr = chemistry_.react(dt, T_K_, ignitionTempFloor_K, cfg_.volume_m3, n_mol_, inhibitor_kg_per_m3);
        if (std::isfinite(rr.heat_W) && rr.heat_W > 0.0) {
            outCombustionHRR_W = rr.heat_W;
        } else {
            outCombustionHRR_W = 0.0;
        }
    }

    const double Cp = mixtureCp_J_per_K();
    if (!(Cp > 0.0) || !isFinite(Cp)) {
        // With no defined thermal inertia, clamp temperature and exit safely.
        T_K_ = std::clamp(T_K_, kMinTemp_K, kMaxTemp_K);
        // Still sanitize moles (visualizer safety)
        for (double& ni : n_mol_) {
            if (!isFinite(ni) || ni < 0.0) ni = 0.0;
        }
        return;
    }

    const double Qloss = heatLoss_W(); // positive removes heat
    const double Qext  = isFinite(externalCooling_W) ? externalCooling_W : 0.0;

    // Apply multiplier to thermal coupling only (keeps telemetry raw, but thermal state consistent with suppression).
    const double mult = std::clamp(combustionHeatMultiplier_0_1, 0.0, 1.0);

    // Net heat into sensible energy:
    // + outCombustionHRR_W adds heat (scaled by mult into the thermal state)
    // - Qloss removes heat
    // - Qext removes heat when positive; adds heat when negative (per convention)
    const double Qnet_W = (outCombustionHRR_W * mult) - Qloss - Qext;

    const double dT = (Qnet_W * dt) / Cp;
    if (isFinite(dT)) {
        T_K_ = std::clamp(T_K_ + dT, kMinTemp_K, kMaxTemp_K);
    } else {
        // Fail safe toward ambient if numerical failure occurs.
        const double Ta = isFinite(cfg_.T_amb_K) ? cfg_.T_amb_K : 295.15;
        T_K_ = std::clamp(Ta, kMinTemp_K, kMaxTemp_K);
    }

    // -------------
    // Final sanitization
    // -------------
    for (double& ni : n_mol_) {
        if (!isFinite(ni) || ni < 0.0) ni = 0.0;
    }
}

} // namespace vfep
