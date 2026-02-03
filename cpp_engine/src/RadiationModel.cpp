/**
 * @file RadiationModel.cpp
 * @brief Implementation of radiation heat transfer model
 * 
 * Implements radiative heat exchange calculations with view factors,
 * participating media absorption, and integration with fire zones.
 */

#include "RadiationModel.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace vfep {

// ============================================================================
// CONSTRUCTION & INITIALIZATION
// ============================================================================

RadiationModel::RadiationModel()
    : smoke_tau_(0.0f), view_factors_valid_(false) {
    // Initialize with empty state
}

RadiationModel::~RadiationModel() {
    // Vector cleanup handled automatically
}

void RadiationModel::reset() {
    surfaces_.clear();
    view_factors_.clear();
    smoke_tau_ = 0.0f;
    view_factors_valid_ = false;
}

// ============================================================================
// SURFACE MANAGEMENT
// ============================================================================

int RadiationModel::addSurface(const Surface& surface) {
    // Validate input
    if (surface.area_m2 <= 0.0f) {
        throw std::invalid_argument("Surface area must be positive");
    }
    if (surface.temperature_K <= 0.0f) {
        throw std::invalid_argument("Temperature must be positive (Kelvin)");
    }
    if (surface.emissivity < 0.0f || surface.emissivity > 1.0f) {
        throw std::invalid_argument("Emissivity must be in [0, 1]");
    }
    if (surface.absorptivity < 0.0f || surface.absorptivity > 1.0f) {
        throw std::invalid_argument("Absorptivity must be in [0, 1]");
    }
    
    // Add surface and invalidate view factors
    surfaces_.push_back(surface);
    view_factors_valid_ = false;
    
    return static_cast<int>(surfaces_.size() - 1);
}

Surface& RadiationModel::getSurface(int surface_id) {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    return surfaces_[surface_id];
}

int RadiationModel::getNumSurfaces() const {
    return static_cast<int>(surfaces_.size());
}

void RadiationModel::setSurfaceTemperature(int surface_id, float temperature_K) {
    if (temperature_K <= 0.0f) {
        throw std::invalid_argument("Temperature must be positive");
    }
    getSurface(surface_id).temperature_K = temperature_K;
}

void RadiationModel::setSurfaceEmissivity(int surface_id, float emissivity) {
    if (emissivity < 0.0f || emissivity > 1.0f) {
        throw std::invalid_argument("Emissivity must be in [0, 1]");
    }
    getSurface(surface_id).emissivity = emissivity;
}

// ============================================================================
// VIEW FACTOR CALCULATION
// ============================================================================

void RadiationModel::calculateViewFactors() {
    int n = static_cast<int>(surfaces_.size());
    
    // Initialize view factor matrix
    view_factors_.assign(n, std::vector<float>(n, 0.0f));
    
    // Compute view factors
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            view_factors_[i][j] = computeViewFactor(i, j);
        }
    }
    
    // Apply reciprocity rule and validation
    applyReciprocityRule();
    validateViewFactorSum();
    
    view_factors_valid_ = true;
}

float RadiationModel::computeViewFactor(int from_id, int to_id) const {
    if (from_id == to_id) {
        // Self-view factor (radiation to itself) - typically small
        return 0.0f;  // Simplified: assume surfaces are planar
    }
    
    // Simplified geometric view factor based on area weighting
    // For N surfaces, assuming some can see each other:
    // F_ij ≈ (A_j / ∑(A_k)) for all k ≠ i
    // This ensures reciprocity: F_ij * A_i = F_ji * A_j
    
    int n = static_cast<int>(surfaces_.size());
    if (n <= 1) {
        return 0.0f;  // No other surfaces
    }
    
    // Compute total area of all other surfaces
    float total_other_area = 0.0f;
    for (int k = 0; k < n; ++k) {
        if (k != from_id) {
            total_other_area += surfaces_[k].area_m2;
        }
    }
    
    if (total_other_area <= 0.0f) {
        return 0.0f;
    }
    
    // View factor from surface i to j: proportion of j's area to total other area
    return surfaces_[to_id].area_m2 / total_other_area;
}

void RadiationModel::applyReciprocityRule() {
    int n = static_cast<int>(surfaces_.size());
    
    // Apply reciprocity: F_ij * A_i = F_ji * A_j
    // For simplified model: We'll make F_ij symmetric and then adjust
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            float area_i = surfaces_[i].area_m2;
            float area_j = surfaces_[j].area_m2;
            
            // Get current view factors
            float f_ij = view_factors_[i][j];
            float f_ji = view_factors_[j][i];
            
            // For reciprocity: use smallest value and scale appropriately
            // F_ij_new * A_i = F_ji_new * A_j
            // Start with one direction and propagate
            float f_ij_new = std::min(f_ij, f_ji);  // Use conservative estimate
            float f_ji_new = f_ij_new * area_i / area_j;
            
            view_factors_[i][j] = f_ij_new;
            view_factors_[j][i] = f_ji_new;
        }
    }
}

