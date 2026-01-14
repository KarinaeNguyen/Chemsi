#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace vfep {
namespace obj {

struct Point2MM { double x_mm = 0.0; double y_mm = 0.0; };
struct Vec3MM { double x_mm = 0.0; double y_mm = 0.0; double z_mm = 0.0; };

struct Threshold1D {
    double min = 0.0, max = 0.0, target = 0.0, alarm_low = 0.0, alarm_high = 0.0;
};

struct AssetRef {
    std::string asset_id, file_name, storage_uri, version;
    std::string effective_from, effective_to, uploaded_at, uploaded_by, checksum_hash;
};

struct PdfAlignment {
    std::string coordinate_system;
    double scale = 1.0;
    Point2MM origin_offset_mm{0.0, 0.0};
    double rotation_deg = 0.0;
};

struct ValidationIssue {
    enum class Severity : std::uint32_t { Warning = 0, Error = 1 };
    Severity severity = Severity::Error;
    std::string object_type, object_id, message;
};

struct ValidationReport {
    std::vector<ValidationIssue> issues;
    bool ok() const {
        for (const auto& i : issues) if (i.severity == ValidationIssue::Severity::Error) return false;
        return true;
    }
};

// 1) Room
struct DataCenterRoomConfig {
    std::string room_id, name, location, security_level;
    int floor_number = 0;
    std::vector<std::string> areas, data_types;
    int number_of_racks = 0;
    std::vector<std::string> rack_configurations;
    double rack_floor_density_kg_m2 = 0.0;
    std::vector<int> rack_heights_supported_u;
    AssetRef floor_plan;
};

struct DataCenterRoomTelemetry {
    double temperature_C = 0.0, humidity_pct = 0.0, airflow_m3_s = 0.0, room_pressure_Pa = 0.0;
    Threshold1D temperature_thresholds, humidity_thresholds, airflow_thresholds, pressure_thresholds;
};

// 2) Rack
struct RackConfig {
    std::string rack_id, label, room_id;
    double centroid_x_mm = 0.0, centroid_y_mm = 0.0, rotation_deg = 0.0;
    int height_u = 42;
    double width_mm = 600.0, depth_mm = 1200.0, max_load_kg = 0.0;
    std::string row, aisle_designation;
    int col_index = 0;
};

struct RackTelemetry {
    bool is_on_fire = false;
    double surface_temp_C = 0.0;
    double risk_to_assets_pct = 0.0;
};

// 3) VFEP + sub-objects
enum class VFEPStatus : std::uint32_t { Normal = 0, Armed = 1, Maintenance = 2, Fault = 3, Offline = 4 };
enum class ArmDeploymentState : std::uint32_t { Stowed = 0, Moving = 1, Aiming = 2, Firing = 3, Returning = 4, Fault = 5 };
enum class CameraType : std::uint32_t { Fixed = 0, PTZ = 1, OnboardArm = 2 };
enum class TriggerType : std::uint32_t { Automatic = 0, Manual = 1, ScheduledTest = 2, RemoteOperator = 3 };

struct VFEPConfig {
    std::string vfep_id, room_id;
    std::vector<std::string> coverage_rack_ids;
    std::string mounting_type, last_tested_at, firmware_version;
        // Unity / orchestration state
    std::string selected_rack_id;
    int selected_hotspot_u = 0;
    bool suppression_active = false;
VFEPStatus status = VFEPStatus::Normal;
};

struct SensorConfig {
    std::string sensor_id, vfep_id, type;
    std::string rack_id;
    int rack_u_position = 0;
    Vec3MM position_mm;
    double facing_deg = 0.0;
    Threshold1D thresholds;
    std::string calibration_notes;
};

struct SensorTelemetry { double reading = 0.0; bool online = true; std::string error_code; };

struct RailConfig {
    std::string rail_id, vfep_id, label;
    std::vector<Point2MM> points_xy_mm;
    double mount_height_mm = 0.0;
    AssetRef rail_mapping_pdf;
    PdfAlignment pdf_alignment;
    std::vector<std::string> related_rack_ids;
};

struct TankConfig {
    std::string tank_id, vfep_id, rail_id, gas_type;
    double capacity_L = 0.0;
        // Suppressant resource model
    double initial_agent_mass_kg = 0.0;
    double regulator_setpoint_bar = 0.0;
Threshold1D pressure_thresholds;
    std::string last_inspected_at, next_inspection_at;
};

struct TankTelemetry {
    double current_pressure_bar = 0.0;
    double regulator_bar = 0.0;
    double remaining_agent_mass_kg = 0.0;
    double current_flow_kg_s = 0.0;
    bool is_depleted = false;
    std::string valve_state;
};

struct ArmConfig {
    std::string arm_id, vfep_id, rail_id;
    double parking_s_0_1 = 0.0, travel_s_min_0_1 = 0.0, travel_s_max_0_1 = 1.0;
        // Deterministic kinematics
    double max_v_s_0_1_per_s = 0.5;
    double max_a_s_0_1_per_s2 = 1.5;
std::vector<std::string> safety_interlocks;
};

struct ArmTelemetry {
    ArmDeploymentState state = ArmDeploymentState::Stowed;
    double s_0_1 = 0.0;
    // Kinematics (s normalized to [0,1])
    double v_s_0_1_per_s = 0.0;
    double target_s_0_1 = 0.0;
    bool has_target = false;
    // Safety/fault
    bool interlock_active = false;
    std::string fault_code;
    // Telemetry traceability
    std::uint64_t last_command_ms = 0;
    std::string last_command_source;
};

struct NozzleConfig {
    std::string nozzle_id, arm_id;
    double pan_min_deg = -90.0, pan_max_deg = 90.0, tilt_min_deg = -30.0, tilt_max_deg = 60.0;
    double flow_rate_kg_s = 0.0;
    std::string spray_pattern;
};

struct NozzleTelemetry {
    bool clogged = false;
    // Current solution
    double pan_deg = 0.0;
    double tilt_deg = 0.0;
    // Commanded targets
    double target_pan_deg = 0.0;
    double target_tilt_deg = 0.0;
    bool has_target = false;
    // Telemetry traceability
    std::uint64_t last_command_ms = 0;
    std::string last_command_source;
};

struct MagazineConfig {
    std::string magazine_id, arm_id, vfep_id;
    int capacity_rounds = 0;
    std::string reload_procedure_ref, lot_tracking;
};

struct MagazineTelemetry { int current_rounds = 0; };

struct TriggerConfig {
    std::string trigger_config_id, vfep_id;
    TriggerType trigger_type = TriggerType::Automatic;
    std::string rules, arming_policy;
};

struct ManualControlConfig {
    std::string manual_control_id, vfep_id, control_mode, authentication_level_required;
    bool audit_logging_enabled = true;
};

struct VFEConfig {
    std::string vfe_id;
    double effective_range_m = 0.0;
    std::string discharge_profile, safety_constraints;
    std::string chemical_id, shell_id;
};

struct ChemicalConfig { std::string chemical_id, name, composition_or_msds_ref, hazard_class, storage_requirements, expiry_date; };
struct ShellConfig { std::string shell_id, shell_material, shell_lot, compatibility; };

// 4) Cameras
struct CameraConfig {
    std::string camera_id, room_id;
    CameraType type = CameraType::Fixed;
    Vec3MM position_mm;
    double yaw_deg = 0.0, pitch_deg = 0.0, roll_deg = 0.0;
    std::string stream_endpoint_ref, retention_policy;
    std::vector<std::string> coverage_rack_ids;
};

struct CameraTelemetry { bool online = true; std::string health; };

struct ArmViewConfig { std::string camera_id, arm_id, reticle_calibration; double latency_budget_ms = 0.0; };
struct DataCenterViewConfig { std::string camera_id, fixed_angle_profile, rack_visibility_index; };

// 5) Map model
struct MapCoordSystem { std::string units = "mm"; std::string origin = "room southwest corner"; std::string axes = "+X east/right, +Y north/up"; };

struct MapModelConfig {
    std::string room_id;
    MapCoordSystem coord;
    AssetRef floor_plan;
    std::vector<std::string> rack_ids;
    std::vector<std::string> rail_ids;
    std::vector<Point2MM> reference_points_xy_mm;
    PdfAlignment pdf_alignment;
};

// 7) Events

// 4) Orchestration / decision outputs (minimal V1)

struct InterlockStatus {
    std::string interlock_id, vfep_id;
    bool allow_arm = true;
    bool allow_suppress = true;
    // Human-readable reasons when blocked
    std::vector<std::string> reasons;
    std::uint64_t updated_ms = 0;
};

enum class IncidentState : std::uint32_t { None = 0, Active = 1, Suppressing = 2, Resolved = 3 };

struct Incident {
    std::string incident_id;
    std::string room_id;
    std::string rack_id;
    IncidentState state = IncidentState::None;
    double started_at_s = 0.0;
    double resolved_at_s = 0.0;
    std::vector<std::string> tags;
};

enum class AlertSeverity : std::uint32_t { Info = 0, Warning = 1, Critical = 2 };

struct Alert {
    std::string alert_id;
    std::string room_id;
    std::string rack_id;
    AlertSeverity severity = AlertSeverity::Info;
    std::string code;
    std::string message;
    std::uint64_t created_ms = 0;
    bool acknowledged = false;
};

struct SuppressionPlan {
    std::string plan_id, vfep_id;
    std::string target_rack_id;
    int target_hotspot_u = 0;
    double requested_flow_kg_s = 0.0;
    double estimated_time_to_control_s = 0.0;
    bool feasible = true;
    std::vector<std::string> notes;
    std::uint64_t updated_ms = 0;
};

struct TriggerEvent { std::string event_id, vfep_id, source, occurred_at, decision_outcome, evidence_ref; };
struct DeploymentEvent { std::string event_id, vfep_id, arm_id, occurred_at, target_rack_id, aim_solution, discharge_params; };
struct ManualActionEvent { std::string event_id, vfep_id, occurred_at, operator_id, action, reason, approvals; };
struct FaultEvent { std::string event_id, vfep_id, occurred_at, code, impacted_subsystem, mitigation; };
struct MaintenanceEvent { std::string event_id, vfep_id, occurred_at, activity, notes; };

// Store
class ObjectStore {
public:
    std::unordered_map<std::string, DataCenterRoomConfig> rooms;
    std::unordered_map<std::string, RackConfig> racks;
    std::unordered_map<std::string, VFEPConfig> vfeps;
    std::unordered_map<std::string, SensorConfig> sensors;
    std::unordered_map<std::string, RailConfig> rails;
    std::unordered_map<std::string, TankConfig> tanks;
    std::unordered_map<std::string, ArmConfig> arms;
    std::unordered_map<std::string, NozzleConfig> nozzles;
    std::unordered_map<std::string, MagazineConfig> magazines;
    std::unordered_map<std::string, TriggerConfig> trigger_configs;
    std::unordered_map<std::string, ManualControlConfig> manual_controls;
    std::unordered_map<std::string, VFEConfig> vfe_round_types;
    std::unordered_map<std::string, ChemicalConfig> chemicals;
    std::unordered_map<std::string, ShellConfig> shells;
    std::unordered_map<std::string, CameraConfig> cameras;
    std::unordered_map<std::string, ArmViewConfig> arm_views;
    std::unordered_map<std::string, DataCenterViewConfig> datacenter_views;
    std::unordered_map<std::string, MapModelConfig> maps;


