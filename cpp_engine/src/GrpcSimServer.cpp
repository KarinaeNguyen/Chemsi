#include "GrpcSimServer.h"

#ifdef CHEMSI_ENABLE_GRPC

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include "vfep_sim_service_v1.grpc.pb.h"

namespace vfep {
namespace grpcsim {

using chemsi::vfep::v1::VFEPUnitySimServiceV1;
using chemsi::vfep::v1::EmptyV1;
using chemsi::vfep::v1::WorldSnapshotV1;
using chemsi::vfep::v1::TelemetryFrameV1;
using chemsi::vfep::v1::CommandV1;
using chemsi::vfep::v1::CommandAckV1;

static chemsi::vfep::v1::VFEPStatusV1 mapStatus(vfep::obj::VFEPStatus s) {
    using V = chemsi::vfep::v1::VFEPStatusV1;
    switch (s) {
    case vfep::obj::VFEPStatus::Normal: return V::VFEP_STATUS_NORMAL;
    case vfep::obj::VFEPStatus::Armed: return V::VFEP_STATUS_ARMED;
    case vfep::obj::VFEPStatus::Maintenance: return V::VFEP_STATUS_MAINTENANCE;
    case vfep::obj::VFEPStatus::Fault: return V::VFEP_STATUS_FAULT;
    case vfep::obj::VFEPStatus::Offline: return V::VFEP_STATUS_OFFLINE;
    default: return V::VFEP_STATUS_NORMAL;
    }
}

static chemsi::vfep::v1::ArmStateV1 mapArmState(vfep::obj::ArmDeploymentState s) {
    using V = chemsi::vfep::v1::ArmStateV1;
    switch (s) {
    case vfep::obj::ArmDeploymentState::Stowed: return V::ARM_STATE_STOWED;
    case vfep::obj::ArmDeploymentState::Moving: return V::ARM_STATE_MOVING;
    case vfep::obj::ArmDeploymentState::Aiming: return V::ARM_STATE_AIMING;
    case vfep::obj::ArmDeploymentState::Firing: return V::ARM_STATE_FIRING;
    case vfep::obj::ArmDeploymentState::Returning: return V::ARM_STATE_RETURNING;
    case vfep::obj::ArmDeploymentState::Fault: return V::ARM_STATE_FAULT;
    default: return V::ARM_STATE_STOWED;
    }
}

static chemsi::vfep::v1::IncidentStateV1 mapIncidentState(vfep::obj::IncidentState s) {
    using V = chemsi::vfep::v1::IncidentStateV1;
    switch (s) {
    case vfep::obj::IncidentState::None: return V::INCIDENT_NONE;
    case vfep::obj::IncidentState::Active: return V::INCIDENT_ACTIVE;
    case vfep::obj::IncidentState::Suppressing: return V::INCIDENT_SUPPRESSING;
    case vfep::obj::IncidentState::Resolved: return V::INCIDENT_RESOLVED;
    default: return V::INCIDENT_NONE;
    }
}

static chemsi::vfep::v1::AlertSeverityV1 mapAlertSeverity(vfep::obj::AlertSeverity s) {
    using V = chemsi::vfep::v1::AlertSeverityV1;
    switch (s) {
    case vfep::obj::AlertSeverity::Info: return V::ALERT_INFO;
    case vfep::obj::AlertSeverity::Warning: return V::ALERT_WARNING;
    case vfep::obj::AlertSeverity::Critical: return V::ALERT_CRITICAL;
    default: return V::ALERT_INFO;
    }
}

namespace detail {

// C++17 detection idiom
template <class...>
using void_t = void;

// ---- row_index variants ----
template <class T, class = void>
struct has_row_index : std::false_type {};
template <class T>
struct has_row_index<T, void_t<decltype(std::declval<const T&>().row_index)>> : std::true_type {};

template <class T, class = void>
struct has_row : std::false_type {};
template <class T>
struct has_row<T, void_t<decltype(std::declval<const T&>().row)>> : std::true_type {};

template <class T, class = void>
struct has_row_idx : std::false_type {};
template <class T>
struct has_row_idx<T, void_t<decltype(std::declval<const T&>().row_idx)>> : std::true_type {};

// ---- col_index variants ----
template <class T, class = void>
struct has_col_index : std::false_type {};
template <class T>
struct has_col_index<T, void_t<decltype(std::declval<const T&>().col_index)>> : std::true_type {};

template <class T, class = void>
struct has_col : std::false_type {};
template <class T>
struct has_col<T, void_t<decltype(std::declval<const T&>().col)>> : std::true_type {};

template <class T, class = void>
struct has_col_idx : std::false_type {};
template <class T>
struct has_col_idx<T, void_t<decltype(std::declval<const T&>().col_idx)>> : std::true_type {};

// ---- position variants ----
template <class T, class = void>
struct has_position_mm_xyz_mm : std::false_type {};
template <class T>
struct has_position_mm_xyz_mm<T, void_t<
    decltype(std::declval<const T&>().position_mm.x_mm),
    decltype(std::declval<const T&>().position_mm.y_mm),
    decltype(std::declval<const T&>().position_mm.z_mm)
>> : std::true_type {};

template <class T, class = void>
struct has_position_xyz_mm : std::false_type {};
template <class T>
struct has_position_xyz_mm<T, void_t<
    decltype(std::declval<const T&>().position.x_mm),
    decltype(std::declval<const T&>().position.y_mm),
    decltype(std::declval<const T&>().position.z_mm)
>> : std::true_type {};

template <class T, class = void>
struct has_pos_mm_xyz_mm : std::false_type {};
template <class T>
struct has_pos_mm_xyz_mm<T, void_t<
    decltype(std::declval<const T&>().pos_mm.x_mm),
    decltype(std::declval<const T&>().pos_mm.y_mm),
    decltype(std::declval<const T&>().pos_mm.z_mm)
>> : std::true_type {};

// Fallback: plain x/y/z (assumed already in mm)
template <class T, class = void>
struct has_position_xyz : std::false_type {};
template <class T>
struct has_position_xyz<T, void_t<
    decltype(std::declval<const T&>().position.x),
    decltype(std::declval<const T&>().position.y),
    decltype(std::declval<const T&>().position.z)
>> : std::true_type {};

template <class T, class = void>
struct has_pos_xyz : std::false_type {};
template <class T>
struct has_pos_xyz<T, void_t<
    decltype(std::declval<const T&>().pos.x),
    decltype(std::declval<const T&>().pos.y),
    decltype(std::declval<const T&>().pos.z)
>> : std::true_type {};

template <class Rack>
int get_row_index(const Rack& rack) {
    if constexpr (has_row_index<Rack>::value) {
        return static_cast<int>(rack.row_index);
    } else if constexpr (has_row<Rack>::value) {
        return static_cast<int>(rack.row);
    } else if constexpr (has_row_idx<Rack>::value) {
        return static_cast<int>(rack.row_idx);
    } else {
        // Long-run safe default: keep schema stable even if model doesn't have rows.
        return 0;
    }
}

template <class Rack>
int get_col_index(const Rack& rack) {
    if constexpr (has_col_index<Rack>::value) {
        return static_cast<int>(rack.col_index);
    } else if constexpr (has_col<Rack>::value) {
        return static_cast<int>(rack.col);
    } else if constexpr (has_col_idx<Rack>::value) {
        return static_cast<int>(rack.col_idx);
    } else {
        return 0;
    }
}

template <class Rack>
double get_x_mm(const Rack& rack) {
    if constexpr (has_position_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position_mm.x_mm);
    } else if constexpr (has_position_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position.x_mm);
    } else if constexpr (has_pos_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.pos_mm.x_mm);
    } else if constexpr (has_position_xyz<Rack>::value) {
        return static_cast<double>(rack.position.x);
    } else if constexpr (has_pos_xyz<Rack>::value) {
        return static_cast<double>(rack.pos.x);
    } else {
        return 0.0;
    }
}

