/**
 * @file CompartmentNetwork.h
 * @brief Multi-compartment fire network model
 * 
 * Phase 9: Track B - Multi-Compartment
 * 
 * Provides:
 * - Network of interconnected compartments
 * - Pressure-driven inter-compartment flow
 * - Opening (door, window) modeling
 * - Buoyancy-driven stack effects
 * 
 * TODO: Implementation
 */

#ifndef CHEMSI_COMPARTMENT_NETWORK_H
#define CHEMSI_COMPARTMENT_NETWORK_H

#include <vector>
#include "ThreeZoneModel.h"

namespace vfep {

struct Opening {
    int from_compartment;    // Source compartment
    int to_compartment;      // Target compartment
    float height_m;          // Opening height
    float width_m;           // Opening width
    float discharge_coeff;   // Typically 0.6-0.7
    
    Opening(int from, int to, float h, float w, float cd = 0.65f)
        : from_compartment(from), to_compartment(to),
          height_m(h), width_m(w), discharge_coeff(cd) {}
    
    float getArea() const { return height_m * width_m; }
};

class CompartmentNetwork {
public:
    CompartmentNetwork();
    ~CompartmentNetwork();
    
    void reset();
    
    // Compartment management
    int addCompartment(const ThreeZoneModel& initial_state);
    Opening& addOpening(const Opening& opening);
    
    // Simulation
    void step(float dt, const std::vector<float>& HRR_W);
    
    // Data access
    const ThreeZoneModel& getCompartment(int id) const;
    float getInterCompartmentFlow(int from_id, int to_id) const;
    float getCompartmentPressure(int id) const;
    
    // TODO: Full implementation
    
private:
    std::vector<ThreeZoneModel> compartments_;
    std::vector<Opening> openings_;
    std::vector<std::vector<float>> mass_flow_;  // kg/s between compartments
    std::vector<float> pressures_;               // Pa
    
    void calculatePressures();
    void calculateMassFlow();
};

} // namespace vfep

#endif // CHEMSI_COMPARTMENT_NETWORK_H