    // Orchestration objects
    std::unordered_map<std::string, InterlockStatus> interlocks;
    std::unordered_map<std::string, SuppressionPlan> suppression_plans;
    std::unordered_map<std::string, Incident> incidents;
    std::unordered_map<std::string, Alert> alerts;
std::unordered_map<std::string, DataCenterRoomTelemetry> room_telemetry;
    std::unordered_map<std::string, RackTelemetry> rack_telemetry;
    std::unordered_map<std::string, SensorTelemetry> sensor_telemetry;
    std::unordered_map<std::string, TankTelemetry> tank_telemetry;
    std::unordered_map<std::string, ArmTelemetry> arm_telemetry;
    std::unordered_map<std::string, NozzleTelemetry> nozzle_telemetry;
    std::unordered_map<std::string, MagazineTelemetry> magazine_telemetry;
    std::unordered_map<std::string, CameraTelemetry> camera_telemetry;

    std::vector<TriggerEvent> trigger_events;
    std::vector<DeploymentEvent> deployment_events;
    std::vector<ManualActionEvent> manual_action_events;
    std::vector<FaultEvent> fault_events;
    std::vector<MaintenanceEvent> maintenance_events;

    void clear();

    void upsert(const DataCenterRoomConfig& v);
    void upsert(const RackConfig& v);
    void upsert(const VFEPConfig& v);
    void upsert(const SensorConfig& v);
    void upsert(const RailConfig& v);
    void upsert(const TankConfig& v);
    void upsert(const ArmConfig& v);
    void upsert(const NozzleConfig& v);
    void upsert(const MagazineConfig& v);
    void upsert(const TriggerConfig& v);
    void upsert(const ManualControlConfig& v);
    void upsert(const VFEConfig& v);
    void upsert(const ChemicalConfig& v);
    void upsert(const ShellConfig& v);
    void upsert(const CameraConfig& v);
    void upsert(const ArmViewConfig& v);
    void upsert(const DataCenterViewConfig& v);
    void upsert(const MapModelConfig& v);
    void upsert(const InterlockStatus& v);
    void upsert(const SuppressionPlan& v);
    void upsert(const Incident& v);
    void upsert(const Alert& v);

    ValidationReport validate() const;
};

ObjectStore makeDefault4x4ObjectStore();

} // namespace obj
} // namespace vfep
