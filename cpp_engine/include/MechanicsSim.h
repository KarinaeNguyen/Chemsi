#pragma once

#include "ObjectModel.h"
#include <cstdint>
#include <string>

namespace vfep {
namespace mech {

// Minimal deterministic mechanics update over vfep::obj::ObjectStore.
// Intended to be "good enough" for Unity integration v1, not a full physics model.
struct MechanicsParams {
    double nozzle_slew_deg_per_s = 90.0;   // simple slew
    double cooling_degC_per_s = 15.0;      // rack cooling when suppressing
    double risk_reduction_pct_per_s = 12.0;
};

// Returns current wallclock ms (helper for traceability fields).
std::uint64_t now_ms();

// Applies a single fixed timestep update in-place.
void tick(vfep::obj::ObjectStore& store, double sim_time_s, double dt, const MechanicsParams& p);

} // namespace mech
} // namespace vfep
