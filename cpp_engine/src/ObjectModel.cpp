#include "ObjectModel.h"

#include <algorithm>

namespace vfep {
namespace obj {

void ObjectStore::clear() {
    rooms.clear();
    racks.clear();
    vfeps.clear();
    sensors.clear();
    rails.clear();
    tanks.clear();
    arms.clear();
    nozzles.clear();
    magazines.clear();
    trigger_configs.clear();
    manual_controls.clear();
    vfe_round_types.clear();
    chemicals.clear();
    shells.clear();
    cameras.clear();
    arm_views.clear();
    datacenter_views.clear();
    maps.clear();


    interlocks.clear();
    suppression_plans.clear();
    incidents.clear();
    alerts.clear();
    room_telemetry.clear();
    rack_telemetry.clear();
    sensor_telemetry.clear();
    tank_telemetry.clear();
    arm_telemetry.clear();
    nozzle_telemetry.clear();
    magazine_telemetry.clear();
    camera_telemetry.clear();

    trigger_events.clear();
    deployment_events.clear();
    manual_action_events.clear();
    fault_events.clear();
    maintenance_events.clear();
}

static inline bool nonEmpty(const std::string& s) { return !s.empty(); }

void ObjectStore::upsert(const DataCenterRoomConfig& v) { rooms[v.room_id] = v; }
void ObjectStore::upsert(const RackConfig& v) { racks[v.rack_id] = v; }
void ObjectStore::upsert(const VFEPConfig& v) { vfeps[v.vfep_id] = v; }
void ObjectStore::upsert(const SensorConfig& v) { sensors[v.sensor_id] = v; }
void ObjectStore::upsert(const RailConfig& v) { rails[v.rail_id] = v; }
void ObjectStore::upsert(const TankConfig& v) { tanks[v.tank_id] = v; }
void ObjectStore::upsert(const ArmConfig& v) { arms[v.arm_id] = v; }
void ObjectStore::upsert(const NozzleConfig& v) { nozzles[v.nozzle_id] = v; }
void ObjectStore::upsert(const MagazineConfig& v) { magazines[v.magazine_id] = v; }
void ObjectStore::upsert(const TriggerConfig& v) { trigger_configs[v.trigger_config_id] = v; }
void ObjectStore::upsert(const ManualControlConfig& v) { manual_controls[v.manual_control_id] = v; }
void ObjectStore::upsert(const VFEConfig& v) { vfe_round_types[v.vfe_id] = v; }
void ObjectStore::upsert(const ChemicalConfig& v) { chemicals[v.chemical_id] = v; }
void ObjectStore::upsert(const ShellConfig& v) { shells[v.shell_id] = v; }
void ObjectStore::upsert(const CameraConfig& v) { cameras[v.camera_id] = v; }
void ObjectStore::upsert(const ArmViewConfig& v) { arm_views[v.camera_id] = v; }
void ObjectStore::upsert(const DataCenterViewConfig& v) { datacenter_views[v.camera_id] = v; }
void ObjectStore::upsert(const MapModelConfig& v) { maps[v.room_id] = v; }

void ObjectStore::upsert(const InterlockStatus& v) { interlocks[v.interlock_id] = v; }
void ObjectStore::upsert(const SuppressionPlan& v) { suppression_plans[v.plan_id] = v; }
void ObjectStore::upsert(const Incident& v) { incidents[v.incident_id] = v; }
void ObjectStore::upsert(const Alert& v) { alerts[v.alert_id] = v; }


ValidationReport ObjectStore::validate() const {
    ValidationReport r;

    auto err = [&](const std::string& type, const std::string& id, const std::string& msg) {
        r.issues.push_back({ValidationIssue::Severity::Error, type, id, msg});
    };
    auto warn = [&](const std::string& type, const std::string& id, const std::string& msg) {
        r.issues.push_back({ValidationIssue::Severity::Warning, type, id, msg});
    };

    // Rooms
    for (const auto& [id, room] : rooms) {
        if (!nonEmpty(id)) err("DataCenterRoom", id, "room_id must be non-empty.");
        if (!nonEmpty(room.name)) warn("DataCenterRoom", id, "name is empty.");
        if (room.number_of_racks < 0) err("DataCenterRoom", id, "number_of_racks must be >= 0.");
    }

    // Racks -> Room FK
    for (const auto& [id, rack] : racks) {
        if (!nonEmpty(id)) err("Rack", id, "rack_id must be non-empty.");
        if (!nonEmpty(rack.room_id)) err("Rack", id, "room_id must be set.");
        if (nonEmpty(rack.room_id) && rooms.find(rack.room_id) == rooms.end()) {
            err("Rack", id, "room_id does not exist: " + rack.room_id);
        }
        if (rack.width_mm <= 0.0 || rack.depth_mm <= 0.0) err("Rack", id, "width_mm and depth_mm must be > 0.");
        if (rack.height_u <= 0) err("Rack", id, "height_u must be > 0.");
    }

    // VFEP -> Room FK + Rack coverage FK
    for (const auto& [id, v] : vfeps) {
        if (!nonEmpty(id)) err("VFEP", id, "vfep_id must be non-empty.");
        if (!nonEmpty(v.room_id)) err("VFEP", id, "room_id must be set.");
        if (nonEmpty(v.room_id) && rooms.find(v.room_id) == rooms.end()) {
            err("VFEP", id, "room_id does not exist: " + v.room_id);
        }
        for (const auto& rid : v.coverage_rack_ids) {
            if (racks.find(rid) == racks.end()) err("VFEP", id, "coverage_rack_id does not exist: " + rid);
        }
    }

    // Sensor -> VFEP + optional Rack
    for (const auto& [id, s] : sensors) {
        if (!nonEmpty(id)) err("Sensor", id, "sensor_id must be non-empty.");
        if (!nonEmpty(s.vfep_id) || vfeps.find(s.vfep_id) == vfeps.end()) {
            err("Sensor", id, "vfep_id missing or does not exist: " + s.vfep_id);
        }
        if (nonEmpty(s.rack_id) && racks.find(s.rack_id) == racks.end()) {
            err("Sensor", id, "rack_id does not exist: " + s.rack_id);
        }
    }

    // Rail -> VFEP
    for (const auto& [id, rail] : rails) {
        if (!nonEmpty(id)) err("Rail", id, "rail_id must be non-empty.");
        if (!nonEmpty(rail.vfep_id) || vfeps.find(rail.vfep_id) == vfeps.end()) {
            err("Rail", id, "vfep_id missing or does not exist: " + rail.vfep_id);
        }
        if (rail.points_xy_mm.size() < 2) warn("Rail", id, "points_xy_mm has <2 points (no usable polyline).");
        for (const auto& rid : rail.related_rack_ids) {
            if (racks.find(rid) == racks.end()) err("Rail", id, "related_rack_id does not exist: " + rid);
        }
    }

    // Tank -> VFEP, optional rail
    for (const auto& [id, t] : tanks) {
        if (!nonEmpty(id)) err("Tank", id, "tank_id must be non-empty.");
        if (!nonEmpty(t.vfep_id) || vfeps.find(t.vfep_id) == vfeps.end()) {
            err("Tank", id, "vfep_id missing or does not exist: " + t.vfep_id);
        }
        if (nonEmpty(t.rail_id) && rails.find(t.rail_id) == rails.end()) {
            err("Tank", id, "rail_id does not exist: " + t.rail_id);
        }
        if (t.capacity_L <= 0.0) warn("Tank", id, "capacity_L <= 0 (unset).");
    }

    // Arms -> VFEP + Rail
    for (const auto& [id, a] : arms) {
        if (!nonEmpty(id)) err("Arm", id, "arm_id must be non-empty.");
        if (!nonEmpty(a.vfep_id) || vfeps.find(a.vfep_id) == vfeps.end()) {
            err("Arm", id, "vfep_id missing or does not exist: " + a.vfep_id);
        }
        if (!nonEmpty(a.rail_id) || rails.find(a.rail_id) == rails.end()) {
            err("Arm", id, "rail_id missing or does not exist: " + a.rail_id);
        }
        if (a.parking_s_0_1 < 0.0 || a.parking_s_0_1 > 1.0) err("Arm", id, "parking_s_0_1 must be in [0,1].");
        if (a.travel_s_min_0_1 < 0.0 || a.travel_s_max_0_1 > 1.0 || a.travel_s_min_0_1 > a.travel_s_max_0_1) {
            err("Arm", id, "travel range must be within [0,1] and min<=max.");
        }
    }

    // Nozzle -> Arm
    for (const auto& [id, n] : nozzles) {
        if (!nonEmpty(id)) err("Nozzle", id, "nozzle_id must be non-empty.");
        if (!nonEmpty(n.arm_id) || arms.find(n.arm_id) == arms.end()) {
            err("Nozzle", id, "arm_id missing or does not exist: " + n.arm_id);
        }
        if (n.flow_rate_kg_s < 0.0) err("Nozzle", id, "flow_rate_kg_s must be >= 0.");
    }

    // Magazine -> Arm or VFEP (at least one)
    for (const auto& [id, m] : magazines) {
        if (!nonEmpty(id)) err("Magazine", id, "magazine_id must be non-empty.");
        const bool has_arm = nonEmpty(m.arm_id);
        const bool has_vfep = nonEmpty(m.vfep_id);
        if (!has_arm && !has_vfep) err("Magazine", id, "must reference arm_id and/or vfep_id.");
        if (has_arm && arms.find(m.arm_id) == arms.end()) err("Magazine", id, "arm_id does not exist: " + m.arm_id);
        if (has_vfep && vfeps.find(m.vfep_id) == vfeps.end()) err("Magazine", id, "vfep_id does not exist: " + m.vfep_id);
        if (m.capacity_rounds < 0) err("Magazine", id, "capacity_rounds must be >= 0.");
    }

    // Trigger + Manual control -> VFEP
    for (const auto& [id, t] : trigger_configs) {
        if (!nonEmpty(id)) err("TriggerConfig", id, "trigger_config_id must be non-empty.");
        if (!nonEmpty(t.vfep_id) || vfeps.find(t.vfep_id) == vfeps.end()) {
            err("TriggerConfig", id, "vfep_id missing or does not exist: " + t.vfep_id);
        }
    }
    for (const auto& [id, m] : manual_controls) {
        if (!nonEmpty(id)) err("ManualControl", id, "manual_control_id must be non-empty.");
        if (!nonEmpty(m.vfep_id) || vfeps.find(m.vfep_id) == vfeps.end()) {
            err("ManualControl", id, "vfep_id missing or does not exist: " + m.vfep_id);
        }
    }

    // VFE -> Chemical + Shell
    for (const auto& [id, v] : vfe_round_types) {
        if (!nonEmpty(id)) err("VFE", id, "vfe_id must be non-empty.");
        if (!nonEmpty(v.chemical_id) || chemicals.find(v.chemical_id) == chemicals.end()) {
            err("VFE", id, "chemical_id missing or does not exist: " + v.chemical_id);
        }
        if (!nonEmpty(v.shell_id) || shells.find(v.shell_id) == shells.end()) {
            err("VFE", id, "shell_id missing or does not exist: " + v.shell_id);
        }
    }

    // Cameras -> Room
    for (const auto& [id, c] : cameras) {
        if (!nonEmpty(id)) err("Camera", id, "camera_id must be non-empty.");
        if (!nonEmpty(c.room_id) || rooms.find(c.room_id) == rooms.end()) {
            err("Camera", id, "room_id missing or does not exist: " + c.room_id);
        }
    }

    // Arm view -> Camera + Arm
    for (const auto& [id, av] : arm_views) {
        if (!nonEmpty(av.camera_id) || cameras.find(av.camera_id) == cameras.end()) {
            err("ArmView", id, "camera_id missing or does not exist: " + av.camera_id);
        }
        if (!nonEmpty(av.arm_id) || arms.find(av.arm_id) == arms.end()) {
            err("ArmView", id, "arm_id missing or does not exist: " + av.arm_id);
        }
    }

    // DC view -> Camera
    for (const auto& [id, dv] : datacenter_views) {
        if (!nonEmpty(dv.camera_id) || cameras.find(dv.camera_id) == cameras.end()) {
            err("DataCenterView", id, "camera_id missing or does not exist: " + dv.camera_id);
        }
    }

    // Map -> Room + referenced ids
    for (const auto& [id, map] : maps) {
        if (!nonEmpty(map.room_id) || rooms.find(map.room_id) == rooms.end()) {
            err("MapModel", id, "room_id missing or does not exist: " + map.room_id);
        }
        for (const auto& rid : map.rack_ids) {
            if (racks.find(rid) == racks.end()) err("MapModel", id, "rack_id does not exist: " + rid);
        }
        for (const auto& rail_id : map.rail_ids) {
            if (rails.find(rail_id) == rails.end()) err("MapModel", id, "rail_id does not exist: " + rail_id);
        }
    }

    // Basic telemetry presence warnings (not errors): encourages separation.
    for (const auto& [id, room] : rooms) {
        if (room_telemetry.find(id) == room_telemetry.end()) warn("DataCenterRoom", id, "no telemetry object present (room_telemetry).");
    }
    for (const auto& [id, rack] : racks) {
        if (rack_telemetry.find(id) == rack_telemetry.end()) warn("Rack", id, "no telemetry object present (rack_telemetry).");
    }

// VFEP selection/suppression coherence (minimal)
for (const auto& [id, vfep] : vfeps) {
    if (!vfep.selected_rack_id.empty()) {
        if (racks.find(vfep.selected_rack_id) == racks.end())
            err("VFEP", id, "selected_rack_id does not exist: " + vfep.selected_rack_id);
    }
    if (vfep.selected_hotspot_u < 0 || vfep.selected_hotspot_u > 60)
        warn("VFEP", id, "selected_hotspot_u out of expected range [0,60].");
}

// Orchestration objects: basic reference checks
for (const auto& [iid, il] : interlocks) {
    if (!il.vfep_id.empty() && vfeps.find(il.vfep_id) == vfeps.end())
        err("InterlockStatus", iid, "vfep_id does not exist: " + il.vfep_id);
}
for (const auto& [pid, plan] : suppression_plans) {
    if (!plan.vfep_id.empty() && vfeps.find(plan.vfep_id) == vfeps.end())
        err("SuppressionPlan", pid, "vfep_id does not exist: " + plan.vfep_id);
    if (!plan.target_rack_id.empty() && racks.find(plan.target_rack_id) == racks.end())
        err("SuppressionPlan", pid, "target_rack_id does not exist: " + plan.target_rack_id);
}
for (const auto& [cid, inc] : incidents) {
    if (!inc.room_id.empty() && rooms.find(inc.room_id) == rooms.end())
        err("Incident", cid, "room_id does not exist: " + inc.room_id);
    if (!inc.rack_id.empty() && racks.find(inc.rack_id) == racks.end())
        err("Incident", cid, "rack_id does not exist: " + inc.rack_id);
}
for (const auto& [aid, al] : alerts) {
    if (!al.room_id.empty() && rooms.find(al.room_id) == rooms.end())
        err("Alert", aid, "room_id does not exist: " + al.room_id);
    if (!al.rack_id.empty() && racks.find(al.rack_id) == racks.end())
        err("Alert", aid, "rack_id does not exist: " + al.rack_id);
}

    return r;
}

// Deterministic default 4x4 rack layout (architect.txt section 6).
static inline std::vector<RackConfig> buildDefaultRacks4x4(const std::string& room_id) {
    const double rack_w = 600.0;
    const double rack_d = 1200.0;
    const double gap_x = 300.0;
    const double dx = rack_w + gap_x;   // 900
    const double dy = 2400.0;           // rack_d + aisle_width (1200), per architect defaults
    const double x0 = 2000.0;
    const double y0 = 2000.0;

    struct RowDef { const char* row; int idx; double y; double rot; };
    const RowDef rows[] = {
        {"A", 0, y0 + 0.0*dy,   0.0},
        {"B", 1, y0 + 1.0*dy, 180.0},
        {"C", 2, y0 + 2.0*dy,   0.0},
        {"D", 3, y0 + 3.0*dy, 180.0},
    };
    const double xs[] = { x0 + 0.0*dx, x0 + 1.0*dx, x0 + 2.0*dx, x0 + 3.0*dx };

    std::vector<RackConfig> out;
    out.reserve(16);

    for (const auto& row : rows) {
        for (int c = 0; c < 4; ++c) {
            RackConfig r;
            r.room_id = room_id;
            r.row = row.row;
            r.col_index = c + 1;
            r.label = r.row + std::to_string(r.col_index);
            r.rack_id = "rack-" + r.label;
            r.centroid_x_mm = xs[c];
            r.centroid_y_mm = row.y;
            r.rotation_deg = row.rot;
            r.width_mm = rack_w;
            r.depth_mm = rack_d;
            r.height_u = 42;
            r.aisle_designation = "unspecified";
            out.push_back(r);
        }
    }
    return out;
}

ObjectStore makeDefault4x4ObjectStore() {
    ObjectStore s;

    // Room
    DataCenterRoomConfig room;
    room.room_id = "room-dc-h5";
    room.name = "DC-H5";
    room.location = "default-site";
    room.security_level = "High";
    room.floor_number = 1;
    room.areas = {"hot_aisle", "cold_aisle"};
    room.data_types = {"internal"};
    room.number_of_racks = 16;
    room.rack_configurations = {"standard_600x1200_42U"};
    room.rack_floor_density_kg_m2 = 0.0;
    room.rack_heights_supported_u = {42, 45, 48};
    room.floor_plan.asset_id = "asset-floorplan-room-dc-h5";
    room.floor_plan.file_name = "floor_plan_placeholder.pdf";
    room.floor_plan.version = "v1";
    s.upsert(room);

    // Room telemetry defaults
    DataCenterRoomTelemetry rt;
    rt.temperature_C = 27.0;
    rt.humidity_pct = 45.0;
    rt.airflow_m3_s = 0.0;
    rt.room_pressure_Pa = 0.0;
    rt.temperature_thresholds = {15.0, 50.0, 24.0, 18.0, 35.0};
    rt.humidity_thresholds = {10.0, 90.0, 45.0, 20.0, 70.0};
    s.room_telemetry[room.room_id] = rt;

    // Racks
    for (const auto& rack : buildDefaultRacks4x4(room.room_id)) {
        s.upsert(rack);
        RackTelemetry rtel;
        rtel.is_on_fire = false;
        rtel.surface_temp_C = 30.0;
        rtel.risk_to_assets_pct = 0.0;
        s.rack_telemetry[rack.rack_id] = rtel;
    }

    // VFEP (one system covering all racks for MVP)
    VFEPConfig vfep;
    vfep.vfep_id = "vfep-01";
    vfep.room_id = room.room_id;
    vfep.mounting_type = "overhead_rail";
    vfep.status = VFEPStatus::Armed;
    vfep.firmware_version = "sim";
    for (const auto& [rid, rack] : s.racks) vfep.coverage_rack_ids.push_back(rid);
    std::sort(vfep.coverage_rack_ids.begin(), vfep.coverage_rack_ids.end());
    s.upsert(vfep);

    // Sensors (basic heat + smoke, placed on two racks)
    {
        SensorConfig heat;
        heat.sensor_id = "sensor-heat-01";
        heat.vfep_id = vfep.vfep_id;
        heat.type = "heat";
        heat.rack_id = "rack-A1";
        heat.rack_u_position = 40;
        heat.position_mm = {2000.0, 2000.0, 2000.0};
        heat.facing_deg = 0.0;
        heat.thresholds = {0.0, 200.0, 60.0, 45.0, 90.0};
        heat.calibration_notes = "default";
        s.upsert(heat);
        s.sensor_telemetry[heat.sensor_id] = {25.0, true, ""};

        SensorConfig smoke;
        smoke.sensor_id = "sensor-smoke-01";
        smoke.vfep_id = vfep.vfep_id;
        smoke.type = "smoke";
        smoke.rack_id = "rack-B1";
        smoke.rack_u_position = 40;
        smoke.position_mm = {2000.0, 4400.0, 2000.0};
        smoke.facing_deg = 180.0;
        smoke.thresholds = {0.0, 1.0, 0.0, 0.2, 0.6};
        smoke.calibration_notes = "default";
        s.upsert(smoke);
        s.sensor_telemetry[smoke.sensor_id] = {0.0, true, ""};
    }

    // Rail polyline: simple straight line spanning the rack field (placeholder geometry)
    RailConfig rail;
    rail.rail_id = "rail-01";
    rail.vfep_id = vfep.vfep_id;
    rail.label = "overhead-rail-main";
    rail.mount_height_mm = 5600.0; // matches the UI sample (5.6m)
    rail.points_xy_mm = { {1500.0, 1500.0}, {5200.0, 1500.0}, {5200.0, 9800.0}, {1500.0, 9800.0} };
    for (const auto& [rid, rack] : s.racks) rail.related_rack_ids.push_back(rid);
    rail.rail_mapping_pdf.asset_id = "asset-railmap-rail-01";
    rail.rail_mapping_pdf.file_name = "rail_map_placeholder.pdf";
    rail.rail_mapping_pdf.version = "v1";
    rail.pdf_alignment.coordinate_system = "room_plan_xy_mm";
    rail.pdf_alignment.scale = 1.0;
    rail.pdf_alignment.origin_offset_mm = {0.0, 0.0};
    rail.pdf_alignment.rotation_deg = 0.0;
    s.upsert(rail);

    // Tank
    TankConfig tank;
    tank.tank_id = "tank-01";
    tank.vfep_id = vfep.vfep_id;
    tank.rail_id = rail.rail_id;
    tank.gas_type = "Nitrogen";
    tank.capacity_L = 50.0;
    tank.initial_agent_mass_kg = 25.0;
    tank.regulator_setpoint_bar = 245.0;
    tank.pressure_thresholds = {0.0, 300.0, 245.0, 50.0, 280.0};
    s.upsert(tank);
    vfep::obj::TankTelemetry tt;
    tt.current_pressure_bar = 245.0;
    tt.regulator_bar = tank.regulator_setpoint_bar;
    tt.remaining_agent_mass_kg = tank.initial_agent_mass_kg;
    tt.current_flow_kg_s = 0.0;
    tt.is_depleted = false;
    tt.valve_state = "online";
    s.tank_telemetry[tank.tank_id] = tt;

    // Arms (2) + nozzle + magazine
    for (int i = 1; i <= 2; ++i) {
        const std::string arm_id = "arm-" + std::to_string(i);
        ArmConfig arm;
        arm.arm_id = arm_id;
        arm.vfep_id = vfep.vfep_id;
        arm.rail_id = rail.rail_id;
        arm.parking_s_0_1 = (i == 1) ? 0.25 : 0.75;
        arm.travel_s_min_0_1 = 0.0;
        arm.travel_s_max_0_1 = 1.0;
        arm.safety_interlocks = {"door_open", "human_presence", "maintenance_lockout"};
        s.upsert(arm);

        ArmTelemetry at;
        at.state = ArmDeploymentState::Stowed;
        at.s_0_1 = arm.parking_s_0_1;
        at.interlock_active = false;
        at.v_s_0_1_per_s = 0.0;
        at.target_s_0_1 = at.s_0_1;
        at.has_target = false;
        at.last_command_ms = 0;
        at.last_command_source = "";
        s.arm_telemetry[arm_id] = at;

        NozzleConfig noz;
        noz.nozzle_id = "nozzle-" + std::to_string(i);
        noz.arm_id = arm_id;
        noz.flow_rate_kg_s = 14.9;  // matches UI sample (kg/min in UI; here kg/s is placeholder)
        noz.spray_pattern = "cone";
        s.upsert(noz);
        vfep::obj::NozzleTelemetry nt;
        nt.clogged = false;
        nt.pan_deg = 0.0;
        nt.tilt_deg = 0.0;
        nt.target_pan_deg = 0.0;
        nt.target_tilt_deg = 0.0;
        nt.has_target = false;
        nt.last_command_ms = 0;
        nt.last_command_source = "";
        s.nozzle_telemetry[noz.nozzle_id] = nt;

        MagazineConfig mag;
        mag.magazine_id = "mag-" + std::to_string(i);
        mag.arm_id = arm_id;
        mag.vfep_id = vfep.vfep_id;
        mag.capacity_rounds = 10;
        mag.reload_procedure_ref = "reload-proc-v1";
        mag.lot_tracking = "lot-placeholder";
        s.upsert(mag);
        s.magazine_telemetry[mag.magazine_id] = {10};

        // Onboard camera for the arm/nozzle view
        CameraConfig cam;
        cam.camera_id = "cam-arm-" + std::to_string(i);
        cam.room_id = room.room_id;
        cam.type = CameraType::OnboardArm;
        cam.position_mm = {0.0, 0.0, rail.mount_height_mm};
        cam.retention_policy = "volatile";
        s.upsert(cam);
        s.camera_telemetry[cam.camera_id] = {true, "ok"};

        ArmViewConfig av;
        av.camera_id = cam.camera_id;
        av.arm_id = arm_id;
        av.reticle_calibration = "reticle-calib-placeholder";
        av.latency_budget_ms = 50.0;
        s.upsert(av);
    }

    // Fixed room camera
    CameraConfig room_cam;
    room_cam.camera_id = "cam-room-01";
    room_cam.room_id = room.room_id;
    room_cam.type = CameraType::Fixed;
    room_cam.position_mm = {1000.0, 1000.0, 3000.0};
    room_cam.yaw_deg = 45.0;
    room_cam.pitch_deg = -20.0;
    room_cam.roll_deg = 0.0;
    room_cam.retention_policy = "default";
    for (const auto& [rid, rack] : s.racks) room_cam.coverage_rack_ids.push_back(rid);
    s.upsert(room_cam);
    s.camera_telemetry[room_cam.camera_id] = {true, "ok"};

    DataCenterViewConfig dcv;
    dcv.camera_id = room_cam.camera_id;
    dcv.fixed_angle_profile = "default_fixed";
    dcv.rack_visibility_index = "placeholder";
    s.upsert(dcv);

    // Trigger + manual control
    TriggerConfig tc;
    tc.trigger_config_id = "trig-01";
    tc.vfep_id = vfep.vfep_id;
    tc.trigger_type = TriggerType::Automatic;
    tc.rules = "sensor_fusion: heat OR smoke; delay_s:2; confirm_s:1";
    tc.arming_policy = "armed_when: room_secure; access: ops";
    s.upsert(tc);

    ManualControlConfig mc;
    mc.manual_control_id = "manual-01";
    mc.vfep_id = vfep.vfep_id;
    mc.control_mode = "remote_console";
    mc.authentication_level_required = "ops_admin";
    mc.audit_logging_enabled = true;
    s.upsert(mc);

    // Chemical + shell + VFE round type
    ChemicalConfig chem;
    chem.chemical_id = "chem-purplek";
    chem.name = "Purple-K";
    chem.composition_or_msds_ref = "msds-purplek";
    chem.hazard_class = "irritant";
    chem.storage_requirements = "keep_dry";
    chem.expiry_date = "2099-01-01";
    s.upsert(chem);

    ShellConfig shell;
    shell.shell_id = "shell-std";
    shell.shell_material = "polymer";
    shell.shell_lot = "lot-std";
    shell.compatibility = "vfep-mag-standard";
    s.upsert(shell);

    VFEConfig vfe;
    vfe.vfe_id = "vfe-std";
    vfe.effective_range_m = 15.0;
    vfe.discharge_profile = "burst";
    vfe.safety_constraints = "storage_temp_0_50C";
    vfe.chemical_id = chem.chemical_id;
    vfe.shell_id = shell.shell_id;
    s.upsert(vfe);

    // Map model
    MapModelConfig map;
    map.room_id = room.room_id;
    map.floor_plan = room.floor_plan;
    for (const auto& [rid, rack] : s.racks) map.rack_ids.push_back(rid);
    std::sort(map.rack_ids.begin(), map.rack_ids.end());
    map.rail_ids.push_back(rail.rail_id);
    map.reference_points_xy_mm = { {0.0, 0.0}, {10000.0, 0.0}, {0.0, 10000.0} };
    s.upsert(map);


// Orchestration baseline objects (allow-by-default)
InterlockStatus il;
il.interlock_id = "interlock-" + vfep.vfep_id;
il.vfep_id = vfep.vfep_id;
il.allow_arm = true;
il.allow_suppress = true;
il.reasons = {};
il.updated_ms = 0;
s.upsert(il);

SuppressionPlan plan;
plan.plan_id = "plan-" + vfep.vfep_id;
plan.vfep_id = vfep.vfep_id;
plan.target_rack_id = "";
plan.target_hotspot_u = 0;
plan.requested_flow_kg_s = 0.0;
plan.estimated_time_to_control_s = 0.0;
plan.feasible = true;
plan.notes = {"default"};
plan.updated_ms = 0;
s.upsert(plan);

    return s;
}

} // namespace obj
} // namespace vfep
