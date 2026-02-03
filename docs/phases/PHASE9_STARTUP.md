# Phase 9: Advanced Physics & Multi-Scale Modeling

**Prepared**: February 3, 2026  
**Status**: READY TO START  
**Foundation**: Phase 8 Complete (62/62 tests, 7/7 scenarios, Production Ready)

---

## Phase 9 Overview

Phase 9 extends VFEP beyond single-compartment fires to advanced multi-physics and multi-scale capabilities:
1. **Detailed Radiation Model** (participating media, view factors)
2. **Multi-Compartment Transport** (room-to-room smoke spread)
3. **Live CFD Coupling** (bidirectional real-time integration)
4. **Flame Spread Models** (surface fire growth)
5. **Machine Learning Integration** (surrogate models, parameter inference)

### Success Criteria
- ✅ Radiation model with view factors operational
- ✅ Multi-compartment network (3+ rooms) validated
- ✅ CFD coupling demonstrated (mock or real)
- ✅ Flame spread validated against test data
- ✅ ML surrogate model for parameter estimation

---

## What You're Building On

### From Phase 8 (✅ COMPLETE)
- **62/62 numeric integrity tests** passing
- **7/7 validation scenarios** passing (mean error: 9.02%)
- **ThreeZoneModel**: Stratified fire physics
- **CFDInterface**: Comparison framework
- **SensitivityAnalysis**: Parameter sweeps
- **UncertaintyQuantification**: Monte Carlo & LHS
- **Complete documentation**: API, user guide, technical report, scenarios
- **Production ready**: Zero warnings, clean build, excellent performance

### Current Capabilities
- Single compartment fires (100-5000 m³)
- Three-zone stratification
- Suppression dynamics
- Sensitivity & uncertainty analysis
- CFD comparison (import/export)
- Performance: <0.5s per 60s simulation

---

## Phase 9 Work Plan

### Track 1: Advanced Radiation Model (3 weeks)

#### Week 1: View Factor Geometry

**Objective**: Implement geometric view factors for radiation exchange

**Theory**:
```
Q_rad = ε σ A_i F_ij (T_i^4 - T_j^4)
```

Where:
- ε = surface emissivity
- σ = Stefan-Boltzmann constant (5.67e-8 W/m²K⁴)
- F_ij = view factor from surface i to surface j
- A_i = surface area

**Implementation**:
```cpp
// New: RadiationModel.h
namespace vfep {

struct Surface {
    double area_m2;
    double emissivity;
    double T_K;
    Vec3 position;
    Vec3 normal;
};

class RadiationModel {
public:
    void addSurface(const Surface& surf);
    void computeViewFactors();  // Geometric calculation
    double radiativeFlux(int surf_i, int surf_j) const;
    void step(double dt);  // Update temperatures
    
private:
    std::vector<Surface> surfaces_;
    std::vector<std::vector<double>> view_factors_;  // F_ij matrix
    
    double computeViewFactor(int i, int j) const;
};

} // namespace vfep
```

**Deliverable**: RadiationModel with 6-surface room validated

---

#### Week 2: Participating Media

**Objective**: Add smoke/gas radiation absorption

**Theory**:
```
dI/ds = -κ I + κ I_b(T)
```

Absorption coefficient: κ(smoke) ≈ 0.1-1.0 m⁻¹

**Implementation**:
```cpp
class ParticipatingMedia {
public:
    void setSmokeDensity(double rho_kg_m3);
    void setAbsorptionCoeff(double kappa);
    double transmissivity(double path_length_m) const;
    
private:
    double kappa_;  // m⁻¹
    double smoke_density_;
};
```

**Deliverable**: Smoke obscuration model validated

---

#### Week 3: Validation

**Targets**:
- Flame-to-wall radiative heat flux
- Hot gas layer radiation
- Smoke absorption effects

**Tests**:
- 9A1: View factor calculation
- 9A2: Surface radiation exchange
- 9A3: Participating media absorption

---

### Track 2: Multi-Compartment Network (3 weeks)

#### Week 1: Compartment Network Structure

**Objective**: Connect multiple compartments with doorways/vents

