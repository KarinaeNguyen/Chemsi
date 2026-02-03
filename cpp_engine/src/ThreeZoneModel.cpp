#include "ThreeZoneModel.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace vfep {

// Constants
constexpr double R_GAS = 8.314;          // J/(mol·K)
constexpr double MW_AIR = 0.029;         // kg/mol
constexpr double CP_AIR = 1005.0;        // J/(kg·K)
constexpr double G_ACCEL = 9.81;         // m/s²

// Zone implementation
double Zone::heatContent_J() const {
    double mass = mass_kg();
    return mass * CP_AIR * T_K;
}

double Zone::density_kg_m3() const {
    if (volume_m3 <= 0.0) return 0.0;
    double total_n_mol = 0.0;
    for (double n : n_mol) {
        total_n_mol += n;
    }
    double mass = total_n_mol * MW_AIR;
    return mass / volume_m3;
}

double Zone::mass_kg() const {
    double total_n_mol = 0.0;
    for (double n : n_mol) {
        total_n_mol += n;
    }
    return total_n_mol * MW_AIR;
}

// ThreeZoneModel implementation
ThreeZoneModel::ThreeZoneModel(double total_height_m, 
                               double floor_area_m2,
                               int num_species)
    : total_height_m_(total_height_m)
    , floor_area_m2_(floor_area_m2)
    , num_species_(num_species)
    , k_exchange_(0.1)
    , h_interface_(10.0)
{
    if (total_height_m <= 0.0 || floor_area_m2 <= 0.0) {
        throw std::invalid_argument("ThreeZoneModel: geometry must be positive");
    }
    
    // Initialize zones with equal heights
    upper_.height_m = total_height_m_ * 0.3;
    middle_.height_m = total_height_m_ * 0.4;
    lower_.height_m = total_height_m_ * 0.3;
    
    upper_.volume_m3 = upper_.height_m * floor_area_m2_;
    middle_.volume_m3 = middle_.height_m * floor_area_m2_;
    lower_.volume_m3 = lower_.height_m * floor_area_m2_;
    
    upper_.n_mol.resize(num_species_, 0.0);
    middle_.n_mol.resize(num_species_, 0.0);
    lower_.n_mol.resize(num_species_, 0.0);
    
    reset();
}

void ThreeZoneModel::reset(double T_amb, double P_amb) {
    // Set temperatures (upper slightly warmer due to stratification)
    upper_.T_K = T_amb + 10.0;
    middle_.T_K = T_amb + 5.0;
    lower_.T_K = T_amb;
    
    upper_.P_Pa = P_amb;
    middle_.P_Pa = P_amb;
    lower_.P_Pa = P_amb;
    
    // Distribute air mass proportionally to volume
    double total_volume = upper_.volume_m3 + middle_.volume_m3 + lower_.volume_m3;
    double total_n_air = (P_amb * total_volume) / (R_GAS * T_amb);
    
    upper_.n_mol[0] = (upper_.volume_m3 / total_volume) * total_n_air;
    middle_.n_mol[0] = (middle_.volume_m3 / total_volume) * total_n_air;
    lower_.n_mol[0] = (lower_.volume_m3 / total_volume) * total_n_air;
}

void ThreeZoneModel::step(double dt, 
                          double combustion_HRR_W,
                          double cooling_W,
                          double ACH) {
    if (dt <= 0.0) return;
    
    // Apply heat release to upper zone
    applyHeatRelease(dt, combustion_HRR_W);
    
    // Update zone boundaries based on density changes
    updateZoneBoundaries();
    
    // Mass exchange between zones (buoyancy-driven)
    updateMassExchange(dt);
    
    // Heat transfer between zones and to walls
    updateHeatTransfer(dt, cooling_W);
    
    // Species transport across boundaries
    updateSpeciesTransport(dt);
    
    // Ventilation (air exchange)
    applyVentilation(dt, ACH);
}

double ThreeZoneModel::smokeLayerHeight_m() const {
    // Interface between lower and middle zone
    return lower_.height_m;
}

