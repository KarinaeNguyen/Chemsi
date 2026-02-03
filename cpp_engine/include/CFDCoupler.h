/**
 * @file CFDCoupler.h
 * @brief Live CFD coupling interface
 * 
 * Phase 9: Track C - CFD Coupling
 * 
 * Provides:
 * - Boundary condition export to CFD
 * - CFD result import and synchronization
 * - Data interpolation between grid types
 * - Stability control for loose coupling
 * 
 * TODO: Implementation
 */

#ifndef CHEMSI_CFD_COUPLER_H
#define CHEMSI_CFD_COUPLER_H

#include "CFDInterface.h"
#include "ThreeZoneModel.h"

namespace vfep {

class CFDCoupler {
public:
    CFDCoupler();
    ~CFDCoupler();
    
    void reset();
    
    // Coupling configuration
    void setLooseCouplingTimeStep(float dt_loose_s);
    void setRemeshingFrequency(int steps);
    
    // Export zone state to CFD boundary conditions
    void exportBoundaryConditions(const ThreeZoneModel& zones);
    
    // Import CFD results
    void importCFDResults(const CFDInterface& cfd);
    
    // Synchronization
    bool isSynchronized() const;
    void synchronize(float sim_time_s);
    
    // Get coupled properties
    float getZoneTemperatureFromCFD(int zone_id) const;
    float getZoneVelocityFromCFD(int zone_id) const;
    
    // TODO: Full implementation
    
private:
    CFDInterface cfd_interface_;
    float last_sync_time_;
    float loose_coupling_dt_;
    int remesh_frequency_;
    
    void mapZoneToCFDDomain(const ThreeZoneModel& zones);
    void mapCFDDomainToZones();
};

} // namespace vfep

#endif // CHEMSI_CFD_COUPLER_H
