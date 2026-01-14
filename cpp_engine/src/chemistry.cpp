#include "Chemistry.h"
#include "Constants.h"

#include <algorithm>
#include <cmath>

namespace vfep {

namespace {
constexpr double kTiny = 1e-15;
constexpr double kMinTemp_K = 250.0;

// Post-ignition pilot/flame support.
//
// Rationale:
// - In this model, ignition is an external command (pilot/flame kernel). After ignition, we should
//   not require the *bulk* reactor temperature to already be high for chemistry to begin.
// - Some toolchains/FP modes can effectively flush extremely small Arrhenius rates to zero at
//   near-ambient temperatures, yielding HRR==0 despite fuel + O2 being present.
//
// This floor is applied ONLY when ignitionTempFloor_K > 0 (i.e., post-ignition). It is still
// constrained by available fuel and oxygen and produces heat only through the same chemistry pathway.
constexpr double kPilotRate_1_per_s = 0.50; // mixing-limited floor on fuel consumption rate

// inhibitor effect: exp(-k_inhib * conc)
constexpr double kInhibCoeff = 5.0; // 1/(kg/m^3) tune later

static bool isFinitePositive(double x) {
    return std::isfinite(x) && x > 0.0;
}
} // namespace

Chemistry::Chemistry(const std::vector<Species>& sp, ChemistryIndex idx, CombustionModel model)
: sp_(sp), idx_(idx), model_(model) {}

ReactionResult Chemistry::react(
    double dt,
    double T_K,
    double ignitionTempFloor_K,
    double V_m3,
    std::vector<double>& n_mol,
    double inhibitor_kg_per_m3
) {
    ReactionResult rr;

    // Basic guards
    if (!isFinitePositive(dt)) return rr;
    if (!isFinitePositive(V_m3)) return rr;
    if (!std::isfinite(T_K)) return rr;
    if (n_mol.empty()) return rr;

    const int iF   = idx_.iFUEL;
    const int iO2  = idx_.iO2;
    const int iCO2 = idx_.iCO2;
    const int iH2O = idx_.iH2O;

    if (iF < 0 || iO2 < 0 || iCO2 < 0 || iH2O < 0) return rr;
    if (iF >= static_cast<int>(n_mol.size())
        || iO2 >= static_cast<int>(n_mol.size())
        || iCO2 >= static_cast<int>(n_mol.size())
        || iH2O >= static_cast<int>(n_mol.size())) {
        return rr;
    }

    const double nFuel = std::max(0.0, n_mol[iF]);
    const double nO2   = std::max(0.0, n_mol[iO2]);
    if (nFuel <= kTiny || nO2 <= kTiny) return rr;

    // Concentrations (mol/m^3)
    const double cFuel = nFuel / V_m3;
    const double cO2   = nO2   / V_m3;

    // Arrhenius temperature clamp.
    // NOTE: ignitionTempFloor_K is a *kinetics* assist only. It does not modify the
    // reactor's bulk temperature; it simply prevents the Arrhenius term from
    // collapsing to ~0 at ambient when a pilot/flame is present (post-ignition).
    const double Tfloor = (std::isfinite(ignitionTempFloor_K) && ignitionTempFloor_K > 0.0)
        ? ignitionTempFloor_K
        : 0.0;
    const double Tuse = std::max(kMinTemp_K, std::max(T_K, Tfloor));
    // NOTE: Do not early-return on Arrhenius underflow when an ignition kernel is present.
    // Some toolchains/FP modes may flush very small Arrhenius rates to 0.0 in Release.
    // We allow kT==0 and rely on the post-ignition pilot rate floor (below) to
    // initiate combustion deterministically while still requiring fuel + O2.
    double kT = model_.A * std::exp(-model_.Ea / (R_universal * Tuse));
    if (!std::isfinite(kT) || kT < 0.0) kT = 0.0;

    const double inhib = std::max(0.0, inhibitor_kg_per_m3);
    const double inhibFactor = std::exp(-kInhibCoeff * inhib);

    // Rate of fuel consumption (mol/m^3/s)
    double rFuel = kT
        * std::pow(std::max(0.0, cFuel), model_.orderFuel)
        * std::pow(std::max(0.0, cO2),   model_.orderO2)
        * inhibFactor;

    // Post-ignition: ensure chemistry starts even if Arrhenius-controlled rate collapses at
    // near-ambient bulk temperatures on a given platform. This is a *kinetics* floor only.
    if (Tfloor > 0.0) {
        // Smooth oxygen dependence so the floor vanishes if O2 is depleted.
        const double o2Factor = cO2 / (cO2 + 1.0);
        const double rPilot = kPilotRate_1_per_s * cFuel * o2Factor * inhibFactor;
        if (std::isfinite(rPilot) && rPilot > 0.0) {
            rFuel = std::max(rFuel, rPilot);
        }
    }

    if (!std::isfinite(rFuel) || rFuel <= 0.0) return rr;

    // Stoichiometry
    const double nuO2  = std::max(0.0, model_.nuO2());
    const double nuCO2 = std::max(0.0, model_.nuCO2());
    const double nuH2O = std::max(0.0, model_.nuH2O());

    const double maxFuelByFuel = nFuel;
    const double maxFuelByO2   = (nuO2 > kTiny) ? (nO2 / nuO2) : 0.0;

    const double fuelToConsume_kin = rFuel * V_m3 * dt; // mol
    const double fuelToConsume =
        std::max(0.0, std::min({fuelToConsume_kin, maxFuelByFuel, maxFuelByO2}));

    if (fuelToConsume <= kTiny) return rr;

    const double o2Consumed = nuO2  * fuelToConsume;
    const double co2Formed  = nuCO2 * fuelToConsume;
    const double h2oFormed  = nuH2O * fuelToConsume;

    // Update moles with non-negativity guards
    n_mol[iF]   = std::max(0.0, n_mol[iF]  - fuelToConsume);
    n_mol[iO2]  = std::max(0.0, n_mol[iO2] - o2Consumed);
    n_mol[iCO2] = std::max(0.0, n_mol[iCO2] + co2Formed);
    n_mol[iH2O] = std::max(0.0, n_mol[iH2O] + h2oFormed);

    rr.dMolFuel = -fuelToConsume;
    rr.dMolO2   = -o2Consumed;
    rr.dMolCO2  = +co2Formed;
    rr.dMolH2O  = +h2oFormed;

    // Heat release rate (W). Positive means heat generation.
    const double Q_J = model_.heatRelease_J_per_molFuel * fuelToConsume;
    rr.heat_W = std::isfinite(Q_J) ? (Q_J / dt) : 0.0;

    return rr;
}

} // namespace vfep
