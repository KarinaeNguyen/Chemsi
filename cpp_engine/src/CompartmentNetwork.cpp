/**
 * @file CompartmentNetwork.cpp
 * @brief Implementation of multi-compartment fire network model
 * 
 * Phase 9: Track B - Multi-Compartment
 * 
 * Implements pressure-driven inter-compartment flow using:
 * - Bernoulli equation for flow through openings
 * - Buoyancy-driven stack effects
 * - Mass and energy conservation across network
 */

#include "CompartmentNetwork.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace vfep {

// Physical constants
constexpr float G_ACCEL = 9.81f;        // Gravitational acceleration (m/s²)
constexpr float ATM_PRESSURE = 101325.0f; // Atmospheric pressure (Pa)
constexpr float R_GAS = 8.314f;         // Universal gas constant (J/(mol·K))

// ============================================================================
// CONSTRUCTION & INITIALIZATION
// ============================================================================

CompartmentNetwork::CompartmentNetwork() {
    reset();
}

CompartmentNetwork::~CompartmentNetwork() {
    // Vector cleanup handled automatically
}

void CompartmentNetwork::reset() {
    compartments_.clear();
    openings_.clear();
    mass_flow_.clear();
    pressures_.clear();
    last_exchange_.clear();
}

// ============================================================================
// COMPARTMENT MANAGEMENT
// ============================================================================

int CompartmentNetwork::addCompartment(const ThreeZoneModel& initial_state) {
    compartments_.push_back(initial_state);
    int id = static_cast<int>(compartments_.size() - 1);
    
    // Resize mass flow matrix and pressure vector
    int n = static_cast<int>(compartments_.size());
    mass_flow_.resize(n, std::vector<float>(n, 0.0f));
    pressures_.resize(n, ATM_PRESSURE);
    
    return id;
}

Opening& CompartmentNetwork::addOpening(const Opening& opening) {
    // Validate opening parameters
    if (opening.from_compartment < 0 || 
        opening.from_compartment >= static_cast<int>(compartments_.size())) {
        throw std::invalid_argument("Invalid from_compartment ID");
    }
    if (opening.to_compartment < 0 || 
        opening.to_compartment >= static_cast<int>(compartments_.size())) {
        throw std::invalid_argument("Invalid to_compartment ID");
    }
    if (opening.height_m <= 0.0f || opening.width_m <= 0.0f) {
        throw std::invalid_argument("Opening dimensions must be positive");
    }
    if (opening.discharge_coeff <= 0.0f || opening.discharge_coeff > 1.0f) {
        throw std::invalid_argument("Discharge coefficient must be in (0, 1]");
    }
    
    openings_.push_back(opening);
    return openings_.back();
}

// ============================================================================
// SIMULATION
// ============================================================================

