#include "MechanicsSim.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace vfep {
namespace mech {

std::uint64_t now_ms() {
    using namespace std::chrono;
    return (std::uint64_t)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

static inline double clampd(double v, double lo, double hi) { return std::max(lo, std::min(hi, v)); }

static void update_interlocks(vfep::obj::ObjectStore& s) {
    // Minimal: allow by default unless explicit conditions exist later.
    // If a future build introduces door/ups/auth zones, populate reasons here.
    for (auto& [id, il] : s.interlocks) {
        il.allow_arm = true;
        il.allow_suppress = true;
        il.reasons.clear();
        il.updated_ms = now_ms();
    }
}

static bool interlocks_allow_suppress(const vfep::obj::ObjectStore& s, const std::string& vfep_id) {
    for (const auto& [id, il] : s.interlocks) {
        if (il.vfep_id == vfep_id) return il.allow_suppress;
    }
    return true;
}

static void tick_arms(vfep::obj::ObjectStore& s, double dt) {
    for (auto& [arm_id, at] : s.arm_telemetry) {
        const auto it_cfg = s.arms.find(arm_id);
        if (it_cfg == s.arms.end()) continue;
        const auto& cfg = it_cfg->second;

        if (!at.has_target) {
            // return to parking if no explicit target
            at.target_s_0_1 = cfg.parking_s_0_1;
        }

        const double target = clampd(at.target_s_0_1, cfg.travel_s_min_0_1, cfg.travel_s_max_0_1);
        const double pos = clampd(at.s_0_1, cfg.travel_s_min_0_1, cfg.travel_s_max_0_1);
        const double err = target - pos;
        const double dir = (err >= 0.0) ? 1.0 : -1.0;

        const double vmax = std::max(0.0, cfg.max_v_s_0_1_per_s);
        const double amax = std::max(0.0, cfg.max_a_s_0_1_per_s2);

        // simple trapezoid (very approximate): accelerate toward target then brake near it
        double v = at.v_s_0_1_per_s;
        const double dist = std::abs(err);
        const double brake_dist = (amax > 1e-9) ? (v*v) / (2.0*amax) : 0.0;
        const bool should_brake = dist <= brake_dist + 1e-6;

        const double a_cmd = should_brake ? (-dir * amax) : (dir * amax);
        v += a_cmd * dt;
        v = clampd(v, -vmax, vmax);

        double new_pos = pos + v * dt;

        // snap if crossing target
        if ((dir > 0.0 && new_pos >= target) || (dir < 0.0 && new_pos <= target)) {
            new_pos = target;
            v = 0.0;
        }

        at.s_0_1 = clampd(new_pos, cfg.travel_s_min_0_1, cfg.travel_s_max_0_1);
        at.v_s_0_1_per_s = v;

        // coarse state
        if (std::abs(target - at.s_0_1) < 1e-6) {
            if (at.state == vfep::obj::ArmDeploymentState::Moving) at.state = vfep::obj::ArmDeploymentState::Aiming;
        } else {
            at.state = vfep::obj::ArmDeploymentState::Moving;
        }
    }
}

static void tick_nozzles(vfep::obj::ObjectStore& s, double dt, const MechanicsParams& p) {
    for (auto& [noz_id, nt] : s.nozzle_telemetry) {
        const auto it_cfg = s.nozzles.find(noz_id);
        if (it_cfg == s.nozzles.end()) continue;
        const auto& cfg = it_cfg->second;

        const double pan_tgt = clampd(nt.target_pan_deg, cfg.pan_min_deg, cfg.pan_max_deg);
        const double tilt_tgt = clampd(nt.target_tilt_deg, cfg.tilt_min_deg, cfg.tilt_max_deg);

        const double max_step = std::max(0.0, p.nozzle_slew_deg_per_s) * dt;

        auto stepToward = [&](double cur, double tgt) -> double {
            const double e = tgt - cur;
            if (std::abs(e) <= max_step) return tgt;
            return cur + (e > 0.0 ? max_step : -max_step);
        };

        nt.pan_deg = stepToward(nt.pan_deg, pan_tgt);
        nt.tilt_deg = stepToward(nt.tilt_deg, tilt_tgt);
        nt.has_target = true;
    }
}

static void tick_suppression(vfep::obj::ObjectStore& s, double sim_time_s, double dt, const MechanicsParams& p) {
    // For MVP, assume single VFEP with single tank and affects selected rack (or hottest rack if none).
    for (const auto& [vfep_id, vfep] : s.vfeps) {
        bool active = vfep.suppression_active;
        if (!active) continue;
        if (!interlocks_allow_suppress(s, vfep_id)) continue;

        // pick tank: first tank belonging to vfep
        vfep::obj::TankTelemetry* tt = nullptr;
        vfep::obj::TankConfig const* tc = nullptr;
        for (auto& [tid, cfg] : s.tanks) {
            if (cfg.vfep_id == vfep_id) {
                tc = &cfg;
                tt = &s.tank_telemetry[tid];
                break;
            }
        }
        if (!tt || !tc) continue;

        // pick rack: selected if valid else find a burning rack
        std::string rack_id = vfep.selected_rack_id;
        if (rack_id.empty() || s.rack_telemetry.find(rack_id) == s.rack_telemetry.end()) {
            for (const auto& rid : vfep.coverage_rack_ids) {
                auto it = s.rack_telemetry.find(rid);
                if (it != s.rack_telemetry.end() && it->second.is_on_fire) { rack_id = rid; break; }
            }
        }
        if (rack_id.empty()) continue;

        auto& rtel = s.rack_telemetry[rack_id];

        // flow model (simple)
        const double flow = 0.5; // kg/s baseline MVP
        if (tt->is_depleted || tt->remaining_agent_mass_kg <= 0.0) {
            tt->is_depleted = true;
            tt->current_flow_kg_s = 0.0;
            continue;
        }

        const double used = flow * dt;
        tt->remaining_agent_mass_kg = std::max(0.0, tt->remaining_agent_mass_kg - used);
        tt->current_flow_kg_s = flow;
        tt->is_depleted = (tt->remaining_agent_mass_kg <= 1e-6);
        tt->regulator_bar = tc->regulator_setpoint_bar;
        // approximate pressure decay with remaining fraction
        const double frac = (tc->initial_agent_mass_kg > 1e-6) ? (tt->remaining_agent_mass_kg / tc->initial_agent_mass_kg) : 0.0;
        tt->current_pressure_bar = tc->regulator_setpoint_bar * clampd(frac, 0.0, 1.0);
        tt->valve_state = "discharging";

        // cool rack
        rtel.surface_temp_C = std::max(20.0, rtel.surface_temp_C - p.cooling_degC_per_s * dt);
        rtel.risk_to_assets_pct = std::max(0.0, rtel.risk_to_assets_pct - p.risk_reduction_pct_per_s * dt);
        if (rtel.surface_temp_C <= 40.0) rtel.is_on_fire = false;

        // Update incident state if present
        for (auto& [inc_id, inc] : s.incidents) {
            if (inc.rack_id == rack_id && inc.state != vfep::obj::IncidentState::Resolved) {
                inc.state = rtel.is_on_fire ? vfep::obj::IncidentState::Suppressing : vfep::obj::IncidentState::Resolved;
                if (inc.state == vfep::obj::IncidentState::Resolved) inc.resolved_at_s = sim_time_s;
            }
        }
    }
}

static void tick_incidents(vfep::obj::ObjectStore& s, double sim_time_s) {
    // create incidents for burning racks if absent
    for (const auto& [rack_id, rt] : s.rack_telemetry) {
        if (!rt.is_on_fire) continue;
        bool found = false;
        for (const auto& [inc_id, inc] : s.incidents) {
            if (inc.rack_id == rack_id && inc.state != vfep::obj::IncidentState::Resolved) { found = true; break; }
        }
        if (!found) {
            vfep::obj::Incident inc;
            inc.incident_id = "inc-" + rack_id + "-" + std::to_string((long long)(sim_time_s*1000.0));
            // best-effort room lookup
            auto itRack = s.racks.find(rack_id);
            inc.room_id = (itRack != s.racks.end()) ? itRack->second.room_id : "";
            inc.rack_id = rack_id;
            inc.state = vfep::obj::IncidentState::Active;
            inc.started_at_s = sim_time_s;
            inc.tags = {"auto"};
            s.upsert(inc);
        }
    }
}

void tick(vfep::obj::ObjectStore& store, double sim_time_s, double dt, const MechanicsParams& p) {
    update_interlocks(store);
    tick_incidents(store, sim_time_s);
    tick_arms(store, dt);
    tick_nozzles(store, dt, p);
    tick_suppression(store, sim_time_s, dt, p);
}

} // namespace mech
} // namespace vfep
