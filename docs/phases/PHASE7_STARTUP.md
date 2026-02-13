# Phase 7: Advanced Validation & Uncertainty Quantification

**Prepared**: February 2, 2026  
**Status**: READY TO START  
**Foundation**: Phase 6 (4/4 scenarios passing, 100% validation)

---

## Phase 7 Overview

Phase 7 expands validation beyond initial scenarios into:
1. **Advanced fire scenarios** (ship fires, tunnels, industrial compartments)
2. **Uncertainty quantification** (parameter sensitivity, Monte Carlo)
3. **Model refinement** (true three-zone implementation, CFD coupling)
4. **Production deployment** (performance optimization, extended test suite)

### Success Criteria
- ✅ 6+ fire scenarios validated (currently: 4)
- ✅ Uncertainty analysis complete (±bounds on all parameters)
- ✅ Advanced model options available (switching between single/multi-zone)
- ✅ Performance: <2s per 60s simulation on standard hardware
- ✅ Documentation: Full API reference and user guide

---

## What You're Building On

### From Phase 6 (✅ Complete)
- 51/51 numeric integrity tests passing
- 4/4 validation scenarios passing (100% success)
- Clean build with zero warnings/errors
- All parameters locked in code and documented
- Two-zone temperature correction implemented

### Current State
- **NIST Baseline**: 71.4 kW (4.85% error) - LOCKED ✅
- **ISO 9705**: 981K (4.11% error) - LOCKED ✅
- **Suppression**: 79.77% reduction (13.95% error) - LOCKED ✅
- **Stratification**: 272K ΔT (9.26% error) - LOCKED ✅

---

## Phase 7 Architecture

### Tier 1: Core Enhancements (Weeks 1-2)

#### 1.1 Sensitivity Analysis Framework
**Objective**: Quantify parameter impact on results

**Scope**:
- Heat release sensitivity (±25% around 100 kJ/mol)
- Wall loss coefficient (h_W): 0.5 to 20 W/m²K
- Room volume variations (10-500 m³)
- Pyrolysis rate variations (0.01-1.0 kg/s)

**Deliverable**:
```cpp
// New: SensitivityAnalysis.h
class SensitivityAnalyzer {
  struct ParameterRange {
    double nominal, min, max;
    int samples;  // e.g., 5 for ±25% at ±5% increments
  };
  
  void analyzeHeatRelease(const ParameterRange& range);
  void analyzeWallLoss(const ParameterRange& range);
  void analyzeGeometry(const ParameterRange& range);
  void analyzePyrolysis(const ParameterRange& range);
  
  void exportSensitivityMatrixCSV(const std::string& filename);
};
```

**Expected Outcome**: Sensitivity matrix (4 parameters × 5 samples = 20 simulations per scenario)

#### 1.2 Uncertainty Quantification (UQ) Module
**Objective**: Calculate confidence intervals on predictions

**Scope**:
- Latin Hypercube Sampling (LHS) for parameter space
- 100 Monte Carlo samples per scenario
- ±95% confidence bounds on peak temperature, HRR, etc.

**Deliverable**:
```cpp
// New: UncertaintyQuantification.h
class MonteCarloUQ {
  struct UQResult {
    double mean, median;
    double ci_lower_95, ci_upper_95;
    double std_dev;
  };
  
  UQResult runMonteCarlo(const ValidationScenario& scenario, 
                         int num_samples = 100);
};
```

**Expected Outcome**: Confidence intervals on all 4 scenarios with supporting statistics

#### 1.3 Parameter Sweep Tool
**Objective**: Systematic exploration of parameter space

**Implementation**:
```cpp
// Extend ValidationSuite.cpp
void runParameterSweep(
  const std::string& parameter_name,
  const std::vector<double>& values,
  const std::string& output_csv
);
```

**Usage**:
```
./SweepTool --param heat_release --min 50e3 --max 200e3 --samples 10
./SweepTool --param h_W --min 0.5 --max 20.0 --samples 8
```

---

### Tier 2: New Scenarios (Weeks 2-3)

#### 2.1 Ship Fire Scenario
**Physics**:
- Confined ship compartment (≈100 m³)
- Limited ventilation (porthole, forced air)
- Aluminum structure (high heat loss)
- Cargo hold fire (cellulose/paper)

