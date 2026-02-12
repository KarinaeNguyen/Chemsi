/**
 * @file FlameSpreadModel.cpp
 * @brief Implementation of flame spread on solid surfaces
 * 
 * Phase 9: Track D - Flame Spread
 * 
 * Implements flame propagation modeling:
 * - Ignition threshold evaluation based on temperature
 * - Surface-to-surface flame spread
 * - Heat release rate calculation from burning surfaces
 * - Material property handling (HRRPUA, ignition temperature)
 */

#include "FlameSpreadModel.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace vfep {

// Physical constants
constexpr float MIN_IGNITION_TEMP = 373.15f;  // 100°C (minimum realistic)
constexpr float MAX_IGNITION_TEMP = 1273.15f; // 1000°C (maximum realistic)
constexpr float MAX_HRRPUA = 5000.0f;         // 5 MW/m² (very intense fire)
constexpr float SPREAD_DISTANCE = 0.5f;       // Flame spreads to surfaces within 0.5m
constexpr float MIN_SPREAD_DISTANCE = 0.05f;  // Avoid singularity at very short distances
constexpr float HEAT_FLUX_TO_TEMP_COEFF = 0.015f; // Empirical temp rise per (W/m²·s)
constexpr float MIN_FUEL_LOAD_KG = 1e-5f;

// ============================================================================
// CONSTRUCTION & INITIALIZATION
// ============================================================================

FlameSpreadModel::FlameSpreadModel() {
    reset();
}

FlameSpreadModel::~FlameSpreadModel() {
    // Vector cleanup handled automatically
}

void FlameSpreadModel::reset() {
    surfaces_.clear();
}

// ============================================================================
// SURFACE MANAGEMENT
// ============================================================================

int FlameSpreadModel::addSurface(const FlammableSurface& surface) {
    // Validate surface properties
    if (surface.area_m2 <= 0.0f) {
        throw std::invalid_argument("Surface area must be positive");
    }
    if (surface.temperature_K <= 0.0f) {
        throw std::invalid_argument("Temperature must be positive");
    }
    if (surface.ignition_temp_K < MIN_IGNITION_TEMP || 
        surface.ignition_temp_K > MAX_IGNITION_TEMP) {
        throw std::invalid_argument("Ignition temperature out of realistic range");
    }
    if (surface.hrrpua_W_m2 < 0.0f || surface.hrrpua_W_m2 > MAX_HRRPUA) {
        throw std::invalid_argument("HRRPUA out of realistic range");
    }
    if (surface.fuel_load_kg < 0.0f) {
        throw std::invalid_argument("Fuel load must be non-negative");
    }
    if (surface.mass_loss_rate_kg_s < 0.0f) {
        throw std::invalid_argument("Mass loss rate must be non-negative");
    }
    if (surface.initial_fuel_load_kg < 0.0f) {
        throw std::invalid_argument("Initial fuel load must be non-negative");
    }
    
    FlammableSurface normalized = surface;
    normalized.is_burning = false;
    normalized.burn_time_s = 0.0f;
    if (normalized.initial_fuel_load_kg <= 0.0f) {
        normalized.initial_fuel_load_kg = std::max(normalized.fuel_load_kg, MIN_FUEL_LOAD_KG);
    }
    if (normalized.initial_fuel_load_kg < normalized.fuel_load_kg) {
        normalized.initial_fuel_load_kg = normalized.fuel_load_kg;
    }

    surfaces_.push_back(normalized);
    return static_cast<int>(surfaces_.size() - 1);
}

void FlameSpreadModel::setSurfaceTemperature(int surface_id, float temp_K) {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    if (temp_K <= 0.0f) {
        throw std::invalid_argument("Temperature must be positive");
    }
    
    surfaces_[surface_id].temperature_K = temp_K;
    
    // Check if heating causes ignition
    checkIgnitionCriteria(0.0f);  // Immediate check
}

// ============================================================================
// FLAME DYNAMICS
// ============================================================================

