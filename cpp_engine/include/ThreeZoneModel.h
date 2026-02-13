#pragma once

#include <vector>
#include <string>
#include <cmath>

namespace vfep {

/**
 * @brief Zone structure for stratified fire model
 * 
 * Represents a single horizontal zone with uniform properties
 */
struct Zone {
    double volume_m3;       ///< Zone volume (m³)
    double height_m;        ///< Zone vertical extent (m)
    double T_K;             ///< Temperature (K)
    double P_Pa;            ///< Pressure (Pa)
    std::vector<double> n_mol; ///< Molar composition per species
    
    /**
     * @brief Calculate total heat content
     * @return Heat content in Joules
     */
    double heatContent_J() const;
    
    /**
     * @brief Calculate zone density
     * @return Density in kg/m³
     */
    double density_kg_m3() const;
    
    /**
     * @brief Calculate total mass
     * @return Mass in kg
     */
    double mass_kg() const;
};

/**
 * @brief Three-zone fire model with coupled mass/energy exchange
 * 
 * Models fire dynamics with three stratified zones:
 * - Upper: Hot smoke layer (0.2-0.4 of height)
 * - Middle: Transition layer (0.3-0.4 of height)
 * - Lower: Cool ambient layer (0.2-0.4 of height)
 * 
 * Includes buoyancy-driven inter-zone flow, heat transfer,
 * and species transport across zone boundaries.
 */
class ThreeZoneModel {
public:
    /**
     * @brief Construct three-zone model
     * @param total_height_m Total vertical height (m)
     * @param floor_area_m2 Floor area (m²)
     * @param num_species Number of chemical species to track
     */
    ThreeZoneModel(double total_height_m, 
                   double floor_area_m2,
                   int num_species);
    
    /**
     * @brief Advance model one timestep
     * @param dt Timestep (s)
     * @param combustion_HRR_W Heat release rate from combustion (W)
     * @param cooling_W Wall heat loss (W)
     * @param ACH Air changes per hour
     */
    void step(double dt, 
              double combustion_HRR_W,
              double cooling_W,
              double ACH);
    
    /**
     * @brief Reset to ambient conditions
     * @param T_amb Ambient temperature (K)
     * @param P_amb Ambient pressure (Pa)
     */
    void reset(double T_amb = 293.15, double P_amb = 101325.0);
    
    // Zone access
    const Zone& upperZone() const { return upper_; }
    const Zone& middleZone() const { return middle_; }
    const Zone& lowerZone() const { return lower_; }
    
    /**
     * @brief Get smoke layer height from floor
     * @return Height in meters
     */
    double smokeLayerHeight_m() const;
    
    /**
     * @brief Get mass-weighted average temperature
     * @return Temperature in K
     */
    double averageTemperature_K() const;
    
    /**
     * @brief Get total mass across all zones
     * @return Mass in kg
     */
    double totalMass_kg() const;
    
    /**
     * @brief Get total energy across all zones
     * @return Energy in J
     */
    double totalEnergy_J() const;
    
private:
    Zone upper_;   ///< Hot smoke layer
    Zone middle_;  ///< Transition layer
    Zone lower_;   ///< Cool ambient layer
    
    double total_height_m_;
    double floor_area_m2_;
    int num_species_;
    
    // Zone coupling parameters
    double k_exchange_;      ///< Mass exchange coefficient
    double h_interface_;     ///< Heat transfer coefficient at interfaces
    
    /**
     * @brief Update zone boundary heights based on density
     */
    void updateZoneBoundaries();
    
    /**
     * @brief Calculate and apply inter-zone mass exchange
     * @param dt Timestep (s)
     */
    void updateMassExchange(double dt);
    
    /**
     * @brief Calculate and apply inter-zone heat transfer
     * @param dt Timestep (s)
     */
    void updateHeatTransfer(double dt, double cooling_W);
    
    /**
     * @brief Update species distribution across zones
     * @param dt Timestep (s)
     */
    void updateSpeciesTransport(double dt);
    
    /**
     * @brief Apply heat release to upper zone
     * @param dt Timestep (s)
     * @param HRR_W Heat release rate (W)
     */
    void applyHeatRelease(double dt, double HRR_W);
    
    /**
     * @brief Apply ventilation effects
     * @param dt Timestep (s)
     * @param ACH Air changes per hour
     */
    void applyVentilation(double dt, double ACH);
};

} // namespace vfep
