/**
 * @file RadiationModel.h
 * @brief Radiation heat transfer model for fire simulation
 * 
 * Phase 9: Advanced Physics
 * 
 * Provides:
 * - View factor calculation between surfaces
 * - Radiative heat exchange between zones and walls
 * - Participating media (smoke) absorption
 * - Surface emissivity and absorptivity handling
 * 
 * @author Chemsi Development Team
 * @version 1.0
 */

#ifndef CHEMSI_RADIATION_MODEL_H
#define CHEMSI_RADIATION_MODEL_H

#include <vector>
#include <cmath>

namespace vfep {

/**
 * @struct Surface
 * @brief Represents a radiating surface element
 */
struct Surface {
    float area_m2;           ///< Surface area in m²
    float temperature_K;     ///< Surface temperature in K
    float emissivity;        ///< Emissivity (0.0-1.0)
    float absorptivity;      ///< Absorptivity (0.0-1.0)
    int zone_id;             ///< Which zone this surface belongs to
    
    Surface(float area = 1.0f, float temp = 298.15f, 
            float emiss = 0.9f, float absorb = 0.9f, int zone = 0)
        : area_m2(area), temperature_K(temp), 
          emissivity(emiss), absorptivity(absorb), zone_id(zone) {}
};

/**
 * @class RadiationModel
 * @brief Handles all radiative heat transfer calculations
 * 
 * Responsibilities:
 * 1. Calculate view factors between surfaces
 * 2. Compute radiative heat exchange
 * 3. Account for participating media (smoke absorption)
 * 4. Integrate with zone/compartment models
 * 
 * @example
 * RadiationModel rad;
 * rad.addSurface(Surface(10.0f, 400.0f, 0.9f, 0.9f, 0));  // Hot surface
 * rad.addSurface(Surface(50.0f, 300.0f, 0.8f, 0.8f, 0));  // Cold wall
 * rad.calculateViewFactors();
 * float heatFlux = rad.getRadiativeHeatFlux(0, 1);
 */
class RadiationModel {
public:
    // ============================================================================
    // CONSTRUCTION & INITIALIZATION
    // ============================================================================
    
    /**
     * @brief Default constructor
     * 
     * Initializes empty radiation model with no surfaces or view factors.
     */
    RadiationModel();
    
    /**
     * @brief Destructor
     */
    ~RadiationModel();
    
    /**
     * @brief Reset all surfaces and view factors
     */
    void reset();
    
    // ============================================================================
    // SURFACE MANAGEMENT
    // ============================================================================
    
    /**
     * @brief Add a radiating surface to the model
     * 
     * @param surface Surface object with area, temperature, emissivity
     * @return Surface ID (index) for later reference
     * @pre surface.area_m2 > 0
     * @pre surface.temperature_K > 0
     * @pre 0 <= surface.emissivity <= 1
     * @pre 0 <= surface.absorptivity <= 1
     */
    int addSurface(const Surface& surface);
    
    /**
     * @brief Get surface by ID
     * 
     * @param surface_id Surface identifier
     * @return Reference to surface (modifiable)
     * @throws std::out_of_range if surface_id invalid
     */
    Surface& getSurface(int surface_id);
    
    /**
     * @brief Get number of surfaces
     * 
     * @return Number of surfaces in model
     */
    int getNumSurfaces() const;
    
    /**
     * @brief Update surface temperature
     * 
     * @param surface_id Which surface to update
     * @param temperature_K New temperature in Kelvin
     * @pre temperature_K > 0
     */
    void setSurfaceTemperature(int surface_id, float temperature_K);
    
    /**
     * @brief Update surface emissivity
     * 
     * @param surface_id Which surface
     * @param emissivity New emissivity (0.0-1.0)
     */
    void setSurfaceEmissivity(int surface_id, float emissivity);
    
    // ============================================================================
    // VIEW FACTOR CALCULATION
    // ============================================================================
    
    /**
     * @brief Calculate all view factors between surfaces
     * 
     * View factors (configuration factors) determine what fraction of
     * radiation from surface i reaches surface j. Calculated using
     * reciprocity and summation rules.
     * 
     * Complexity: O(n²) where n = number of surfaces
     * 
     * @see getViewFactor()
     */
    void calculateViewFactors();
    
    /**
     * @brief Get view factor from surface i to surface j
     * 
     * Returns F_ij, the fraction of radiation from surface i that
     * reaches surface j. Always 0 <= F_ij <= 1.
     * 
     * @param from_surface Source surface ID
     * @param to_surface Target surface ID
     * @return View factor F_ij
     * @pre View factors must be calculated first (calculateViewFactors())
     * @note F_ij is NOT symmetric; F_ij != F_ji in general
     */
    float getViewFactor(int from_surface, int to_surface) const;
    
    /**
     * @brief Is view factor matrix calculated?
     * 
     * @return true if calculateViewFactors() has been called
     */
    bool isViewFactorsCalculated() const;
    