void FlameSpreadModel::updateFlameSpread(float dt) {
    if (dt <= 0.0f) {
        throw std::invalid_argument("Timestep must be positive");
    }
    
    // Step 1: Update burn time for all burning surfaces
    for (auto& surface : surfaces_) {
        if (surface.is_burning) {
            surface.burn_time_s += dt;
            if (surface.mass_loss_rate_kg_s > 0.0f) {
                surface.fuel_load_kg = std::max(0.0f, surface.fuel_load_kg - surface.mass_loss_rate_kg_s * dt);
                if (surface.fuel_load_kg <= MIN_FUEL_LOAD_KG) {
                    surface.is_burning = false;
                }
            }
        }
    }
    
    // Step 2: Check for new ignitions based on temperature
    checkIgnitionCriteria(dt);
    
    // Step 3: Propagate flame to nearby surfaces
    propagateFlame(dt);
}

void FlameSpreadModel::igniteAtLocation(int surface_id) {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    
    FlammableSurface& surface = surfaces_[surface_id];
    
    if (!surface.is_burning) {
        surface.is_burning = true;
        surface.burn_time_s = 0.0f;
    }
}

void FlameSpreadModel::extinguish(int surface_id) {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    
    FlammableSurface& surface = surfaces_[surface_id];
    surface.is_burning = false;
    // Note: We keep burn_time_s to track cumulative burn duration
}

// ============================================================================
// HEAT FEEDBACK
// ============================================================================

float FlameSpreadModel::getTotalHeatReleaseRate() const {
    float total_hrr = 0.0f;
    
    for (const auto& surface : surfaces_) {
        if (surface.is_burning) {
            float fuel_factor = 0.0f;
            if (surface.fuel_load_kg > MIN_FUEL_LOAD_KG && surface.initial_fuel_load_kg > 0.0f) {
                fuel_factor = std::min(1.0f, surface.fuel_load_kg / surface.initial_fuel_load_kg);
            }
            total_hrr += surface.hrrpua_W_m2 * surface.area_m2 * fuel_factor;
        }
    }
    
    return total_hrr;
}

float FlameSpreadModel::getHeatReleaseRateFromSurface(int surface_id) const {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    
    const FlammableSurface& surface = surfaces_[surface_id];
    
    if (!surface.is_burning || surface.fuel_load_kg <= MIN_FUEL_LOAD_KG || surface.initial_fuel_load_kg <= 0.0f) {
        return 0.0f;
    }
    
    float fuel_factor = std::min(1.0f, surface.fuel_load_kg / surface.initial_fuel_load_kg);
    return surface.hrrpua_W_m2 * surface.area_m2 * fuel_factor;
}

float FlameSpreadModel::getSurfaceHeatFluxWm2(int surface_id) const {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }

    const FlammableSurface& surface = surfaces_[surface_id];
    if (!surface.is_burning || surface.fuel_load_kg <= MIN_FUEL_LOAD_KG || surface.initial_fuel_load_kg <= 0.0f) {
        return 0.0f;
    }

    float fuel_factor = std::min(1.0f, surface.fuel_load_kg / surface.initial_fuel_load_kg);
    return surface.hrrpua_W_m2 * fuel_factor;
}

// ============================================================================
// STATUS QUERIES
// ============================================================================

bool FlameSpreadModel::isSurfaceBurning(int surface_id) const {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        throw std::out_of_range("Invalid surface ID");
    }
    
    return surfaces_[surface_id].is_burning;
}

int FlameSpreadModel::getNumBurningSurfaces() const {
    int count = 0;
    for (const auto& surface : surfaces_) {
        if (surface.is_burning && surface.fuel_load_kg > MIN_FUEL_LOAD_KG) {
            ++count;
        }
    }
    return count;
}

// ============================================================================
// PRIVATE METHODS - IGNITION & SPREAD LOGIC
// ============================================================================

