# Phase 7: Architecture & Design Reference

**Document Date**: February 2, 2026  
**Purpose**: Technical architecture for Phase 7 implementation  
**Audience**: Development team

---

## Design Principles

### 1. Backward Compatibility
- Phase 6 parameters remain LOCKED and unchanged
- Existing ValidationSuite continues to work
- Single-zone model remains default
- All numeric tests continue to pass

### 2. Optional Features
- Advanced models (three-zone) are opt-in
- New scenarios don't affect existing validation
- Sensitivity analysis is separate module
- Performance optimization doesn't break API

### 3. Code Organization
- Phase 7 code in separate `phase7/` directory
- Clear separation of concerns
- Reusable components
- Extensible design for Phase 8+

### 4. Testing First
- Unit tests before implementation
- Validation against literature
- Performance benchmarks
- Reproducibility verified

---

## Module Architecture

```
┌─────────────────────────────────────────────────────┐
│           Phase 7 Enhancement Layer                 │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐ │
│  │ Sensitivity  │  │ Uncertainty  │  │ Scenario │ │
│  │  Analysis    │  │Quantification│  │ Library  │ │
│  └──────────────┘  └──────────────┘  └──────────┘ │
│                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐ │
│  │Three-Zone    │  │   CFD        │  │Performance
│  │ Model        │  │ Coupling     │  │ Tools    │ │
│  └──────────────┘  └──────────────┘  └──────────┘ │
│                                                     │
├─────────────────────────────────────────────────────┤
│         Phase 6 Core (LOCKED & STABLE)             │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐ │
│  │ Simulation   │  │Reactor       │  │Chemistry │ │
│  │   (LOCKED)   │  │  (LOCKED)    │  │(LOCKED)  │ │
│  └──────────────┘  └──────────────┘  └──────────┘ │
│                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────┐ │
│  │ Suppression  │  │ Ventilation  │  │ Li-Ion   │ │
│  │ (LOCKED)     │  │ (LOCKED)     │  │(LOCKED)  │ │
│  └──────────────┘  └──────────────┘  └──────────┘ │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## Tier 1: Sensitivity Analysis

### Class Hierarchy

```cpp
// phase7/SensitivityAnalysis.h

namespace vfep::phase7 {

struct ParameterRange {
  std::string name;
  double nominal_value;
  double min_value;
  double max_value;
  int num_samples;
  
  // Generate sample values
  std::vector<double> generateSamples() const;
};

struct SensitivityResult {
  std::string parameter_name;
  std::vector<double> sample_values;
  std::vector<double> predicted_values;
  double correlation_coefficient;
  
  // Statistical measures
  double sensitivity_index() const;
  double elasticity() const;
};

class SensitivityAnalyzer {
public:
  // Analyze individual parameters
  SensitivityResult analyzeParameter(
    const ValidationScenario& scenario,
    const ParameterRange& range,
    const std::string& metric_name  // e.g., "peak_T_K"
  );
  
  // Batch analysis of all parameters
  std::vector<SensitivityResult> analyzeAllParameters(
    const ValidationScenario& scenario
  );
  
  // Export results
  void exportResultsCSV(
    const std::vector<SensitivityResult>& results,
    const std::string& filename
  );
  
private:
  // Helper methods
  double runSingleSimulation(
    const ValidationScenario& scenario,
    const std::string& parameter_name,
    double parameter_value
  );
};

} // namespace vfep::phase7
```

### Parameter Sweep Tool

```cpp
// tools/phase7/SweepTool.cpp

// Usage examples:
// ./SweepTool --scenario NIST --param heat_release --min 50e3 --max 200e3 --samples 10
// ./SweepTool --scenario ISO9705 --param h_W --min 0.5 --max 20 --samples 8
// ./SweepTool --all-scenarios --all-params

struct SweepConfig {
  std::string scenario_name;
  std::string parameter_name;
  double min_value, max_value;
  int num_samples;
  std::string output_file;
};