    // ============================================================================
    // RADIATIVE HEAT EXCHANGE
    // ============================================================================
    
    /**
     * @brief Get radiative heat flux FROM surface i TO surface j
     * 
     * Computes Q_ij = F_ij * ε_i * σ * A_i * (T_i^4 - T_j^4)
     * 
     * Positive value: heat flows from i to j
     * Negative value: heat flows from j to i
     * 
     * @param from_id Source surface
     * @param to_id Target surface
     * @return Heat flux in Watts
     * @pre calculateViewFactors() called previously
     * @see getRadiativeHeatExchange()
     */
    float getRadiativeHeatFlux(int from_id, int to_id) const;
    
    /**
     * @brief Get net radiative heat exchange for a surface
     * 
     * Sums all heat fluxes involving this surface (incoming and outgoing).
     * 
     * @param surface_id Which surface
     * @return Net heat to surface in Watts (positive = gaining heat)
     * @pre calculateViewFactors() called previously
     */
    float getRadiativeHeatExchange(int surface_id) const;
    
    /**
     * @brief Get total radiative power from all surfaces
     * 
     * @return Sum of radiated power (Watts)
     */
    float getTotalRadiatedPower() const;
    
    // ============================================================================
    // PARTICIPATING MEDIA (SMOKE)
    // ============================================================================
    
    /**
     * @brief Set smoke optical thickness (extinction coefficient)
     * 
     * Models smoke absorption/scattering. Higher values = more absorption.
     * Typical range: 0.0 (clear) to 0.5 (heavily smoke-filled)
     * 
     * @param optical_thickness Extinction coefficient τ
     * @pre optical_thickness >= 0.0
     */
    void setSmokeMeanBeamLength(float optical_thickness);
    
    /**
     * @brief Get smoke mean beam length
     * 
     * @return Current optical thickness value
     */
    float getSmokeMeanBeamLength() const;
    
    /**
     * @brief Calculate transmissivity through smoke
     * 
     * Uses Beer-Lambert law: τ = exp(-κ * L)
     * where κ = extinction coefficient, L = mean beam length
     * 
     * @param distance_m Mean beam length through smoke
     * @return Transmissivity (0.0-1.0)
     */
    float getTransmissivity(float distance_m) const;
    
    // ============================================================================
    // INTEGRATION WITH ZONES
    // ============================================================================
    
    /**
     * @brief Get radiative heat transfer to a specific zone
     * 
     * Sums all radiative heat exchange from surfaces in other zones
     * to surfaces in target zone.
     * 
     * @param zone_id Which zone to query
     * @return Heat flux into zone (Watts)
     */
    float getRadiativeHeatToZone(int zone_id) const;
    
    /**
     * @brief Update all surface temperatures from zone data
     * 
     * Connects zone model to radiation model by updating wall
     * temperatures based on zone temperatures.
     * 
     * @param zone_temps Vector of zone temperatures (K)
     * @param zone_ids Vector of surface zone assignments
     * @see Surface::zone_id
     */
    void updateSurfaceTemperaturesFromZones(
        const std::vector<float>& zone_temps,
        const std::vector<int>& zone_ids);
    
    // ============================================================================
    // RADIATION CONSTANTS
    // ============================================================================
    
    /// Stefan-Boltzmann constant: σ = 5.67e-8 W/(m²·K⁴)
    static constexpr float STEFAN_BOLTZMANN = 5.67e-8f;
    
    /// Absolute zero in Kelvin
    static constexpr float ABSOLUTE_ZERO_K = 273.15f;
    
private:
    // ============================================================================
    // DATA MEMBERS
    // ============================================================================
    
    std::vector<Surface> surfaces_;           ///< All surfaces in model
    std::vector<std::vector<float>> view_factors_;  ///< View factor matrix F_ij
    float smoke_tau_;                         ///< Smoke extinction coefficient
    bool view_factors_valid_;                 ///< Is F matrix up-to-date?
    
    // ============================================================================
    // PRIVATE METHODS
    // ============================================================================
    
    /**
     * @brief Compute single view factor F_ij
     * 
     * Implementation detail: uses radiosity/view factor algebra.
     * 
     * @param from_id Source surface
     * @param to_id Target surface
     * @return View factor value
     */
    float computeViewFactor(int from_id, int to_id) const;
    
    /**
     * @brief Apply reciprocity rule: F_ij * A_i = F_ji * A_j
     * 
     * Ensures view factors satisfy thermodynamic reciprocity.
     */
    void applyReciprocityRule();
    
    /**
     * @brief Validate view factors sum to 1.0 for each surface
     * 
     * @return true if all surfaces satisfy ∑_j F_ij = 1.0
     */
    bool validateViewFactorSum();
};

} // namespace vfep

#endif // CHEMSI_RADIATION_MODEL_H
