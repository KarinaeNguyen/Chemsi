# Phase 9 Radiation Model Design

**Status**: Design Phase  
**Track**: A - Radiation  
**Target Tests**: 9A1, 9A2, 9A3  
**Timeline**: 3 weeks  

---

## Overview

The Radiation Model extends the single-zone/multi-zone fire models with detailed radiative heat transfer calculation. This is the foundation for accurate fire dynamics prediction.

### What It Does
- **Calculates view factors** between surfaces (geometric configuration)
- **Computes radiative heat exchange** using Stefan-Boltzmann law
- **Accounts for smoke absorption** (participating media)
- **Integrates with ThreeZoneModel** for coupled zone-radiation simulation

### Why It Matters
In fire compartments:
- **~40-60% of total heat transfer is radiative** (especially in upper zone)
- Radiation causes **secondary fires** at a distance from main fire
- Smoke absorption **dramatically reduces** long-distance radiation
- Critical for accurate **flame spread** and **ignition** predictions

---

## Architecture

### Class Structure

```
RadiationModel
├── Surfaces (vector<Surface>)
│   ├── area_m2, temperature_K
│   ├── emissivity, absorptivity
│   └── zone_id (which compartment)
├── View Factors (matrix F_ij)
├── Smoke Optical Thickness (τ)
└── Methods
    ├── Surface Management
    ├── View Factor Calculation
    ├── Heat Exchange Calculation
    └── Zone Integration
```

### Data Flow

```
Zone Model (ThreeZoneModel)
    ↓
Zone Temperatures
    ↓
RadiationModel
    ├── Update Surface Temps
    ├── Calculate View Factors (once)
    ├── Compute Heat Fluxes
    └── Sum Radiative Heat to Zones
    ↓
Radiative Heat → Zone Energy Balance
    ↓
Updated Zone Temperatures
```

---

## Implementation Plan

### Phase 9A1: View Factor Calculation
**Objective**: Test basic view factor geometry

**What to Implement**:
```cpp
RadiationModel rad;
Surface s1(10.0f, 400.0f, 0.9f, 0.9f, 0);  // Hot surface
Surface s2(50.0f, 300.0f, 0.8f, 0.8f, 0);  // Cold wall
rad.addSurface(s1);
rad.addSurface(s2);
rad.calculateViewFactors();

// Tests:
// - View factors sum to 1.0 for each surface
// - Reciprocity satisfied: F_ij * A_i ≈ F_ji * A_j
// - View factor values reasonable (0 < F_ij < 1)
```

**Test Cases**:
1. Single surface: F_ii = 0 (planar surface)
2. Two perpendicular walls: F_12 < 0.5
3. Two parallel surfaces: F > 0.5
4. Reciprocity validation

**Estimated Code**:
- `addSurface()` - 10 lines
- `calculateViewFactors()` - 20 lines
- `computeViewFactor()` - 30 lines (geometric algebra)
- `applyReciprocityRule()` - 15 lines

---

### Phase 9A2: Radiative Heat Exchange
**Objective**: Calculate heat flux between surfaces

**What to Implement**:
```cpp
// Continued from 9A1...
float q_12 = rad.getRadiativeHeatFlux(0, 1);  // Heat from S1 to S2
float q_21 = rad.getRadiativeHeatFlux(1, 0);  // Heat from S2 to S1

// Tests:
// - q_12 > 0 (hot surface radiates to cold)
// - q_21 < q_12 (cold surface radiates less)
// - q_net = q_12 - q_21 > 0 (net heat from hot to cold)
// - Stefan-Boltzmann law: q ∝ (T1^4 - T2^4)
```

**Physics Validation**:
- At same temperature: q_12 = q_21 (thermal equilibrium)
- At 400 K → 300 K: ~1000 W/m² radiated
- Doubling temperature: Q increases ~16× (T^4 relationship)

**Estimated Code**:
- `getRadiativeHeatFlux()` - 15 lines
- `getRadiativeHeatExchange()` - 10 lines
- Helper: `getTransmissivity()` - 10 lines

---

### Phase 9A3: Smoke Absorption
**Objective**: Model participating media effects

