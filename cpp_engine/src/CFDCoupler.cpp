/**
 * @file CFDCoupler.cpp
 * @brief Implementation of CFD coupling interface
 * 
 * Phase 9: Track C - CFD Coupling
 * 
 * Implements loose coupling between zone models and CFD simulations:
 * - Boundary condition export from zones to CFD
 * - CFD result import and mapping back to zones
 * - Temporal synchronization for stability
 * - Data interpolation between different grid types
 */

#include "CFDCoupler.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace vfep {

// Default coupling parameters
constexpr float DEFAULT_COUPLING_DT = 0.1f;  // 100ms coupling timestep
constexpr int DEFAULT_REMESH_FREQ = 10;      // Remesh every 10 coupling steps

// ============================================================================
// CONSTRUCTION & INITIALIZATION
// ============================================================================

CFDCoupler::CFDCoupler()
    : last_sync_time_(0.0f),
      loose_coupling_dt_(DEFAULT_COUPLING_DT),
            remesh_frequency_(DEFAULT_REMESH_FREQ),
            sync_count_(0) {
    // Initialize with empty CFD interface
}

CFDCoupler::~CFDCoupler() {
    // Cleanup handled automatically
}

void CFDCoupler::reset() {
    cfd_interface_.clear();
    last_sync_time_ = 0.0f;
    loose_coupling_dt_ = DEFAULT_COUPLING_DT;
    remesh_frequency_ = DEFAULT_REMESH_FREQ;
    sync_count_ = 0;
}

// ============================================================================
// COUPLING CONFIGURATION
// ============================================================================

void CFDCoupler::setLooseCouplingTimeStep(float dt_loose_s) {
    if (dt_loose_s <= 0.0f) {
        throw std::invalid_argument("Coupling timestep must be positive");
    }
    if (dt_loose_s > 10.0f) {
        throw std::invalid_argument("Coupling timestep too large (max 10s)");
    }
    loose_coupling_dt_ = dt_loose_s;
}

void CFDCoupler::setRemeshingFrequency(int steps) {
    if (steps < 1) {
        throw std::invalid_argument("Remeshing frequency must be at least 1");
    }
    if (steps > 1000) {
        throw std::invalid_argument("Remeshing frequency too large (max 1000)");
    }
    remesh_frequency_ = steps;
}

// ============================================================================
// BOUNDARY CONDITION EXPORT
// ============================================================================

void CFDCoupler::exportBoundaryConditions(const ThreeZoneModel& zones) {
    // Export zone properties as CFD boundary conditions
    // In a real implementation, this would:
    // 1. Extract zone temperatures, velocities, pressures
    // 2. Map to CFD domain boundaries
    // 3. Write boundary condition files for CFD solver
    
    // For this implementation, we store simplified boundary data
    std::vector<GridPoint> boundary_points;
    
    // Upper zone boundary
    const Zone& upper = zones.upperZone();
    GridPoint upper_pt;
    upper_pt.x = 0.5;  // Center of domain
    upper_pt.y = 0.5;
    upper_pt.z = 2.5;  // Upper zone height
    upper_pt.T_K = upper.T_K;
    upper_pt.rho_kg_m3 = upper.density_kg_m3();
    upper_pt.P_Pa = upper.P_Pa;
    upper_pt.u = 0.0;
    upper_pt.v = 0.0;
    upper_pt.w = 0.5;  // Upward buoyancy velocity estimate
    boundary_points.push_back(upper_pt);
    
    // Middle zone boundary
    const Zone& middle = zones.middleZone();
    GridPoint middle_pt;
    middle_pt.x = 0.5;
    middle_pt.y = 0.5;
    middle_pt.z = 1.5;  // Middle zone height
    middle_pt.T_K = middle.T_K;
    middle_pt.rho_kg_m3 = middle.density_kg_m3();
    middle_pt.P_Pa = middle.P_Pa;
    middle_pt.u = 0.0;
    middle_pt.v = 0.0;
    middle_pt.w = 0.1;  // Small upward velocity
    boundary_points.push_back(middle_pt);
    
    // Lower zone boundary
    const Zone& lower = zones.lowerZone();
    GridPoint lower_pt;
    lower_pt.x = 0.5;
    lower_pt.y = 0.5;
    lower_pt.z = 0.5;  // Lower zone height
    lower_pt.T_K = lower.T_K;
    lower_pt.rho_kg_m3 = lower.density_kg_m3();
    lower_pt.P_Pa = lower.P_Pa;
    lower_pt.u = 0.0;
    lower_pt.v = 0.0;
    lower_pt.w = 0.0;  // No vertical velocity at floor
    boundary_points.push_back(lower_pt);
    
    // In a real system, we'd write these to file or send to CFD solver
    // For now, store in internal CFD interface, honoring remesh cadence
    if (sync_count_ % remesh_frequency_ == 0 || cfd_interface_.gridPointCount() == 0) {
        mapZoneToCFDDomain(zones);
    }
}