**Parameters**:
```cpp
Volume: 100 m³
Area: 150 m² (plus internal structure)
h_W: 15 W/m²K (aluminum conducts heat)
Heat Release: 150 kJ/mol (cellulose)
Pyrolysis Rate: 0.05-0.15 kg/s
Ignition: t=5s (smoldering start)
```

**Target Metrics**:
- Peak temperature: 800-1000K
- Time to 500K: <60s
- HRR growth: Controlled, no flashover

**Literature**:
- IMO SOLAS fire testing standards
- CFD studies on ship compartments

#### 2.2 Tunnel Fire Scenario
**Physics**:
- Long, slender geometry (500 m long, 10 m³ cross-section)
- Flow-driven ventilation (0.5-2.0 m/s)
- Concrete walls (moderate heat loss)
- Vehicle fire (liquid fuel: gasoline/diesel)

**Parameters**:
```cpp
Volume: 5000 m³ (500m × 10 m³)
Area: 1000 m² (tunnel walls)
h_W: 8 W/m²K (concrete)
Heat Release: 450 kJ/mol (hydrocarbons)
Pyrolysis Rate: 0.1-0.3 kg/s
ACH: 2-10 (flow-dependent)
Ignition: t=2s
```

**Target Metrics**:
- Smoke layer height: >2m (stratification)
- Downwind temperature rise: 50-150K
- HRR plateau: 500-2000 kW

**Literature**:
- EUREKA 499 tunnel fire experiments
- Memorial Tunnel Fire Ventilation Test Program

#### 2.3 Industrial Compartment Fire
**Physics**:
- Large warehouse (1000-5000 m³)
- Ceiling vents and floor openings
- Steel frame (low thermal mass)
- Rack storage (mixed materials)

**Parameters**:
```cpp
Volume: 2500 m³
Area: 800 m² (walls + ceiling)
h_W: 5 W/m²K (steel radiates)
Heat Release: 200 kJ/mol (mixed)
Pyrolysis Rate: 0.2-0.5 kg/s
ACH: 1-5 (natural + forced)
Ignition: t=3s
```

**Target Metrics**:
- Ceiling temperature: >500K (sprinkler activation)
- Time to untenable conditions: 2-5 min
- Smoke filling rate: varies with ACH

**Literature**:
- ISO 9414 (Gross calorific potential)
- ASTM E603 (Room fire test)

---

### Tier 3: Model Refinement (Weeks 3-4)

#### 3.1 True Three-Zone Model
**Current**: Single-zone with post-hoc two-zone correction  
**Target**: Coupled three-zone system (upper hot layer, middle cool layer, lower ambient)

**Architecture**:
```cpp
// New: ThreeZoneModel.h
class ThreeZoneFireModel {
  struct Zone {
    double volume_m3;
    double T_K;
    double P_Pa;
    Species composition;  // O2, N2, CO2, H2O, CO, smoke
  };
  
  Zone upper_zone_, middle_zone_, lower_zone_;
  
  void step(double dt);
  void updateMassExchange();
  void updateHeatTransfer();
  void updateSpecies();
};
```

**Implementation Steps**:
1. Define zone boundaries (heights, volumes)
2. Implement inter-zone mass flow (buoyancy-driven)
3. Add zone-to-zone heat transfer
4. Species diffusion across zones
5. Validate against ISO 9705 single scenario

**Expected Performance**:
- ISO 9705: 1000-1050K (vs 981K current)
- Better layer height tracking
- Realistic temperature profile over time

#### 3.2 CFD Coupling Interface
**Objective**: Enable comparison with CFD codes (FDS, OpenFOAM)

**Approach**:
```cpp
// New: CFDInterface.h
class CFDCoupler {
  struct GridPoint {
    double x, y, z;
    double T, u, v, w;  // temperature and velocity
    Species spec;
  };
  
  // Import velocity field from CFD
  void importVelocityField(const std::string& cgns_file);
  
  // Export VFEP results for comparison
  void exportResults(const std::string& output_format);
  
  // Point-to-field interpolation
  double interpTemperature(double x, double y, double z, double t);
};
```

**File Format Support**:
- CGNS (CFD General Notation System) - standard
- HDF5 - for large datasets
- VTK - for visualization

**Comparison Methodology**:
1. Run CFD case with same parameters
2. Extract velocity field at fire location
3. Import into VFEP with frozen velocity
4. Compare temperature/species predictions
5. Document differences and reasons

---