double ThreeZoneModel::averageTemperature_K() const {
    double total_mass = totalMass_kg();
    if (total_mass <= 0.0) return 293.15;
    
    double mass_upper = upper_.mass_kg();
    double mass_middle = middle_.mass_kg();
    double mass_lower = lower_.mass_kg();
    
    return (mass_upper * upper_.T_K + 
            mass_middle * middle_.T_K + 
            mass_lower * lower_.T_K) / total_mass;
}

double ThreeZoneModel::totalMass_kg() const {
    return upper_.mass_kg() + middle_.mass_kg() + lower_.mass_kg();
}

double ThreeZoneModel::totalEnergy_J() const {
    return upper_.heatContent_J() + middle_.heatContent_J() + lower_.heatContent_J();
}

void ThreeZoneModel::updateZoneBoundaries() {
    // Adjust zone heights based on density (buoyancy)
    // Lighter (hotter) material rises, denser sinks
    
    double rho_upper = upper_.density_kg_m3();
    double rho_middle = middle_.density_kg_m3();
    double rho_lower = lower_.density_kg_m3();
    
    // Smooth adjustment (don't change too rapidly)
    double adjustment_rate = 0.05;
    
    if (rho_upper < rho_middle) {
        // Upper is lighter, expand slightly
        double delta_h = adjustment_rate * 0.1;
        upper_.height_m = std::min(upper_.height_m + delta_h, total_height_m_ * 0.5);
    }
    
    if (rho_middle < rho_lower) {
        // Middle is lighter than lower
        double delta_h = adjustment_rate * 0.1;
        middle_.height_m = std::max(middle_.height_m + delta_h, total_height_m_ * 0.2);
    }
    
    // Ensure heights sum to total
    double sum_h = upper_.height_m + middle_.height_m + lower_.height_m;
    if (sum_h != total_height_m_) {
        double scale = total_height_m_ / sum_h;
        upper_.height_m *= scale;
        middle_.height_m *= scale;
        lower_.height_m *= scale;
    }
    
    // Update volumes
    upper_.volume_m3 = upper_.height_m * floor_area_m2_;
    middle_.volume_m3 = middle_.height_m * floor_area_m2_;
    lower_.volume_m3 = lower_.height_m * floor_area_m2_;
}

void ThreeZoneModel::updateMassExchange(double dt) {
    // Buoyancy-driven flow: hot rises, cold sinks
    
    double rho_upper = upper_.density_kg_m3();
    double rho_middle = middle_.density_kg_m3();
    double rho_lower = lower_.density_kg_m3();
    
    double A_interface = floor_area_m2_;
    
    // Upper to middle flow (if upper is lighter)
    if (rho_upper < rho_middle) {
        double delta_rho = std::abs(rho_middle - rho_upper);
        double mdot = k_exchange_ * A_interface * std::sqrt(G_ACCEL * delta_rho * upper_.height_m);
        mdot = std::min(mdot, upper_.mass_kg() / dt * 0.1); // Limit to 10% per timestep
        
        double n_transfer = (mdot * dt) / MW_AIR;
        double fraction = n_transfer / std::max(1e-12, upper_.n_mol[0]);
        fraction = std::min(fraction, 0.1);
        
        // Transfer mass and energy
        for (int i = 0; i < num_species_; ++i) {
            double dn = upper_.n_mol[i] * fraction;
            upper_.n_mol[i] -= dn;
            middle_.n_mol[i] += dn;
        }
    }
    
    // Middle to lower flow (if middle is lighter)
    if (rho_middle < rho_lower) {
        double delta_rho = std::abs(rho_lower - rho_middle);
        double mdot = k_exchange_ * A_interface * std::sqrt(G_ACCEL * delta_rho * middle_.height_m);
        mdot = std::min(mdot, middle_.mass_kg() / dt * 0.1);
        
        double n_transfer = (mdot * dt) / MW_AIR;
        double fraction = n_transfer / std::max(1e-12, middle_.n_mol[0]);
        fraction = std::min(fraction, 0.1);
        
        for (int i = 0; i < num_species_; ++i) {
            double dn = middle_.n_mol[i] * fraction;
            middle_.n_mol[i] -= dn;
            lower_.n_mol[i] += dn;
        }
    }
}