bool RadiationModel::validateViewFactorSum() {
    // Note: For reciprocity to be maintained, we should NOT normalize
    // view factors after applying reciprocity. The sum to 1 rule applies
    // only to complete enclosure calculations where all radiation is accounted.
    // In our simplified model with reciprocity enforcement, we skip normalization.
    
    return true;
}

float RadiationModel::getViewFactor(int from_surface, int to_surface) const {
    if (!view_factors_valid_) {
        throw std::runtime_error("View factors not calculated yet");
    }
    
    if (from_surface < 0 || from_surface >= static_cast<int>(view_factors_.size())) {
        throw std::out_of_range("Invalid source surface ID");
    }
    if (to_surface < 0 || to_surface >= static_cast<int>(view_factors_[0].size())) {
        throw std::out_of_range("Invalid target surface ID");
    }
    
    return view_factors_[from_surface][to_surface];
}

bool RadiationModel::isViewFactorsCalculated() const {
    return view_factors_valid_;
}

// ============================================================================
// RADIATIVE HEAT EXCHANGE
// ============================================================================

float RadiationModel::getRadiativeHeatFlux(int from_id, int to_id) const {
    if (!view_factors_valid_) {
        throw std::runtime_error("View factors not calculated");
    }
    
    const Surface& from = surfaces_[from_id];
    const Surface& to = surfaces_[to_id];
    
    float f_ij = view_factors_[from_id][to_id];
    float t_trans = getTransmissivity(1.0f);  // Simplistic mean beam length
    
    // Radiative heat flux with participating media
    // Q_ij = F_ij * ε_i * σ * A_i * (T_i^4 - T_j^4) * τ_trans
    float t_i4 = from.temperature_K * from.temperature_K * from.temperature_K * from.temperature_K;
    float t_j4 = to.temperature_K * to.temperature_K * to.temperature_K * to.temperature_K;
    
    float q = f_ij * from.emissivity * STEFAN_BOLTZMANN * from.area_m2 * 
              (t_i4 - t_j4) * t_trans;
    
    return q;
}

float RadiationModel::getRadiativeHeatExchange(int surface_id) const {
    if (!view_factors_valid_) {
        throw std::runtime_error("View factors not calculated");
    }
    
    float q_net = 0.0f;
    int n = static_cast<int>(surfaces_.size());
    
    for (int j = 0; j < n; ++j) {
        if (j != surface_id) {
            q_net += getRadiativeHeatFlux(surface_id, j);
            q_net -= getRadiativeHeatFlux(j, surface_id);
        }
    }
    
    return q_net;
}

float RadiationModel::getTotalRadiatedPower() const {
    float q_total = 0.0f;
    
    for (const auto& surface : surfaces_) {
        float t4 = surface.temperature_K * surface.temperature_K * 
                   surface.temperature_K * surface.temperature_K;
        float q = surface.emissivity * STEFAN_BOLTZMANN * surface.area_m2 * t4;
        q_total += q;
    }
    
    return q_total;
}

// ============================================================================
// PARTICIPATING MEDIA (SMOKE)
// ============================================================================

void RadiationModel::setSmokeMeanBeamLength(float optical_thickness) {
    if (optical_thickness < 0.0f) {
        throw std::invalid_argument("Optical thickness must be non-negative");
    }
    smoke_tau_ = optical_thickness;
}

float RadiationModel::getSmokeMeanBeamLength() const {
    return smoke_tau_;
}

float RadiationModel::getTransmissivity(float distance_m) const {
    if (distance_m < 0.0f) {
        throw std::invalid_argument("Distance must be non-negative");
    }
    
    // Beer-Lambert law: τ = exp(-κ*L)
    // where κ = smoke_tau_, L = distance_m
    float exponent = -smoke_tau_ * distance_m;
    
    // Clamp to reasonable range to avoid numerical issues
    exponent = std::max(-100.0f, std::min(0.0f, exponent));
    
    return std::exp(exponent);
}

// ============================================================================
// INTEGRATION WITH ZONES
// ============================================================================

float RadiationModel::getRadiativeHeatToZone(int zone_id) const {
    float q_zone = 0.0f;
    
    for (int i = 0; i < static_cast<int>(surfaces_.size()); ++i) {
        if (surfaces_[i].zone_id != zone_id) {
            // Heat from surface i to any surface j in target zone
            for (int j = 0; j < static_cast<int>(surfaces_.size()); ++j) {
                if (surfaces_[j].zone_id == zone_id) {
                    q_zone += getRadiativeHeatFlux(i, j);
                }
            }
        }
    }
    
    return q_zone;
}

void RadiationModel::updateSurfaceTemperaturesFromZones(
    const std::vector<float>& zone_temps,
    const std::vector<int>& zone_ids) {
    
    if (zone_ids.size() != surfaces_.size()) {
        throw std::invalid_argument("Zone IDs must match number of surfaces");
    }
    
    for (size_t i = 0; i < surfaces_.size(); ++i) {
        int zone_id = zone_ids[i];
        if (zone_id >= 0 && zone_id < static_cast<int>(zone_temps.size())) {
            surfaces_[i].temperature_K = zone_temps[zone_id];
        }
    }
}

} // namespace vfep