**Architecture**:
```cpp
// New: CompartmentNetwork.h
namespace vfep {

struct Compartment {
    int id;
    double volume_m3;
    ThreeZoneModel* fire_model;
    std::vector<int> connected_to;  // Neighbor IDs
};

struct Opening {
    int from_compartment;
    int to_compartment;
    double area_m2;
    double height_m;
    bool is_door;
};

class CompartmentNetwork {
public:
    int addCompartment(double volume, double area);
    void addOpening(const Opening& opening);
    void step(double dt);
    
    const Compartment& getCompartment(int id) const;
    
private:
    std::vector<Compartment> compartments_;
    std::vector<Opening> openings_;
    
    void computeFlows();  // Mass/energy between compartments
    void updatePressures();
    void transportSpecies();
};

} // namespace vfep
```

---

#### Week 2: Flow Calculations

**Physics**:
- Buoyancy-driven flow through openings
- Pressure-driven ventilation
- Neutral plane calculation

**Equations**:
```
ṁ = C_d A √(2 Δρ g H)  // Buoyancy flow
```

---

#### Week 3: Validation

**Test Case**: ISO 9239 multi-room scenario
- Fire in Room A → Smoke to Room B → Smoke to Room C
- Track temperatures, smoke density, travel time

**Tests**:
- 9B1: Two-compartment smoke transport
- 9B2: Three-compartment network
- 9B3: Pressure balance across openings

---

### Track 3: CFD Live Coupling (4 weeks)

#### Objective
Bidirectional coupling with external CFD solver (FDS, OpenFOAM, or mock)

**Workflow**:
1. VFEP computes zone-averaged properties
2. Export to CFD as boundary conditions
3. CFD computes detailed flow field
4. Import CFD velocity/temperature back to VFEP
5. VFEP updates based on CFD results
6. Repeat every N timesteps

**Implementation**:
```cpp
// Extend CFDInterface
class CFDCoupler {
public:
    void setBoundaryConditions(const ZoneProperties& vfep_state);
    void runCFDStep();
    void importCFDResults();
    void synchronize();  // Exchange data
    
    bool useMockCFD;  // For testing without real CFD
};
```

**Deliverable**: Coupled simulation demo (even if mock CFD)

---

### Track 4: Flame Spread Model (3 weeks)

#### Objective
Model fire growth on combustible surfaces

**Physics**:
- Heat transfer to surface
- Pyrolysis ignition criteria
- Flame front propagation
- Heat feedback

**Implementation**:
```cpp
// New: FlameSpread.h
struct SurfaceElement {
    double x, y;  // Position
    double T_K;   // Temperature
    bool ignited;
    double fuel_remaining_kg;
};

class FlameSpreadModel {
public:
    void addSurface(const std::vector<SurfaceElement>& elements);
    void step(double dt, double Q_rad_W);
    double totalBurnRate() const;
    
private:
    std::vector<SurfaceElement> elements_;
    
    bool checkIgnition(const SurfaceElement& elem) const;
    void propagateFront();
};
```

**Validation**:
- LIFT apparatus data
- Cone calorimeter tests
- ISO 5658-2 flame spread

**Tests**:
- 9C1: Surface ignition threshold
- 9C2: Flame front propagation
- 9C3: Coupled HRR feedback

---

### Track 5: Machine Learning Integration (4 weeks)

#### Week 1-2: Surrogate Model Training

**Objective**: Train ML model to emulate VFEP for fast prediction

**Approach**:
1. Generate training data (1000+ scenarios with parameter sweeps)
2. Train neural network: Input = (geometry, fuel, ventilation) → Output = (peak T, peak HRR, time to flashover)
3. Validate surrogate accuracy

**Tools**:
- Python: TensorFlow / PyTorch
- Export VFEP results to CSV
- Train regression model
- Export trained model weights

**Deliverable**: Surrogate model with <10% error

---

#### Week 3: Parameter Inference

**Objective**: Given measured data, infer unknown parameters

**Approach**:
- Bayesian inference or optimization
- Minimize error between simulation and measurements
- Estimate fuel properties, heat loss coefficients, etc.

**Implementation**:
```cpp
// New: ParameterInference.h
class BayesianInference {
public:
    void setObservations(const std::vector<double>& measured_T);
    void setParameterPriors(const std::map<string, Distribution>& priors);
    
    ParameterDistribution run(int n_samples);  // MCMC sampling
};
```

---

#### Week 4: Real-Time Prediction