template <class Rack>
double get_y_mm(const Rack& rack) {
    if constexpr (has_position_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position_mm.y_mm);
    } else if constexpr (has_position_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position.y_mm);
    } else if constexpr (has_pos_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.pos_mm.y_mm);
    } else if constexpr (has_position_xyz<Rack>::value) {
        return static_cast<double>(rack.position.y);
    } else if constexpr (has_pos_xyz<Rack>::value) {
        return static_cast<double>(rack.pos.y);
    } else {
        return 0.0;
    }
}

template <class Rack>
double get_z_mm(const Rack& rack) {
    if constexpr (has_position_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position_mm.z_mm);
    } else if constexpr (has_position_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.position.z_mm);
    } else if constexpr (has_pos_mm_xyz_mm<Rack>::value) {
        return static_cast<double>(rack.pos_mm.z_mm);
    } else if constexpr (has_position_xyz<Rack>::value) {
        return static_cast<double>(rack.position.z);
    } else if constexpr (has_pos_xyz<Rack>::value) {
        return static_cast<double>(rack.pos.z);
    } else {
        return 0.0;
    }
}

} // namespace detail

class ServiceImpl final : public VFEPUnitySimServiceV1::Service {
public:
    ServiceImpl(vfep::obj::ObjectStore& store, std::mutex& mu, std::atomic<bool>& stop_flag, double& sim_time_s)
        : store_(store), mu_(mu), stop_flag_(stop_flag), sim_time_s_(sim_time_s) {}