void CompartmentNetwork::step(float dt, const std::vector<float>& HRR_W) {
    if (HRR_W.size() != compartments_.size()) {
        throw std::invalid_argument("HRR_W size must match number of compartments");
    }
    
    // Step 1: Calculate pressure differences between compartments
    calculatePressures();
    
    // Step 2: Calculate inter-compartment mass flow based on pressures
    calculateMassFlow();
    
    // Step 3: Step each compartment forward with its HRR and mass exchanges
    last_exchange_.assign(compartments_.size(), ExchangeSummary{});
    for (size_t i = 0; i < compartments_.size(); ++i) {
        // Get net mass inflow/outflow for this compartment
        float mass_in = 0.0f;
        float mass_out = 0.0f;
        float enthalpy_in = 0.0f;
        float enthalpy_out = 0.0f;
        
        for (size_t j = 0; j < compartments_.size(); ++j) {
            if (i != j) {
                float flow_j_to_i = mass_flow_[j][i];
                float flow_i_to_j = mass_flow_[i][j];
                
                if (flow_j_to_i > 0.0f) {
                    mass_in += flow_j_to_i * dt;
                    // Enthalpy transfer: h = cp * T * m
                    // Use upper zone temperature of source compartment as representative
                    float T_source = static_cast<float>(compartments_[j].upperZone().T_K);
                    float cp = 1005.0f; // Air specific heat (J/(kg·K))
                    enthalpy_in += cp * T_source * flow_j_to_i * dt;
                }
                if (flow_i_to_j > 0.0f) {
                    mass_out += flow_i_to_j * dt;
                    float T_sink = static_cast<float>(compartments_[i].upperZone().T_K);
                    float cp = 1005.0f;
                    enthalpy_out += cp * T_sink * flow_i_to_j * dt;
                }
            }
        }

        // Estimate ventilation rate from total outflow
        const Zone& upper = compartments_[i].upperZone();
        const Zone& middle = compartments_[i].middleZone();
        const Zone& lower = compartments_[i].lowerZone();

        double volume_m3 = upper.volume_m3 + middle.volume_m3 + lower.volume_m3;
        double rho_avg = 0.0;
        if (volume_m3 > 0.0) {
            rho_avg = (upper.density_kg_m3() * upper.volume_m3 +
                       middle.density_kg_m3() * middle.volume_m3 +
                       lower.density_kg_m3() * lower.volume_m3) / volume_m3;
        }

        double vol_flow_m3_s = (rho_avg > 0.0) ? (mass_out / rho_avg) : 0.0;
        double ACH_exchange = (volume_m3 > 0.0) ? (vol_flow_m3_s / volume_m3) * 3600.0 : 0.0;
        
        // Step the compartment with fire HRR
        // ThreeZoneModel.step(dt, HRR_W, cooling_W, ACH)
        double cooling_W = 5000.0;  // Baseline wall heat loss
        double ACH = 0.5 + ACH_exchange; // Include exchange-driven ventilation

        double net_exchange_W = (dt > 0.0f) ? static_cast<double>((enthalpy_in - enthalpy_out) / dt) : 0.0;
        double effective_HRR_W = static_cast<double>(HRR_W[i]) + net_exchange_W;

        ExchangeSummary summary;
        summary.mass_in_kg = mass_in;
        summary.mass_out_kg = mass_out;
        summary.ach = static_cast<float>(ACH);
        summary.net_exchange_W = static_cast<float>(net_exchange_W);
        summary.enthalpy_in_J = enthalpy_in;
        summary.enthalpy_out_J = enthalpy_out;
        last_exchange_[i] = summary;

        compartments_[i].step(static_cast<double>(dt),
                      effective_HRR_W,
                      cooling_W,
                      ACH);
        
        // Mass/species redistribution is not yet modeled explicitly.
    }
}

// ============================================================================
// DATA ACCESS
// ============================================================================

const ThreeZoneModel& CompartmentNetwork::getCompartment(int id) const {
    if (id < 0 || id >= static_cast<int>(compartments_.size())) {
        throw std::out_of_range("Invalid compartment ID");
    }
    return compartments_[id];
}

float CompartmentNetwork::getInterCompartmentFlow(int from_id, int to_id) const {
    int n = static_cast<int>(compartments_.size());
    
    if (from_id < 0 || from_id >= n) {
        throw std::out_of_range("Invalid from_id");
    }
    if (to_id < 0 || to_id >= n) {
        throw std::out_of_range("Invalid to_id");
    }
    
    // Check if mass_flow_ matrix is properly sized
    if (mass_flow_.size() != static_cast<size_t>(n) || 
        (n > 0 && mass_flow_[0].size() != static_cast<size_t>(n))) {
        return 0.0f;  // Matrix not initialized yet
    }
    
    return mass_flow_[from_id][to_id];
}

float CompartmentNetwork::getCompartmentPressure(int id) const {
    if (id < 0 || id >= static_cast<int>(pressures_.size())) {
        throw std::out_of_range("Invalid compartment ID");
    }
    return pressures_[id];
}

const std::vector<CompartmentNetwork::ExchangeSummary>& CompartmentNetwork::getLastExchangeSummary() const {
    return last_exchange_;
}

bool CompartmentNetwork::exportExchangeCSV(const std::string& filename, float time_s) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        return false;
    }

    out << "time_s,compartment_id,pressure_Pa,mass_in_kg,mass_out_kg,ACH,net_exchange_W,enthalpy_in_J,enthalpy_out_J\n";
    out << std::fixed << std::setprecision(6);

    const int n = static_cast<int>(compartments_.size());
    for (int i = 0; i < n; ++i) {
        ExchangeSummary summary;
        if (i >= 0 && i < static_cast<int>(last_exchange_.size())) {
            summary = last_exchange_[i];
        }

        float pressure = (i >= 0 && i < static_cast<int>(pressures_.size())) ? pressures_[i] : ATM_PRESSURE;

        out << time_s << ','
            << i << ','
            << pressure << ','
            << summary.mass_in_kg << ','
            << summary.mass_out_kg << ','
            << summary.ach << ','
            << summary.net_exchange_W << ','
            << summary.enthalpy_in_J << ','
            << summary.enthalpy_out_J << '\n';
    }

    return true;
}

// ============================================================================
// PRIVATE METHODS - PHYSICS CALCULATIONS
// ============================================================================