**What to Implement**:
```cpp
// Continued from 9A2...
rad.setSmokeMeanBeamLength(0.1f);  // Light smoke

float tau = rad.getTransmissivity(5.0f);  // Through 5m of smoke
// tau ≈ 0.6 means 40% of radiation absorbed by smoke

float q_with_smoke = rad.getRadiativeHeatFlux(0, 1);
// q_with_smoke < q_without_smoke due to absorption

// Tests:
// - Transmissivity decreases with smoke (τ ↓ → τ_trans ↓)
// - Beer-Lambert law: τ = exp(-κ*L)
// - Heavy smoke (κ=0.5): τ ≈ 0 (no radiation reaches target)
// - No smoke (κ=0): τ = 1 (full radiation)
```

**Physics Validation**:
- Clear air (κ=0): transmissivity = 1.0
- Light smoke (κ=0.1, L=5m): τ = e^(-0.5) ≈ 0.6
- Heavy smoke (κ=0.5, L=5m): τ = e^(-2.5) ≈ 0.08

**Estimated Code**:
- `setSmokeMeanBeamLength()` - 5 lines
- `getTransmissivity()` - 10 lines
- Modified `getRadiativeHeatFlux()` - apply τ_trans factor

---

## Test-First Development

### Test 9A1: View Factors
```cpp
TEST(PhaseNine, RadiationViewFactors_9A1) {
    vfep::RadiationModel rad;
    
    // Setup: Two surfaces in small compartment
    rad.addSurface(vfep::Surface(10.0f, 400.0f, 0.9f, 0.9f, 0));
    rad.addSurface(vfep::Surface(50.0f, 300.0f, 0.8f, 0.8f, 0));
    
    // Calculate
    rad.calculateViewFactors();
    
    // Verify: View factors between 0 and 1
    EXPECT_TRUE(rad.isViewFactorsCalculated());
    EXPECT_GE(rad.getViewFactor(0, 1), 0.0f);
    EXPECT_LE(rad.getViewFactor(0, 1), 1.0f);
    EXPECT_GE(rad.getViewFactor(1, 0), 0.0f);
    EXPECT_LE(rad.getViewFactor(1, 0), 1.0f);
    
    // Verify: Reciprocity A_i*F_ij ≈ A_j*F_ji
    float left = 10.0f * rad.getViewFactor(0, 1);
    float right = 50.0f * rad.getViewFactor(1, 0);
    EXPECT_NEAR(left, right, 0.1f);  // Within 10%
}
```

### Test 9A2: Heat Exchange
```cpp
TEST(PhaseNine, RadiationHeatExchange_9A2) {
    vfep::RadiationModel rad;
    rad.addSurface(vfep::Surface(10.0f, 400.0f, 0.9f, 0.9f, 0));
    rad.addSurface(vfep::Surface(50.0f, 300.0f, 0.8f, 0.8f, 0));
    rad.calculateViewFactors();
    
    float q_01 = rad.getRadiativeHeatFlux(0, 1);  // Hot → Cold
    float q_10 = rad.getRadiativeHeatFlux(1, 0);  // Cold → Hot
    
    EXPECT_GT(q_01, 0.0f);        // Heat flows from hot
    EXPECT_GT(q_01, q_10);        // More heat hot→cold than cold→hot
    EXPECT_LT(q_10, q_01);        // Cold radiates less
}
```

### Test 9A3: Smoke Absorption
```cpp
TEST(PhaseNine, RadiationSmoke_9A3) {
    vfep::RadiationModel rad;
    rad.addSurface(vfep::Surface(10.0f, 400.0f, 0.9f, 0.9f, 0));
    rad.addSurface(vfep::Surface(50.0f, 300.0f, 0.8f, 0.8f, 0));
    rad.calculateViewFactors();
    
    float q_clear = rad.getRadiativeHeatFlux(0, 1);
    
    rad.setSmokeMeanBeamLength(0.1f);
    float tau = rad.getTransmissivity(5.0f);
    float q_smoke = rad.getRadiativeHeatFlux(0, 1);
    
    EXPECT_LT(tau, 1.0f);        // Smoke reduces transmissivity
    EXPECT_LE(q_smoke, q_clear); // Smoke reduces heat transfer
}
```

---

## Integration with ThreeZoneModel

### Coupling Pattern