    grpc::Status GetWorldSnapshot(grpc::ServerContext*, const EmptyV1*, WorldSnapshotV1* out) override {
        std::lock_guard<std::mutex> lk(mu_);
        out->set_schema_version("1.0");

        for (const auto& [id, room] : store_.rooms) {
            auto* r = out->add_rooms();
            r->set_room_id(room.room_id);
            r->set_name(room.name);
            r->set_floor_number(room.floor_number);
            r->set_security_level(room.security_level);
        }

        for (const auto& [id, rack] : store_.racks) {
            auto* r = out->add_racks();
            r->set_rack_id(rack.rack_id);
            r->set_room_id(rack.room_id);

            // Compatibility: older/newer RackConfig layouts
            r->set_row_index(detail::get_row_index(rack));
            r->set_col_index(detail::get_col_index(rack));

            r->set_height_u(rack.height_u);

            auto* p = r->mutable_position_mm();
            p->set_x_mm(detail::get_x_mm(rack));
            p->set_y_mm(detail::get_y_mm(rack));
            p->set_z_mm(detail::get_z_mm(rack));
        }

        for (const auto& [id, vfep] : store_.vfeps) {
            auto* v = out->add_vfeps();
            v->set_vfep_id(vfep.vfep_id);
            v->set_room_id(vfep.room_id);
            v->set_status(mapStatus(vfep.status));
            v->set_mounting_type(vfep.mounting_type);
            v->set_firmware_version(vfep.firmware_version);
            for (const auto& rid : vfep.coverage_rack_ids) v->add_coverage_rack_ids(rid);
        }

        for (const auto& [id, rail] : store_.rails) {
            auto* r = out->add_rails();
            r->set_rail_id(rail.rail_id);
            r->set_vfep_id(rail.vfep_id);
            r->set_label(rail.label);
            r->set_mount_height_mm(rail.mount_height_mm);
            for (const auto& pt : rail.points_xy_mm) {
                auto* p = r->add_points_xy_mm();
                p->set_x_mm(pt.x_mm);
                p->set_y_mm(pt.y_mm);
            }
            for (const auto& rid : rail.related_rack_ids) r->add_related_rack_ids(rid);
        }

        for (const auto& [id, arm] : store_.arms) {
            auto* a = out->add_arms();
            a->set_arm_id(arm.arm_id);
            a->set_vfep_id(arm.vfep_id);
            a->set_rail_id(arm.rail_id);
            a->set_travel_s_min_0_1(arm.travel_s_min_0_1);
            a->set_travel_s_max_0_1(arm.travel_s_max_0_1);
            a->set_parking_s_0_1(arm.parking_s_0_1);
            a->set_max_v_s_0_1_per_s(arm.max_v_s_0_1_per_s);
            a->set_max_a_s_0_1_per_s2(arm.max_a_s_0_1_per_s2);
        }

        for (const auto& [id, noz] : store_.nozzles) {
            auto* n = out->add_nozzles();
            n->set_nozzle_id(noz.nozzle_id);
            n->set_arm_id(noz.arm_id);
            n->set_pan_min_deg(noz.pan_min_deg);
            n->set_pan_max_deg(noz.pan_max_deg);
            n->set_tilt_min_deg(noz.tilt_min_deg);
            n->set_tilt_max_deg(noz.tilt_max_deg);
            n->set_flow_rate_kg_s(noz.flow_rate_kg_s);
        }

        for (const auto& [id, tank] : store_.tanks) {
            auto* t = out->add_tanks();
            t->set_tank_id(tank.tank_id);
            t->set_vfep_id(tank.vfep_id);
            t->set_rail_id(tank.rail_id);
            t->set_capacity_l(tank.capacity_L);
            t->set_initial_agent_mass_kg(tank.initial_agent_mass_kg);
            t->set_regulator_setpoint_bar(tank.regulator_setpoint_bar);
        }

        return grpc::Status::OK;
    }