// ============================================================================
// CFD RESULT IMPORT
// ============================================================================

void CFDCoupler::importCFDResults(const CFDInterface& cfd) {
    // Import CFD results into our internal interface
    // In a real implementation, this would:
    // 1. Read CFD solver output files (VTK, HDF5, etc.)
    // 2. Interpolate CFD data to zone model grid
    // 3. Extract relevant fields (temperature, velocity, pressure)
    
    // For simplified implementation, copy the CFD interface
    cfd_interface_ = cfd;
    
    // Map CFD domain back to zone representation
    mapCFDDomainToZones();
}

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

bool CFDCoupler::isSynchronized() const {
    // Check if coupling is synchronized
    // In loose coupling, we only exchange data every loose_coupling_dt_ seconds
    // Return true if we're within a coupling window
    
    // Simplified: consider synchronized if we have CFD data and have synced at least once
    return cfd_interface_.gridPointCount() > 0 && last_sync_time_ >= 0.0f;
}

void CFDCoupler::synchronize(float sim_time_s) {
    if (sim_time_s < last_sync_time_) {
        throw std::invalid_argument("Simulation time went backwards");
    }

    if (last_sync_time_ > 0.0f && (sim_time_s - last_sync_time_) < loose_coupling_dt_) {
        return;
    }
    
    // Update synchronization time
    // In a real implementation, this would:
    // 1. Check if we need to exchange data (time > last_sync + dt_coupling)
    // 2. Trigger boundary condition export
    // 3. Wait for/import CFD results
    // 4. Interpolate and update zone model
    
    last_sync_time_ = sim_time_s;
    ++sync_count_;

    runMockCFDStep(sim_time_s);
}

// ============================================================================
// COUPLED PROPERTY QUERIES
// ============================================================================

float CFDCoupler::getZoneTemperatureFromCFD(int zone_id) const {
    if (zone_id < 0 || zone_id > 2) {
        throw std::out_of_range("Zone ID must be 0 (upper), 1 (middle), or 2 (lower)");
    }
    
    if (cfd_interface_.gridPointCount() == 0) {
        throw std::runtime_error("No CFD data loaded");
    }
    
    // Interpolate CFD temperature at zone center location
    // Zone 0 (upper) at z=2.5, zone 1 (middle) at z=1.5, zone 2 (lower) at z=0.5
    double z_positions[3] = {2.5, 1.5, 0.5};
    double z = z_positions[zone_id];
    
    // Interpolate at center of domain (x=0.5, y=0.5)
    double T = cfd_interface_.interpolateTemperature(0.5, 0.5, z);
    
    return static_cast<float>(T);
}

float CFDCoupler::getZoneVelocityFromCFD(int zone_id) const {
    if (zone_id < 0 || zone_id > 2) {
        throw std::out_of_range("Zone ID must be 0 (upper), 1 (middle), or 2 (lower)");
    }
    
    if (cfd_interface_.gridPointCount() == 0) {
        throw std::runtime_error("No CFD data loaded");
    }
    
    // Interpolate CFD velocity at zone center location
    double z_positions[3] = {2.5, 1.5, 0.5};
    double z = z_positions[zone_id];
    
    double u, v, w;
    cfd_interface_.interpolateVelocity(0.5, 0.5, z, u, v, w);
    
    // Return vertical velocity magnitude (most relevant for buoyancy)
    return static_cast<float>(std::abs(w));
}

