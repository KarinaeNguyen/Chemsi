/**
 * @file FlameSpreadModel.h
 * @brief Flame spread on solid surfaces
 * 
 * Phase 9: Track D - Flame Spread
 * 
 * Provides:
 * - Ignition threshold evaluation
 * - Surface flame propagation
 * - Heat feedback to fire growth model
 * - Material property handling (HRRPUA)
 * 
 * TODO: Implementation
 */

#ifndef CHEMSI_FLAME_SPREAD_MODEL_H
#define CHEMSI_FLAME_SPREAD_MODEL_H

#include <vector>

namespace vfep {

struct FlammableSurface {
    float x_m, y_m, z_m;           // Position
    float area_m2;                 // Surface area
    float temperature_K;           // Current temperature
    float ignition_temp_K;         // Ignition threshold
    float hrrpua_W_m2;            // Heat release rate per unit area
    bool is_burning;               // Flame present?
    float burn_time_s;             // How long burning
    
    FlammableSurface() : x_m(0), y_m(0), z_m(0), area_m2(1.0f),
                         temperature_K(298.15f), ignition_temp_K(573.15f),
                         hrrpua_W_m2(500.0f), is_burning(false), burn_time_s(0) {}
};

class FlameSpreadModel {
public:
    FlameSpreadModel();
    ~FlameSpreadModel();
    
    void reset();
    
    // Surface management
    int addSurface(const FlammableSurface& surface);
    void setSurfaceTemperature(int surface_id, float temp_K);
    
    // Flame dynamics
    void updateFlameSpread(float dt);
    void igniteAtLocation(int surface_id);
    void extinguish(int surface_id);
    
    // Heat feedback
    float getTotalHeatReleaseRate() const;
    float getHeatReleaseRateFromSurface(int surface_id) const;
    
    // Status queries
    bool isSurfaceBurning(int surface_id) const;
    int getNumBurningSurfaces() const;
    
    // TODO: Full implementation
    
private:
    std::vector<FlammableSurface> surfaces_;
    
    void checkIgnitionCriteria(float dt);
    bool canIgnite(int surface_id) const;
    void propagateFlame();
};

} // namespace vfep

#endif // CHEMSI_FLAME_SPREAD_MODEL_H