    grpc::Status StreamTelemetry(grpc::ServerContext* ctx, const EmptyV1*, grpc::ServerWriter<TelemetryFrameV1>* writer) override {
        while (!ctx->IsCancelled() && !stop_flag_.load()) {
            TelemetryFrameV1 frame;
            {
                std::lock_guard<std::mutex> lk(mu_);
                frame.set_schema_version("1.0");
                frame.set_sim_time_s(sim_time_s_);

                for (const auto& [rack_id, rt] : store_.rack_telemetry) {
                    auto* r = frame.add_racks();
                    r->set_rack_id(rack_id);
                    r->set_is_on_fire(rt.is_on_fire);
                    r->set_surface_temp_c(rt.surface_temp_C);
                    r->set_risk_to_assets_pct(rt.risk_to_assets_pct);
                }
                for (const auto& [tank_id, tt] : store_.tank_telemetry) {
                    auto* t = frame.add_tanks();
                    t->set_tank_id(tank_id);
                    t->set_current_pressure_bar(tt.current_pressure_bar);
                    t->set_regulator_bar(tt.regulator_bar);
                    t->set_remaining_agent_mass_kg(tt.remaining_agent_mass_kg);
                    t->set_current_flow_kg_s(tt.current_flow_kg_s);
                    t->set_is_depleted(tt.is_depleted);
                    t->set_valve_state(tt.valve_state);
                }
                for (const auto& [arm_id, at] : store_.arm_telemetry) {
                    auto* a = frame.add_arms();
                    a->set_arm_id(arm_id);
                    a->set_state(mapArmState(at.state));
                    a->set_s_0_1(at.s_0_1);
                    a->set_v_s_0_1_per_s(at.v_s_0_1_per_s);
                    a->set_target_s_0_1(at.target_s_0_1);
                    a->set_has_target(at.has_target);
                    a->set_interlock_active(at.interlock_active);
                    a->set_fault_code(at.fault_code);
                }
                for (const auto& [noz_id, nt] : store_.nozzle_telemetry) {
                    auto* n = frame.add_nozzles();
                    n->set_nozzle_id(noz_id);
                    n->set_clogged(nt.clogged);
                    n->set_pan_deg(nt.pan_deg);
                    n->set_tilt_deg(nt.tilt_deg);
                    n->set_target_pan_deg(nt.target_pan_deg);
                    n->set_target_tilt_deg(nt.target_tilt_deg);
                    n->set_has_target(nt.has_target);
                }
                for (const auto& [iid, il] : store_.interlocks) {
                    auto* i = frame.add_interlocks();
                    i->set_interlock_id(iid);
                    i->set_vfep_id(il.vfep_id);
                    i->set_allow_arm(il.allow_arm);
                    i->set_allow_suppress(il.allow_suppress);
                    for (const auto& reason : il.reasons) i->add_reasons(reason);
                    i->set_updated_ms(il.updated_ms);
                }
                for (const auto& [inc_id, inc] : store_.incidents) {
                    auto* i = frame.add_incidents();
                    i->set_incident_id(inc_id);
                    i->set_room_id(inc.room_id);
                    i->set_rack_id(inc.rack_id);
                    i->set_state(mapIncidentState(inc.state));
                    i->set_started_at_s(inc.started_at_s);
                    i->set_resolved_at_s(inc.resolved_at_s);
                    for (const auto& tag : inc.tags) i->add_tags(tag);
                }
                for (const auto& [al_id, al] : store_.alerts) {
                    auto* a = frame.add_alerts();
                    a->set_alert_id(al_id);
                    a->set_room_id(al.room_id);
                    a->set_rack_id(al.rack_id);
                    a->set_severity(mapAlertSeverity(al.severity));
                    a->set_code(al.code);
                    a->set_message(al.message);
                    a->set_created_ms(al.created_ms);
                    a->set_acknowledged(al.acknowledged);
                }
                for (const auto& [vfep_id, vfep] : store_.vfeps) {
                    auto* v = frame.add_vfeps();
                    v->set_vfep_id(vfep_id);
                    v->set_status(mapStatus(vfep.status));
                    v->set_selected_rack_id(vfep.selected_rack_id);
                    v->set_selected_hotspot_u(vfep.selected_hotspot_u);
                    v->set_suppression_active(vfep.suppression_active);
                }
            }

            if (!writer->Write(frame)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return grpc::Status::OK;
    }

    grpc::Status SendCommand(grpc::ServerContext*, const CommandV1* cmd, CommandAckV1* ack) override {
        std::lock_guard<std::mutex> lk(mu_);
        const std::uint64_t ts = cmd->client_timestamp_ms();

        auto ok = [&](const std::string& msg) {
            ack->set_ok(true);
            ack->set_message(msg);
            return grpc::Status::OK;
        };
        auto bad = [&](const std::string& msg) {
            ack->set_ok(false);
            ack->set_message(msg);
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, msg);
        };

        if (cmd->has_set_armed()) {
            const auto& c = cmd->set_armed();
            auto it = store_.vfeps.find(c.vfep_id());
            if (it == store_.vfeps.end()) return bad("Unknown vfep_id");
            it->second.status = c.armed() ? vfep::obj::VFEPStatus::Armed : vfep::obj::VFEPStatus::Normal;
            return ok("set_armed applied");
        }
        if (cmd->has_select_target()) {
            const auto& c = cmd->select_target();
            auto it = store_.vfeps.find(c.vfep_id());
            if (it == store_.vfeps.end()) return bad("Unknown vfep_id");
            it->second.selected_rack_id = c.rack_id();
            it->second.selected_hotspot_u = c.hotspot_u();
            return ok("select_target applied");
        }
        if (cmd->has_start_suppression()) {
            const auto& c = cmd->start_suppression();
            auto it = store_.vfeps.find(c.vfep_id());
            if (it == store_.vfeps.end()) return bad("Unknown vfep_id");
            it->second.suppression_active = true;
            return ok("start_suppression applied");
        }
        if (cmd->has_stop_suppression()) {
            const auto& c = cmd->stop_suppression();
            auto it = store_.vfeps.find(c.vfep_id());
            if (it == store_.vfeps.end()) return bad("Unknown vfep_id");
            it->second.suppression_active = false;
            return ok("stop_suppression applied");
        }
        if (cmd->has_manual_aim()) {
            const auto& c = cmd->manual_aim();
            auto it = store_.nozzle_telemetry.find(c.nozzle_id());
            if (it == store_.nozzle_telemetry.end()) return bad("Unknown nozzle_id");
            it->second.target_pan_deg = c.pan_deg();
            it->second.target_tilt_deg = c.tilt_deg();
            it->second.has_target = true;
            it->second.last_command_ms = ts;
            it->second.last_command_source = "grpc";
            return ok("manual_aim applied");
        }
        if (cmd->has_move_arm()) {
            const auto& c = cmd->move_arm();
            auto it = store_.arm_telemetry.find(c.arm_id());
            if (it == store_.arm_telemetry.end()) return bad("Unknown arm_id");
            it->second.target_s_0_1 = c.target_s_0_1();
            it->second.has_target = true;
            it->second.last_command_ms = ts;
            it->second.last_command_source = "grpc";
            return ok("move_arm applied");
        }
        if (cmd->has_reset()) {
            const auto& c = cmd->reset();
            auto it = store_.vfeps.find(c.vfep_id());
            if (it == store_.vfeps.end()) return bad("Unknown vfep_id");
            it->second.selected_rack_id.clear();
            it->second.selected_hotspot_u = 0;
            it->second.suppression_active = false;
            // reset tanks
            for (auto& [tid, cfg] : store_.tanks) {
                if (cfg.vfep_id != c.vfep_id()) continue;
                auto& tt = store_.tank_telemetry[tid];
                tt.remaining_agent_mass_kg = cfg.initial_agent_mass_kg;
                tt.current_pressure_bar = cfg.regulator_setpoint_bar;
                tt.regulator_bar = cfg.regulator_setpoint_bar;
                tt.current_flow_kg_s = 0.0;
                tt.is_depleted = false;
                tt.valve_state = "online";
            }
            return ok("reset applied");
        }

        return bad("No command set");
    }

private:
    vfep::obj::ObjectStore& store_;
    std::mutex& mu_;
    std::atomic<bool>& stop_flag_;
    double& sim_time_s_;
};

struct GrpcSimServer::Impl {
    vfep::obj::ObjectStore store;
    vfep::mech::MechanicsParams params;
    std::mutex mu;
    std::atomic<bool> stop_flag{false};
    double sim_time_s = 0.0;