void FlameSpreadModel::checkIgnitionCriteria(float dt) {
    // Check each surface for ignition conditions
    for (size_t i = 0; i < surfaces_.size(); ++i) {
        if (!surfaces_[i].is_burning && canIgnite(static_cast<int>(i))) {
            surfaces_[i].is_burning = true;
            surfaces_[i].burn_time_s = 0.0f;
        }
    }
}

bool FlameSpreadModel::canIgnite(int surface_id) const {
    if (surface_id < 0 || surface_id >= static_cast<int>(surfaces_.size())) {
        return false;
    }
    
    const FlammableSurface& surface = surfaces_[surface_id];
    
    // Already burning
    if (surface.is_burning) {
        return false;
    }
    
    // Check if temperature exceeds ignition threshold
    if (surface.temperature_K >= surface.ignition_temp_K) {
        return true;
    }
    
    // Check if nearby burning surfaces provide enough radiant heat
    // (Simplified: if any nearby surface is burning, temperature will rise)
    for (size_t i = 0; i < surfaces_.size(); ++i) {
        if (i == static_cast<size_t>(surface_id)) {
            continue;
        }
        
        const FlammableSurface& other = surfaces_[i];
        if (!other.is_burning) {
            continue;
        }
        
        // Calculate distance between surfaces
        float dx = surface.x_m - other.x_m;
        float dy = surface.y_m - other.y_m;
        float dz = surface.z_m - other.z_m;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        // If burning surface is nearby, assume radiative heating
        if (distance < SPREAD_DISTANCE) {
            // Simplified: if very close and surface is warm, can ignite
            if (surface.temperature_K > surface.ignition_temp_K * 0.8f) {
                return true;
            }
        }
    }
    
    return false;
}

void FlameSpreadModel::propagateFlame(float dt) {
    // Find all currently burning surfaces
    std::vector<int> burning_ids;
    for (size_t i = 0; i < surfaces_.size(); ++i) {
        if (surfaces_[i].is_burning) {
            burning_ids.push_back(static_cast<int>(i));
        }
    }
    
    // Check each burning surface for potential spread to neighbors
    for (int burning_id : burning_ids) {
        const FlammableSurface& burning_surface = surfaces_[burning_id];
        
        // Check all other surfaces
        for (size_t target_id = 0; target_id < surfaces_.size(); ++target_id) {
            if (static_cast<int>(target_id) == burning_id) {
                continue;
            }
            
            FlammableSurface& target = surfaces_[target_id];
            if (target.is_burning) {
                continue;  // Already burning
            }
            
            // Calculate distance
            float dx = target.x_m - burning_surface.x_m;
            float dy = target.y_m - burning_surface.y_m;
            float dz = target.z_m - burning_surface.z_m;
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            distance = std::max(distance, MIN_SPREAD_DISTANCE);
            
            // Flame spread criterion: nearby surfaces heat up from radiation
            if (distance < SPREAD_DISTANCE) {
                float fuel_factor = 0.0f;
                if (burning_surface.fuel_load_kg > MIN_FUEL_LOAD_KG && burning_surface.initial_fuel_load_kg > 0.0f) {
                    fuel_factor = std::min(1.0f, burning_surface.fuel_load_kg / burning_surface.initial_fuel_load_kg);
                }

                // Estimate radiative heating
                // q_rad ∝ 1/r² for point source
                float heat_flux = burning_surface.hrrpua_W_m2 * 
                                 burning_surface.area_m2 * fuel_factor / 
                                 (4.0f * 3.14159f * distance * distance);
                
                // Simplified: raise temperature based on heat flux and timestep
                float temp_rise = heat_flux * HEAT_FLUX_TO_TEMP_COEFF * dt;
                target.temperature_K += temp_rise;
                
                // Check if this heating causes ignition
                if (target.temperature_K >= target.ignition_temp_K) {
                    target.is_burning = true;
                    target.burn_time_s = 0.0f;
                }
            }
        }
    }
}

} // namespace vfep