void CompartmentNetwork::calculatePressures() {
    // Calculate pressure in each compartment based on temperature and density
    for (size_t i = 0; i < compartments_.size(); ++i) {
        // Use upper zone as representative (hot zone drives pressure)
        const Zone& upper = compartments_[i].upperZone();
        
        // Ideal gas law: P = ρ * R * T / M
        // Simplified: Use average zone pressure, add buoyancy contribution
        double rho = upper.density_kg_m3();
        double T = upper.T_K;
        double height = upper.height_m;
        
        // Pressure = atmospheric + hydrostatic + thermal expansion effects
        float delta_P = static_cast<float>(rho * G_ACCEL * height);
        
        // Add thermal expansion contribution (hot gas creates overpressure)
        float T_ref = 298.15f; // Reference temperature (25°C)
        float thermal_expansion = ATM_PRESSURE * (static_cast<float>(T) / T_ref - 1.0f) * 0.1f;
        
        pressures_[i] = ATM_PRESSURE + delta_P + thermal_expansion;
        
        // Clamp to reasonable range
        pressures_[i] = std::max(ATM_PRESSURE * 0.95f, 
                                 std::min(ATM_PRESSURE * 1.10f, pressures_[i]));
    }
}

void CompartmentNetwork::calculateMassFlow() {
    // Initialize mass flow matrix to zero
    int n = static_cast<int>(compartments_.size());
    
    // Ensure matrix is properly sized
    if (mass_flow_.size() != static_cast<size_t>(n)) {
        mass_flow_.assign(n, std::vector<float>(n, 0.0f));
    } else {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                mass_flow_[i][j] = 0.0f;
            }
        }
    }
    
    // Calculate flow through each opening
    for (const auto& opening : openings_) {
        int i = opening.from_compartment;
        int j = opening.to_compartment;
        
        // Get pressure difference
        float P_i = pressures_[i];
        float P_j = pressures_[j];
        float delta_P = P_i - P_j;
        
        // Get densities (use upper zone as representative)
        double rho_i = compartments_[i].upperZone().density_kg_m3();
        double rho_j = compartments_[j].upperZone().density_kg_m3();
        double rho_avg = (rho_i + rho_j) / 2.0;
        
        // Bernoulli flow through opening: m_dot = C_d * A * sqrt(2 * rho * |ΔP|)
        // Direction: from high pressure to low pressure
        if (std::abs(delta_P) > 0.01f) { // Threshold to avoid numerical noise (lowered from 0.1)
            float velocity = std::sqrt(2.0f * std::abs(delta_P) / static_cast<float>(rho_avg));
            float area = opening.getArea();
            float mass_flow_rate = opening.discharge_coeff * area * 
                                   static_cast<float>(rho_avg) * velocity;
            
            // Assign flow direction
            if (delta_P > 0.0f) {
                // Flow from i to j
                mass_flow_[i][j] = mass_flow_rate;
                mass_flow_[j][i] = 0.0f;
            } else {
                // Flow from j to i
                mass_flow_[j][i] = mass_flow_rate;
                mass_flow_[i][j] = 0.0f;
            }
        }
    }
    
    // Add buoyancy-driven flow (stack effect through vertical openings)
    for (const auto& opening : openings_) {
        int i = opening.from_compartment;
        int j = opening.to_compartment;
        
        // Get temperature difference
        double T_i = compartments_[i].upperZone().T_K;
        double T_j = compartments_[j].upperZone().T_K;
        double delta_T = T_i - T_j;
        
        // Stack effect: hot air rises, creating pressure difference
        // Additional flow contribution: m_dot_stack ∝ sqrt(ΔT * height)
        if (std::abs(delta_T) > 10.0) { // Threshold: 10K temperature difference
            double rho_avg = (compartments_[i].upperZone().density_kg_m3() + 
                             compartments_[j].upperZone().density_kg_m3()) / 2.0;
            double T_avg = (T_i + T_j) / 2.0;
            
            // Buoyancy velocity: v = sqrt(2 * g * h * ΔT / T)
            float v_buoyancy = std::sqrt(2.0f * G_ACCEL * opening.height_m * 
                                        std::abs(static_cast<float>(delta_T)) / 
                                        static_cast<float>(T_avg));
            float mass_flow_buoyancy = opening.discharge_coeff * opening.getArea() * 
                                       static_cast<float>(rho_avg) * v_buoyancy * 0.5f;
            
            // Add to existing flow (hot to cold)
            if (delta_T > 0.0) {
                mass_flow_[i][j] += mass_flow_buoyancy;
            } else {
                mass_flow_[j][i] += mass_flow_buoyancy;
            }
        }
    }
}

} // namespace vfep