    std::unique_ptr<grpc::Server> server;
    std::thread sim_thread;

    void simLoop(int tick_hz) {
        const double dt = (tick_hz > 0) ? (1.0 / static_cast<double>(tick_hz)) : 0.05;
        auto next = std::chrono::steady_clock::now();
        while (!stop_flag.load()) {
            next += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(dt));
            {
                std::lock_guard<std::mutex> lk(mu);
                vfep::mech::tick(store, sim_time_s, dt, params);
                sim_time_s += dt;
            }
            std::this_thread::sleep_until(next);
        }
    }
};

GrpcSimServer::GrpcSimServer() : impl_(new Impl()) {
    impl_->store = vfep::obj::makeDefault4x4ObjectStore();
}

GrpcSimServer::~GrpcSimServer() {
    Stop();
}

bool GrpcSimServer::Run(const std::string& bind_addr, int port, int tick_hz) {
    const std::string addr = bind_addr + ":" + std::to_string(port);

    ServiceImpl service(impl_->store, impl_->mu, impl_->stop_flag, impl_->sim_time_s);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    impl_->server = builder.BuildAndStart();
    if (!impl_->server) {
        std::cerr << "ERROR: Could not start gRPC server on " << addr << "\n";
        return false;
    }

    std::cout << "[gRPC] VFEPUnitySimServiceV1 listening on " << addr << " tick_hz=" << tick_hz << "\n";
    impl_->stop_flag.store(false);
    impl_->sim_thread = std::thread([this, tick_hz]() { impl_->simLoop(tick_hz); });

    impl_->server->Wait(); // blocks
    return true;
}

void GrpcSimServer::Stop() {
    if (!impl_) return;
    if (impl_->stop_flag.exchange(true)) return;
    if (impl_->server) {
        impl_->server->Shutdown();
        impl_->server.reset();
    }
    if (impl_->sim_thread.joinable()) impl_->sim_thread.join();
}

} // namespace grpcsim
} // namespace vfep

#else // CHEMSI_ENABLE_GRPC

#include <iostream>

namespace vfep {
namespace grpcsim {

struct GrpcSimServer::Impl {};

GrpcSimServer::GrpcSimServer() : impl_(new Impl()) {}
GrpcSimServer::~GrpcSimServer() {}

bool GrpcSimServer::Run(const std::string&, int, int) {
    std::cerr << "CHEMSI_ENABLE_GRPC is disabled at build time.\n";
    return false;
}
void GrpcSimServer::Stop() {}

} // namespace grpcsim
} // namespace vfep

#endif // CHEMSI_ENABLE_GRPC