```cpp
class Simulation {
    ThreeZoneModel zones_;
    RadiationModel radiation_;
    
    void step(float dt, float HRR_W) {
        // 1. Zone step (with convection)
        zones_.step(dt, HRR_W, 0.0f);
        
        // 2. Update radiation model surfaces
        std::vector<float> zone_temps = {
            zones_.zones_[0].T_K,
            zones_.zones_[1].T_K,
            zones_.zones_[2].T_K
        };
        std::vector<int> zone_ids = {0, 0, 1, 1, 2, 2};  // Surfaces → zones
        radiation_.updateSurfaceTemperaturesFromZones(zone_temps, zone_ids);
        
        // 3. Compute radiative heat
        float q_rad_upper = radiation_.getRadiativeHeatToZone(0);
        float q_rad_middle = radiation_.getRadiativeHeatToZone(1);
        float q_rad_lower = radiation_.getRadiativeHeatToZone(2);
        
        // 4. Add to zone energy balance (next iteration)
        // This becomes part of energy balance in next step
    }
};
```

---

## Validation Scenarios

### Scenario P9.1: Radiative vs Convective Heat Transfer
- **Setup**: Single compartment with heat source, measure wall heating
- **Radiation only**: No forced ventilation
- **Expected**: Upper zone heats walls by radiation (~40% of HRR)
- **Success Criterion**: Wall temperature rise > 50 K in 60 seconds

### Scenario P9.2: Smoke Absorption Effect
- **Setup**: Large compartment, measure radiation at different distances
- **Vary**: Smoke density (optical thickness)
- **Expected**: Heavy smoke blocks distant radiation
- **Success Criterion**: Clear air radiates 5× more than heavy smoke

### Scenario P9.3: Multi-Zone Radiation Exchange
- **Setup**: Two-zone model (hot upper, cool lower)
- **Measure**: Radiative coupling between zones
- **Expected**: Upper zone radiates down, lower zone radiates up (weaker)
- **Success Criterion**: Net upward heat flow from upper to lower zone

---

## Performance Requirements

| Operation | Target | Notes |
|-----------|--------|-------|
| `addSurface()` | < 0.1 ms | O(1) |
| `calculateViewFactors()` | < 5 ms | O(n²) for n surfaces |
| `getRadiativeHeatFlux()` | < 0.01 ms | O(1), called many times |
| `getRadiativeHeatExchange()` | < 0.1 ms | O(n) |

**For 60-second simulation with 10 surfaces, 1000 steps**:
- View factor calc: once = 5 ms
- Heat flux calls: 1000 × 100 = 100 ms per simulation
- Total radiation overhead: ~10% of total simulation time ✓

---

## Known Limitations & Future Work

### Current Implementation (Phase 9)
- ✅ View factor calculation (simplified/approximate)
- ✅ Stefan-Boltzmann radiative exchange
- ✅ Participating media (smoke) absorption
- ❌ Geometric view factor integration (simplified)
- ❌ Spectral radiation (assumes gray body)
- ❌ Scattering effects

### Phase 10+ Potential
- Detailed geometric view factor calculation (solid angle integration)
- Spectral/wavelength-dependent absorption
- Scattering by particles and gases
- Real material properties database (emissivity vs temperature)
- Computational optimization for many surfaces

---

## Checklist for Phase 9A Completion

- [ ] RadiationModel.h header complete with full documentation
- [ ] RadiationModel.cpp implementation complete
- [ ] Test 9A1 written and passing (view factors)
- [ ] Test 9A2 written and passing (heat exchange)
- [ ] Test 9A3 written and passing (smoke absorption)
- [ ] All 62+ tests passing (no regressions)
- [ ] Code review complete
- [ ] Design documentation updated
- [ ] Integration with ThreeZoneModel verified
- [ ] Performance benchmarked (<5 ms view factors)
- [ ] PR created and merged

---

## Reference Materials

**Physics**:
- View factors: Incropera et al., "Fundamentals of Heat Transfer"
- Stefan-Boltzmann: Thermal radiation textbooks
- Participating media: Modest, "Radiative Heat Transfer"

**Implementation**:
- Similar models in FDS (NIST Fire Dynamics Simulator)
- Comparison with CFD radiation modules
- Validation against NIST fire test data

---

**Created**: February 3, 2026  
**Status**: Phase 9 Radiation Design Document  
**Next Phase**: Begin implementation with Test 9A1