class SweepTool {
public:
  int run(const SweepConfig& config);
  
private:
  void parseArguments(int argc, char** argv, SweepConfig& config);
  void executeSweep(const SweepConfig& config);
  void generateReport(const SweepConfig& config);
};
```

### Expected Outputs

**CSV Format**:
```
scenario,parameter,value,peak_T_K,peak_HRR_W,error_pct
NIST,heat_release,50000,450.0,30000,-60%
NIST,heat_release,75000,620.5,50000,-30%
NIST,heat_release,100000,725.0,71400,0%
NIST,heat_release,125000,810.3,85000,19%
NIST,heat_release,150000,880.1,98000,37%
ISO9705,h_W,0.5,981.0,XXX,4.11%
ISO9705,h_W,1.0,850.0,XXX,-17%
...
```

---

## Tier 2: Uncertainty Quantification

### Class Hierarchy

```cpp
// phase7/UncertaintyQuantification.h

namespace vfep::phase7 {

struct UQResult {
  // Central tendency
  double mean;
  double median;
  double std_dev;
  
  // Confidence intervals
  double ci_lower_95;  // 2.5 percentile
  double ci_upper_95;  // 97.5 percentile
  
  // Distribution info
  int num_samples;
  std::vector<double> all_samples;  // For histogram analysis
};

class MonteCarloUQ {
public:
  // Run uncertainty analysis
  UQResult runMonteCarlo(
    const ValidationScenario& scenario,
    int num_samples = 100,
    unsigned int seed = 42
  );
  
  // Advanced: correlated parameters
  UQResult runMonteCarloCorrelated(
    const ValidationScenario& scenario,
    const std::map<std::string, ParameterRange>& ranges,
    int num_samples = 100,
    const std::string& correlation_matrix_file = ""
  );
  
  // Export results
  void exportResultsCSV(
    const std::map<std::string, UQResult>& results,
    const std::string& filename
  );
  
  void exportHistogram(
    const UQResult& result,
    const std::string& output_file,
    int num_bins = 20
  );
  
private:
  // Sampling methods
  std::vector<double> latinHypercubeSample(
    const ParameterRange& range,
    int num_samples
  );
  
  // Run single MC sample
  double runMCsample(
    const ValidationScenario& scenario,
    const std::map<std::string, double>& parameter_values
  );
};

} // namespace vfep::phase7
```

### Expected Outputs

**UQ Results CSV**:
```
scenario,metric,mean,median,std_dev,ci_lower_95,ci_upper_95,samples
NIST,peak_HRR_W,71.2,71.0,2.3,67.5,75.1,100
NIST,peak_T_K,724.5,724.2,18.3,689.1,761.2,100
ISO9705,peak_T_upper_zone_K,983.1,982.5,12.4,958.6,1007.2,100
Suppression,reduction_pct,79.8,79.9,1.2,77.6,82.0,100
...
```

---

## Tier 2: New Scenarios

### Scenario Architecture

```cpp
// phase7/ScenarioLibrary.h

namespace vfep::phase7 {

class ShipFireScenario : public IValidationScenario {
public:
  // Override base methods
  std::string name() const override;
  void configure(Simulation& sim) const override;
  ValidationMetrics run(Simulation& sim, double dt, double t_end) const override;
  ValidationMetrics getExpectedResults() const override;
  
private:
  static constexpr double VOLUME_M3 = 100.0;
  static constexpr double AREA_M2 = 150.0;
  static constexpr double H_W_W_M2K = 15.0;  // Aluminum
  static constexpr double HEAT_RELEASE_J_MOL = 150e3;
  static constexpr double PYROLYSIS_RATE_KG_S = 0.05;
};

class TunnelFireScenario : public IValidationScenario {
public:
  std::string name() const override;
  void configure(Simulation& sim) const override;
  ValidationMetrics run(Simulation& sim, double dt, double t_end) const override;
  ValidationMetrics getExpectedResults() const override;
  
private:
  static constexpr double VOLUME_M3 = 5000.0;  // 500m × 10m³ cross-section
  static constexpr double AREA_M2 = 1000.0;
  static constexpr double H_W_W_M2K = 8.0;     // Concrete
  static constexpr double HEAT_RELEASE_J_MOL = 450e3;
  static constexpr double PYROLYSIS_RATE_KG_S = 0.1;
  static constexpr double ACH = 5.0;  // Moderate ventilation
};

class IndustrialCompartmentScenario : public IValidationScenario {
public:
  std::string name() const override;
  void configure(Simulation& sim) const override;
  ValidationMetrics run(Simulation& sim, double dt, double t_end) const override;
  ValidationMetrics getExpectedResults() const override;
  
private:
  static constexpr double VOLUME_M3 = 2500.0;
  static constexpr double AREA_M2 = 800.0;
  static constexpr double H_W_W_M2K = 5.0;     // Steel radiates
  static constexpr double HEAT_RELEASE_J_MOL = 200e3;
  static constexpr double PYROLYSIS_RATE_KG_S = 0.2;
  static constexpr double ACH = 2.0;
};

} // namespace vfep::phase7
```

### Integration with ValidationSuite

```cpp
// Extend ValidationSuite.cpp