**Objective**: Use ML for real-time fire prediction in IoT sensors

**Concept**:
- Lightweight surrogate model runs on edge device
- Sensor data (T, smoke) feeds model
- Predict fire development 60s ahead
- Trigger alarms early

---

## Phase 9 Testing Strategy

### New Tests (Target: 70-75 total)

**Track 1: Radiation (3 tests)**
- 9A1: View factor geometric calculation
- 9A2: Surface radiative exchange
- 9A3: Participating media absorption

**Track 2: Multi-Compartment (3 tests)**
- 9B1: Two-compartment mass transport
- 9B2: Three-compartment network flow
- 9B3: Pressure balance accuracy

**Track 3: CFD Coupling (2 tests)**
- 9C1: Boundary condition export
- 9C2: Field data import and sync

**Track 4: Flame Spread (3 tests)**
- 9D1: Surface ignition threshold
- 9D2: Flame front propagation rate
- 9D3: Coupled HRR feedback loop

**Track 5: Machine Learning (2 tests)**
- 9E1: Surrogate model accuracy
- 9E2: Parameter inference convergence

**Total: 62 (Phase 8) + 13 (Phase 9) = 75 tests**

---

## Phase 9 Success Metrics

| Metric | Target | Measurement |
|--------|--------|-------------|
| Tests Passing | 70+/70+ | NumericIntegrity.exe |
| Validation Scenarios | 10/10 | 7 existing + 3 new |
| Radiation Error | <15% | vs. FDS or experiments |
| Multi-Compartment Error | <20% | ISO 9239 |
| Surrogate Model Error | <10% | vs. full VFEP |
| Performance | <5s per 60s | Multi-room simulation |
| Documentation | 6 files | +2 new Phase 9 docs |

---

## Phase 9 Timeline (12 weeks)

**Week 1-3**: Radiation model  
**Week 4-6**: Multi-compartment network  
**Week 7-10**: CFD coupling + Flame spread  
**Week 11-14**: Machine learning integration  
**Week 15-16**: Documentation and validation  

---

## New Dependencies

### Optional Libraries
- **OpenFOAM/FDS**: For CFD coupling (if not using mock)
- **TensorFlow/PyTorch**: For ML surrogate models
- **HDF5**: For large dataset I/O
- **Eigen/Armadillo**: Linear algebra (view factors)

---

## Long-Term Vision (Beyond Phase 9)

- **Phase 10**: Structural response (fire damage to buildings)
- **Phase 11**: Egress modeling (people movement)
- **Phase 12**: Optimization algorithms (design optimization)
- **Phase 13**: Cloud deployment (web-based simulation)
- **Phase 14**: Regulatory compliance tools (code checking)

---

## Getting Started with Phase 9

### Day 1 Tasks
1. Review Phase 8 completion report
2. Read this document end-to-end
3. Choose starting track (recommend: Radiation → Multi-Compartment → rest)
4. Set up development environment (same as Phase 8)
5. Create Phase 9 branch in git
6. Begin RadiationModel.h skeleton

### First Implementation
Start with radiation view factors (simplest):
1. Implement 6-surface box room
2. Calculate geometric view factors
3. Add radiative exchange equations
4. Validate against analytical solutions
5. Add test 9A1

---

## Questions to Consider

1. Should we prioritize CFD coupling or multi-compartment first?
2. Which ML framework? (TensorFlow vs. PyTorch)
3. Mock CFD sufficient, or pursue real OpenFOAM coupling?
4. Focus on depth (one track perfect) or breadth (all tracks prototype)?

---

## Conclusion

Phase 9 represents the evolution from **production tool** to **advanced research platform**:
- Radiation: Better accuracy in radiant-dominated fires
- Multi-Compartment: Whole-building smoke analysis
- CFD Coupling: Best of zone + CFD
- Flame Spread: Fire growth prediction
- Machine Learning: Fast inference, parameter estimation

**Phase 8 gave us a robust foundation. Phase 9 pushes the boundaries of fire modeling.**

---

**Status**: Ready to Begin  
**Foundation**: 62/62 tests, 7/7 scenarios, production-ready codebase  
**Timeline**: 12-16 weeks for full Phase 9  
**End Goal**: State-of-the-art multi-physics fire simulation platform 🚀

---

*"Phase 8 was about validation. Phase 9 is about innovation."*