### Tier 4: Performance & Deployment (Week 4)

#### 4.1 Performance Optimization
**Targets**:
- Single scenario: <2 seconds (60s simulation time)
- Batch sweep (20 scenarios): <30 seconds

**Optimization Areas**:
```cpp
// Profile these:
1. Species reaction kinetics (most expensive)
2. Combustion model (frequent calculation)
3. Heat transfer calculations
4. Numerical integration (timestep management)

// Candidates for optimization:
- Vectorize reaction rates (SIMD)
- Cache frequent calculations
- Reduce solver iterations in converged regions
- Parallel batch sweep support
```

#### 4.2 Extended Test Suite
**Add to NumericIntegrity**:
- Sensitivity analysis tests (10 new tests)
- Uncertainty quantification tests (5 new tests)
- Three-zone model tests (8 new tests)
- CFD coupling tests (mock interface, 3 tests)

**Target**: 77+ total numeric tests (currently 51)

#### 4.3 User Documentation
**Create**:
- **Phase7_API_Reference.md**: Full API documentation
- **Phase7_User_Guide.md**: How to use sensitivity analysis, UQ, scenarios
- **Phase7_Technical_Report.md**: Results and conclusions
- **Examples/*.cpp**: Sample programs showing advanced features

---

## Technical Dependencies

### Libraries Needed
- **Boost.Random** - LHS sampling for uncertainty quantification (already available)
- **HDF5** - For CFD data exchange (optional, install if needed)
- **Eigen** - For matrix operations in three-zone model (optional)

### Build Changes
```cmake
# In CMakeLists.txt, Phase 7 additions:
if(ENABLE_PHASE7)
  add_library(SensitivityAnalysis src/SensitivityAnalysis.cpp)
  add_library(ThreeZoneModel src/ThreeZoneModel.cpp)
  add_library(UncertaintyQuantification src/UncertaintyQuantification.cpp)
  
  add_executable(SweepTool tools/SweepTool.cpp)
  target_link_libraries(SweepTool SensitivityAnalysis)
endif()
```

---

## Development Plan

### Week 1: Sensitivity & UQ Foundation
- [ ] Design `SensitivityAnalysis` class
- [ ] Implement parameter sweep framework
- [ ] Create `MonteCarloUQ` module with LHS sampling
- [ ] Generate sensitivity matrix for all 4 current scenarios
- [ ] Document results in CSV format

### Week 2: New Scenarios
- [ ] Implement Ship Fire scenario
- [ ] Implement Tunnel Fire scenario
- [ ] Implement Industrial Compartment scenario
- [ ] Validate against literature benchmarks
- [ ] Add 12 numeric tests

### Week 3: Three-Zone Model
- [ ] Design three-zone architecture
- [ ] Implement inter-zone mass exchange
- [ ] Implement zone heat transfer
- [ ] Validate against ISO 9705
- [ ] Add 8 numeric tests
- [ ] Document differences from single-zone

### Week 4: Performance & Deployment
- [ ] Profile code and identify bottlenecks
- [ ] Implement optimizations
- [ ] Create CFD coupling interface (mock)
- [ ] Write comprehensive documentation
- [ ] Final validation and cleanup

---

## Metrics & Success Criteria

### Code Quality
- ✅ Maintain zero compiler warnings
- ✅ All 77+ tests passing
- ✅ No performance regressions

### Validation
- ✅ All 6 scenarios within ±15% of literature
- ✅ Uncertainty bounds published
- ✅ Three-zone model improves ISO 9705

### Performance
- ✅ Single scenario: <2s
- ✅ Batch sweep: <30s (20 scenarios)
- ✅ Memory: <500 MB peak

### Documentation
- ✅ API reference complete
- ✅ User guide with examples
- ✅ Technical report with results

---

## Quick Start Checklist

### Before You Start
- [ ] Read PHASE6_FINAL_COMPLETION.md
- [ ] Review PHASE6_CALIBRATION_PARAMETERS.md
- [ ] Verify current build: `cmake --build build-mingw64 --config Release`
- [ ] Verify 4/4 scenarios pass: `./build-mingw64/ValidationSuite.exe`
- [ ] Verify 51/51 tests pass: `./build-mingw64/NumericIntegrity.exe`

### Day 1: Baseline & Planning
- [ ] Create branch: `git checkout -b phase7/advanced-validation`
- [ ] Create project structure:
  ```bash
  mkdir -p cpp_engine/src/phase7
  mkdir -p cpp_engine/include/phase7
  mkdir -p tools/phase7
  mkdir -p docs/phase7
  ```
- [ ] Create Phase 7 log file: `PHASE7_SESSION1_LOG.md`
- [ ] Review this entire document
- [ ] Identify first week priorities

### Day 2: Start Development
- [ ] Design SensitivityAnalysis class
- [ ] Create header file: `cpp_engine/include/phase7/SensitivityAnalysis.h`
- [ ] Create implementation: `cpp_engine/src/phase7/SensitivityAnalysis.cpp`
- [ ] Add to CMakeLists.txt with optional flag
- [ ] First compilation test

### Week 1 Goals
- [ ] Sensitivity analysis framework working
- [ ] Parameter sweep producing CSV output
- [ ] Monte Carlo UQ giving confidence intervals
- [ ] All 4 current scenarios analyzed

---

## File Structure

```
cpp_engine/
├── include/
│   └── phase7/
│       ├── SensitivityAnalysis.h
│       ├── UncertaintyQuantification.h
│       ├── ScenarioLibrary.h
│       └── ThreeZoneModel.h
├── src/
│   ├── Simulation.cpp (add three-zone option)
│   └── phase7/
│       ├── SensitivityAnalysis.cpp
│       ├── UncertaintyQuantification.cpp
│       ├── ShipFireScenario.cpp
│       ├── TunnelFireScenario.cpp
│       ├── IndustrialCompartmentScenario.cpp
│       └── ThreeZoneModel.cpp
└── tests/
    └── Phase7/
        ├── TestSensitivityAnalysis.cpp
        ├── TestUncertaintyQuantification.cpp
        ├── TestThreeZoneModel.cpp
        └── TestNewScenarios.cpp

tools/
└── phase7/
    └── SweepTool.cpp

docs/
└── phase7/
    ├── Phase7_Startup.md (this file)
    ├── Phase7_API_Reference.md (to create)
    ├── Phase7_User_Guide.md (to create)
    └── Phase7_Technical_Report.md (to create)
```

---

## Recommended Reading Order

Before starting Phase 7, review in this order:

1. **[PHASE6_FINAL_COMPLETION.md](PHASE6_FINAL_COMPLETION.md)** - 15 min
   - Understand what Phase 6 achieved
   - Note locked parameters

2. **[PHASE6_CALIBRATION_PARAMETERS.md](PHASE6_CALIBRATION_PARAMETERS.md)** - 20 min
   - All current parameters documented
   - Physical basis for each value
   - Implementation code locations

3. **This document** (Phase7_Startup.md) - 30 min
   - Understand Phase 7 scope
   - Architecture decisions
   - Development plan

4. **[Simulation.h](cpp_engine/include/Simulation.h)** - 15 min
   - Understand public API
   - Current methods available
   - Extension points for Phase 7

5. **[ValidationSuite.cpp](cpp_engine/src/ValidationSuite.cpp)** - 15 min
   - How scenarios are implemented
   - Pattern for new scenarios

---

## Questions & Decisions

### Q1: Which tier to start with?
**A**: Tier 1 (Sensitivity & UQ) - provides foundation for understanding parameter impacts before adding new scenarios.

### Q2: Should we keep Phase 6 scenarios unchanged?
**A**: YES - Phase 6 parameters are LOCKED. New scenarios use same physical models but different geometry/fuel.

### Q3: Can we run multiple scenarios in parallel?
**A**: In Phase 7, yes. Add OpenMP support to batch sweep tool.

### Q4: What if three-zone model breaks something?
**A**: Keep it as optional flag in code. Users can switch between models. Single-zone remains default.

### Q5: Do we need CFD coupling immediately?
**A**: No - create interface with mock data first (Week 4). Real CFD integration can be Phase 8.

---

## Success Indicators

✅ **Phase 7 Success = **
- 6+ fire scenarios validated
- Uncertainty analysis complete
- Three-zone model implemented
- <2s per scenario performance
- Comprehensive documentation

**Timeline**: 4 weeks  
**Effort**: ~120-160 hours  
**Team Size**: 1-2 engineers  

---

**Prepared by**: Automated System  
**Date**: February 2, 2026  
**Status**: READY TO START  
**Next Step**: Create Phase 7 project structure and begin Week 1 work