bool CFDCoupler::exportCouplingCSV(const std::string& filename,
                                  float time_s,
                                  const ThreeZoneModel& zones) const {
    if (cfd_interface_.gridPointCount() == 0) {
        return false;
    }

    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << "time_s,zone_id,vfep_T_K,vfep_P_Pa,vfep_rho_kgm3,cfd_T_K,cfd_w_mps\n";
    out << std::fixed << std::setprecision(6);

    const Zone* zones_ptr[3] = {&zones.upperZone(), &zones.middleZone(), &zones.lowerZone()};
    for (int zone_id = 0; zone_id < 3; ++zone_id) {
        const Zone& z = *zones_ptr[zone_id];
        float cfd_T = getZoneTemperatureFromCFD(zone_id);
        float cfd_w = getZoneVelocityFromCFD(zone_id);

        out << time_s << ','
            << zone_id << ','
            << static_cast<float>(z.T_K) << ','
            << static_cast<float>(z.P_Pa) << ','
            << static_cast<float>(z.density_kg_m3()) << ','
            << cfd_T << ','
            << cfd_w << '\n';
    }

    return true;
}

// ============================================================================
// PRIVATE METHODS - DOMAIN MAPPING
// ============================================================================

void CFDCoupler::mapZoneToCFDDomain(const ThreeZoneModel& zones) {
    // Map zone model to CFD domain
    // Create a simple structured grid with zone-averaged properties
    
    std::vector<GridPoint> grid;
    
    // Create 3x3x3 grid (27 points) representing the three zones
    const int nx = 3, ny = 3, nz = 3;
    const double dx = 1.0 / (nx - 1);
    const double dy = 1.0 / (ny - 1);
    const double dz = 3.0 / (nz - 1);  // 3m height
    
    for (int k = 0; k < nz; ++k) {
        // Determine which zone this height belongs to
        double z = k * dz;
        const Zone* zone_ptr;
        
        if (z > 2.0) {
            zone_ptr = &zones.upperZone();
        } else if (z > 1.0) {
            zone_ptr = &zones.middleZone();
        } else {
            zone_ptr = &zones.lowerZone();
        }
        
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                GridPoint pt;
                pt.x = i * dx;
                pt.y = j * dy;
                pt.z = z;
                pt.T_K = zone_ptr->T_K;
                pt.rho_kg_m3 = zone_ptr->density_kg_m3();
                pt.P_Pa = zone_ptr->P_Pa;
                pt.u = 0.0;
                pt.v = 0.0;
                pt.w = (z > 1.0) ? 0.5 : 0.1;  // Simple buoyancy estimate
                
                grid.push_back(pt);
            }
        }
    }
    
    // Store in internal CFD interface (normally would export to file)
    cfd_interface_.setGridPoints(grid, nx, ny, nz, 0.0, 0.0, 0.0, dx, dy, dz);
}

void CFDCoupler::runMockCFDStep(float sim_time_s) {
    if (cfd_interface_.gridPointCount() == 0) {
        return;
    }

    std::vector<GridPoint> updated = cfd_interface_.gridPoints();
    const double omega = 0.5;  // rad/s
    const double amp_T = 5.0;  // K
    const double amp_w = 0.1;  // m/s

    for (auto& p : updated) {
        const double phase = omega * sim_time_s + 0.5 * p.z;
        p.T_K += amp_T * std::sin(phase);
        p.w += amp_w * std::cos(phase);
    }

    cfd_interface_.setGridPoints(
        updated,
        cfd_interface_.gridNx(),
        cfd_interface_.gridNy(),
        cfd_interface_.gridNz(),
        cfd_interface_.gridXMin(),
        cfd_interface_.gridYMin(),
        cfd_interface_.gridZMin(),
        cfd_interface_.gridDx(),
        cfd_interface_.gridDy(),
        cfd_interface_.gridDz());
}

void CFDCoupler::mapCFDDomainToZones() {
    // Map CFD domain back to zones
    // In a real implementation, this would:
    // 1. Integrate CFD fields over zone volumes
    // 2. Compute volume-averaged properties
    // 3. Update zone model with CFD-informed values
    
    // For simplified implementation, just verify we have data
    if (cfd_interface_.gridPointCount() == 0) {
        // No CFD data to map
        return;
    }
    
    // In a full implementation, we'd compute:
    // - Volume-averaged temperature per zone
    // - Mass-weighted velocity profiles
    // - Inter-zone exchange rates from CFD velocity field
    // And use these to update the zone model
}

} // namespace vfep