void ThreeZoneModel::updateHeatTransfer(double dt, double cooling_W) {
    // Zone-to-zone conduction/radiation
    double Q_upper_middle = h_interface_ * floor_area_m2_ * (upper_.T_K - middle_.T_K);
    double Q_middle_lower = h_interface_ * floor_area_m2_ * (middle_.T_K - lower_.T_K);
    
    // Apply heat flows
    double mass_upper = std::max(1e-6, upper_.mass_kg());
    double mass_middle = std::max(1e-6, middle_.mass_kg());
    double mass_lower = std::max(1e-6, lower_.mass_kg());
    
    upper_.T_K -= (Q_upper_middle * dt) / (mass_upper * CP_AIR);
    middle_.T_K += (Q_upper_middle * dt) / (mass_middle * CP_AIR);
    
    middle_.T_K -= (Q_middle_lower * dt) / (mass_middle * CP_AIR);
    lower_.T_K += (Q_middle_lower * dt) / (mass_lower * CP_AIR);
    
    // Wall heat loss (distributed across zones proportional to area)
    double total_surface = 2.0 * floor_area_m2_ + 
                          2.0 * std::sqrt(floor_area_m2_) * total_height_m_;
    
    double cooling_upper = cooling_W * (upper_.height_m / total_height_m_);
    double cooling_middle = cooling_W * (middle_.height_m / total_height_m_);
    double cooling_lower = cooling_W * (lower_.height_m / total_height_m_);
    
    upper_.T_K -= (cooling_upper * dt) / (mass_upper * CP_AIR);
    middle_.T_K -= (cooling_middle * dt) / (mass_middle * CP_AIR);
    lower_.T_K -= (cooling_lower * dt) / (mass_lower * CP_AIR);
    
    // Prevent temperatures from going below ambient
    upper_.T_K = std::max(upper_.T_K, 273.15);
    middle_.T_K = std::max(middle_.T_K, 273.15);
    lower_.T_K = std::max(lower_.T_K, 273.15);
}

void ThreeZoneModel::updateSpeciesTransport(double dt) {
    // Diffusion-driven species transport
    // Simplified: follow mass exchange patterns
    // (Already handled in updateMassExchange)
}

void ThreeZoneModel::applyHeatRelease(double dt, double HRR_W) {
    if (HRR_W <= 0.0) return;
    
    // Add heat to upper zone (fire plume concentrates here)
    double Q_release = HRR_W * dt;
    double mass_upper = std::max(1e-6, upper_.mass_kg());
    upper_.T_K += Q_release / (mass_upper * CP_AIR);
}

void ThreeZoneModel::applyVentilation(double dt, double ACH) {
    if (ACH <= 0.0) return;
    
    // Ventilation: fresh air enters lower, exhaust from upper
    double total_volume = upper_.volume_m3 + middle_.volume_m3 + lower_.volume_m3;
    double exchange_rate_s = ACH / 3600.0;
    double volume_exchanged = total_volume * exchange_rate_s * dt;
    
    // Moles of air exchanged
    double T_amb = 293.15;
    double P_amb = 101325.0;
    double n_fresh = (P_amb * volume_exchanged) / (R_GAS * T_amb);
    
    // Remove from upper (exhaust hot air)
    double fraction_remove = std::min(0.1, n_fresh / std::max(1e-12, upper_.n_mol[0]));
    for (int i = 0; i < num_species_; ++i) {
        upper_.n_mol[i] *= (1.0 - fraction_remove);
    }
    
    // Add to lower (fresh air inlet)
    lower_.n_mol[0] += n_fresh;
    
    // Cool upper zone slightly due to air exchange
    upper_.T_K = upper_.T_K * (1.0 - fraction_remove) + T_amb * fraction_remove;
}

} // namespace vfep