int main() {
  // Existing Phase 6 scenarios
  runValidationSuite();
  
  // NEW Phase 7 scenarios
  if (ENABLE_PHASE7) {
    runPhase7Scenarios();
  }
}

void runPhase7Scenarios() {
  using namespace vfep::phase7;
  
  // Ship Fire
  ShipFireScenario ship;
  ValidationMetrics ship_results = ship.run(sim, 0.05, 300.0);
  evaluateScenario(ship_results, ship.getExpectedResults());
  
  // Tunnel Fire
  TunnelFireScenario tunnel;
  ValidationMetrics tunnel_results = tunnel.run(sim, 0.05, 600.0);
  evaluateScenario(tunnel_results, tunnel.getExpectedResults());
  
  // Industrial Compartment
  IndustrialCompartmentScenario industrial;
  ValidationMetrics ind_results = industrial.run(sim, 0.05, 400.0);
  evaluateScenario(ind_results, industrial.getExpectedResults());
}
```

---

## Tier 3: Three-Zone Model

### Zone Model Architecture

```cpp
// phase7/ThreeZoneModel.h

namespace vfep::phase7 {

class Zone {
public:
  // State variables
  double getTemperature() const { return T_K_; }
  double getPressure() const { return P_Pa_; }
  double getVolume() const { return V_m3_; }
  
  // Species composition
  void setO2Concentration(double fraction) { O2_frac_ = fraction; }
  double getO2Concentration() const { return O2_frac_; }
  
  // Mass exchange
  void receiveInflow(double mdot_kg_s, const Zone* source_zone);
  void sendOutflow(double mdot_kg_s);
  
  // Heat transfer
  void receiveHeat(double Q_W) { Q_in_W_ = Q_W; }
  double getLossToWalls(double area_m2, double h_W) const;
  
  // Update state
  void update(double dt);
  
private:
  double T_K_, P_Pa_, V_m3_;
  double O2_frac_, CO2_frac_, H2O_frac_, N2_frac_;
  double rho_;  // density
  
  // Flow state
  double mdot_in_kg_s_, mdot_out_kg_s_;
  double Q_in_W_;  // Heat rate in
};

class ThreeZoneFireModel {
public:
  ThreeZoneFireModel(
    double room_volume_m3,
    double wall_area_m2,
    double h_W_W_m2K,
    double ceiling_height_m = 3.0
  );
  
  // Access zones
  Zone& getUpperZone() { return upper_zone_; }
  Zone& getMiddleZone() { return middle_zone_; }
  Zone& getLowerZone() { return lower_zone_; }
  
  // Combustion source
  void setCombustionPower(double Q_W) { Q_combustion_W_ = Q_W; }
  
  // Ventilation
  void setInflowRate(double mdot_kg_s) { mdot_inflow_kg_s_ = mdot_kg_s; }
  void setOutflowRate(double mdot_kg_s) { mdot_outflow_kg_s_ = mdot_kg_s; }
  
  // Step simulation
  void step(double dt);
  
  // Utilities
  double getVolume_AveragedTemperature() const;
  double getUpperZoneHeight() const { return z_interface_upper_; }
  
private:
  Zone upper_zone_, middle_zone_, lower_zone_;
  
  // Geometry
  double total_volume_m3_, wall_area_m2_, h_W_W_m2K_;
  double z_interface_upper_;  // Height of upper-middle interface
  double z_interface_lower_;  // Height of middle-lower interface
  
  // Energy source
  double Q_combustion_W_;
  
  // Mass exchange
  double mdot_inflow_kg_s_, mdot_outflow_kg_s_;
  
