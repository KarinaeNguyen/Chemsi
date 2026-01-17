#pragma once

#include <cmath>
#include <cstddef>

// Uses vfep::world::Vec3d and CeilingRail model + geometry.
#include "ceiling_rail.h"

namespace vfep {
namespace world {

class RailMountedNozzle {
public:
    struct Config {
        // Vertical distance from rail centerline down to nozzle (meters).
        double nozzle_drop_from_rail_m = 0.15;
    };

    struct Inputs {
        // Rail model must be recomputed/valid before nozzle recompute.
        const CeilingRail* ceiling_rail = nullptr;

        // Parametric position along the rail perimeter [0..1].
        double s_0_1 = 0.0;

        // Independent aiming, degrees.
        double yaw_deg   = 0.0;   // rotation about +Y (world up)
        double pitch_deg = 0.0;   // rotation about local-right after yaw
    };

    struct Pose {
        // Carriage pose on rail
        Vec3d rail_pos_room_m          = {0.0, 0.0, 0.0};
        Vec3d rail_tangent_unit_room   = {1.0, 0.0, 0.0};

        // Nozzle pose
        Vec3d nozzle_pos_room_m        = {0.0, 0.0, 0.0};
        Vec3d spray_dir_unit_room      = {1.0, 0.0, 0.0};

        // Convenience axes (right-handed frame)
        Vec3d up_unit_room             = {0.0, 1.0, 0.0};
        Vec3d right_unit_room          = {1.0, 0.0, 0.0};
        Vec3d forward_unit_room        = {1.0, 0.0, 0.0};
    };

public:
    void setConfig(const Config& cfg) { cfg_ = cfg; }
    const Config& config() const { return cfg_; }

    // Pure kinematics (no simulation side effects).
    void recompute(const Inputs& in);

    bool isValid() const { return valid_; }
    const Pose& pose() const { return pose_; }

private:
    Config cfg_;
    Pose pose_;
    bool valid_ = false;
};

} // namespace world
} // namespace vfep