  // Private methods
  void updateInterfaceHeights();
  void exchangeMass();
  void exchangeHeat();
  void updateSpecies();
  void balanceMass();
};

} // namespace vfep::phase7
```

### Validation Strategy for Three-Zone

```cpp
// Test against ISO 9705 and NIST

void validateThreeZoneModel() {
  // Single-zone baseline
  Simulation single_zone;
  single_zone.resetToDataCenterRackScenario();
  // ... run and record results
  double single_peak_T = single_zone.observe().T_K;
  
  // Three-zone model
  ThreeZoneFireModel three_zone(
    120.0,   // volume
    180.0,   // area
    10.0     // h_W
  );
  
  // Should give similar average temperature
  // but better upper zone representation
  
  REQUIRE(three_zone.getVolume_AveragedTemperature() 
          ≈ single_peak_T (±10%));
  
  // Upper zone should be hotter than average
  REQUIRE(three_zone.getUpperZone().getTemperature() 
          > three_zone.getVolume_AveragedTemperature());
}
```

---

## Tier 4: Performance & Deployment

### Performance Optimization Targets

```cpp
// Current state: ~5s per 60s simulation (5x slowdown acceptable)
// Target: <2s per 60s simulation (2x slowdown)

// Optimization candidates:

// 1. Reaction rate calculation (most CPU time)
// BEFORE: Calculate every step for every state variable
// AFTER: Cache rates where O2 > 15% (no combustion anyway)

// 2. Heat transfer loops
// BEFORE: Individual calculations per component
// AFTER: Vectorized with SIMD (Eigen or std::valarray)

// 3. Numerical integration
// BEFORE: Fixed timestep (safe but slow)
// AFTER: Adaptive timestep (larger dt when stable)

// 4. Species tracking
// BEFORE: Track 10+ species
// AFTER: Track only O2, CO2, H2O (for UQ simulations)
```

### Profiling Integration

```cpp
// New: PerformanceProfiler.h

class PerformanceProfiler {
public:
  void startTimer(const std::string& section_name);
  void stopTimer(const std::string& section_name);
  
  void printReport();
  void exportCSV(const std::string& filename);
  
private:
  std::map<std::string, double> cumulative_times_;
  std::map<std::string, int> call_counts_;
};

// Usage:
// profiler.startTimer("combustion");
// calculateCombustionRate(...);
// profiler.stopTimer("combustion");
```

---

## Integration Checklist

### CMakeLists.txt Changes
```cmake
# Add Phase 7 option
option(ENABLE_PHASE7 "Build Phase 7 features" OFF)

if(ENABLE_PHASE7)
  # Add phase7 headers
  list(APPEND PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/SensitivityAnalysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/UncertaintyQuantification.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/ThreeZoneModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/ScenarioLibrary.h
  )
  
  # Add phase7 sources
  list(APPEND SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/SensitivityAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/UncertaintyQuantification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/ThreeZoneModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/Scenarios.cpp
  )
  
  # Build SweepTool
  add_executable(SweepTool tools/phase7/SweepTool.cpp)
  target_link_libraries(SweepTool chemsi)
endif()
```

### Build Commands
```bash
# Build without Phase 7 (default)
cmake --build build-mingw64 --config Release

# Build with Phase 7
cmake -DENABLE_PHASE7=ON build-phase7
cmake --build build-phase7 --config Release

# Test Phase 7
./build-phase7/ValidationSuite.exe
./build-phase7/SweepTool --scenario NIST --param heat_release --samples 10
```

---

## Backward Compatibility Matrix

| Feature | Phase 6 | Phase 7 | Breaking? |
|---------|---------|---------|-----------|
| Simulation.step() | ✅ | ✅ | No |
| ValidationSuite | ✅ | ✅ Extended | No |
| NumericIntegrity | 51 tests | 77+ tests | No |
| Heat release 100 kJ/mol | LOCKED | LOCKED | No |
| NIST 71.4 kW | LOCKED | LOCKED | No |
| Single-zone (default) | Yes | Yes | No |
| Three-zone model | N/A | Optional | N/A |
| Sensitivity analysis | N/A | New | N/A |

---

**Document Purpose**: Reference for Phase 7 implementation  
**Status**: READY FOR DEVELOPMENT  
**Next Step**: Begin Tier 1 (Sensitivity Analysis)
