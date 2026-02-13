# VFEP API Reference - Phase 8

**Version**: Phase 8 Complete  
**Date**: February 3, 2026  
**Status**: Production Ready  

---

## Overview

This document provides comprehensive API documentation for the Ventilated Fire Evolution Predictor (VFEP) simulation engine after Phase 8 completion.

## Core Modules

### 1. Simulation (Core Engine)

**File**: `cpp_engine/include/Simulation.h`

```cpp
namespace vfep {
  class Simulation {
  public:
    Simulation();
    void step(double dt);
    void reset();
    void ignite();
    void enableSuppression();
    const Observation& observe() const;
  };
}
```

**Key Methods**:
- `step(double dt)` - Advance simulation by timestep
- `reset()` - Reset to initial conditions
- `ignite()` - Trigger fire ignition
- `enableSuppression()` - Activate suppression system
- `observe()` - Get current simulation state

### 2. ThreeZoneModel (Phase 8)

**File**: `cpp_engine/include/ThreeZoneModel.h`

```cpp
namespace vfep {
  class ThreeZoneModel {
  public:
    ThreeZoneModel(double total_height_m, double floor_area_m2, int num_species);
    
    void step(double dt, double combustion_HRR_W, double cooling_W, double ACH);
    void reset(double T_amb = 293.15, double P_amb = 101325.0);
    
    const Zone& upperZone() const;
    const Zone& middleZone() const;
    const Zone& lowerZone() const;
    
    double smokeLayerHeight_m() const;
    double averageTemperature_K() const;
    double totalMass_kg() const;
    double totalEnergy_J() const;
  };
}
```

**Features**:
- Three stratified zones (upper hot, middle transition, lower ambient)
- Buoyancy-driven inter-zone flow
- Heat transfer across zone boundaries
- Species transport and diffusion
- Mass and energy conservation

**Example Usage**:
```cpp
vfep::ThreeZoneModel model(3.0, 25.0, 5); // 3m height, 25m² floor, 5 species
model.reset();

for (int i = 0; i < 1000; ++i) {
    model.step(0.1, 50e3, 5e3, 2.0); // dt, HRR, cooling, ACH
}

std::cout << "Smoke layer height: " << model.smokeLayerHeight_m() << " m\n";
std::cout << "Upper zone temp: " << model.upperZone().T_K << " K\n";
```

### 3. CFDInterface (Phase 8)

**File**: `cpp_engine/include/CFDInterface.h`

```cpp
namespace vfep {
  class CFDInterface {
  public:
    CFDInterface();
    
    bool importVelocityField(const std::string& vtk_file);
    bool importTemperatureField(const std::string& vtk_file);
    bool exportResults(const std::string& output_vtk, const std::vector<GridPoint>& points);
    bool exportComparisonCSV(const std::string& csv_file, 
                            const std::vector<GridPoint>& vfep_points,
                            const std::vector<GridPoint>& cfd_points);
    
    double interpolateTemperature(double x, double y, double z) const;
    void interpolateVelocity(double x, double y, double z, double& u, double& v, double& w) const;
    
    ComparisonStats compareTemperature(const std::vector<GridPoint>& vfep_points,
                                      const std::vector<GridPoint>& cfd_points) const;
    ComparisonStats compareVelocity(const std::vector<GridPoint>& vfep_points,
                                   const std::vector<GridPoint>& cfd_points) const;
    
    static bool generateMockCFD(const std::string& output_vtk, int nx, int ny, int nz,
                               const std::string& scenario);
    
    size_t gridPointCount() const;
    void clear();
  };
}
```

**Features**:
- VTK file import/export
- Trilinear interpolation for field values
- Statistical comparison tools
- Mock CFD data generation for testing

**Example Usage**:
```cpp
vfep::CFDInterface cfd;

// Generate mock data for testing
vfep::CFDInterface::generateMockCFD("test.vtk", 10, 10, 10, "room_fire");

// Import and query
cfd.importTemperatureField("test.vtk");
double T = cfd.interpolateTemperature(1.5, 2.0, 1.0);

// Compare results
auto stats = cfd.compareTemperature(vfep_results, cfd_results);
std::cout << "Mean error: " << stats.mean_error << " K\n";
std::cout << "RMSE: " << stats.rmse << " K\n";
```

### 4. SensitivityAnalysis (Phase 7)

**File**: `cpp_engine/include/SensitivityAnalysis.h`

```cpp
namespace vfep {
  class SensitivityAnalyzer {
  public:
    void setSweepParameters(const SweepParameters& params);
    SweepResults runSweep(const std::string& parameter_name, 
                         const std::vector<double>& values);
    void exportCSV(const std::string& filename, const SweepResults& results);
  };
}
```

**Features**:
- Parameter sweep capabilities
- Multi-parameter sensitivity analysis
- CSV export for plotting

### 5. UncertaintyQuantification (Phase 7)

**File**: `cpp_engine/include/UncertaintyQuantification.h`

```cpp
namespace vfep {
  class MonteCarloUQ {
  public:
    void setScenario(const ScenarioConfig& config);
    void setRanges(const UQRanges& ranges);
    
    UQSummary runMonteCarlo(int num_samples);
    UQSummary runLatinHypercube(int num_samples);
  };
}
```

**Features**:
- Monte Carlo uncertainty quantification
- Latin Hypercube Sampling (LHS)
- Statistical summaries (mean, median, CI, std dev)

## Data Structures

### Observation

```cpp
struct Observation {
    double time_s;
    double T_K;              // Temperature (K)
    double HRR_W;            // Heat Release Rate (W)
    double O2_volpct;        // Oxygen (vol%)
    double CO2_volpct;       // Carbon Dioxide (vol%)
    double H2O_volpct;       // Water Vapor (vol%)
    double fuel_kg;          // Remaining fuel (kg)
    double inhibitor_kgm3;   // Suppression agent (kg/m³)
    double inert_kgm3;       // Inert gas (kg/m³)
    double ACH;              // Air Changes per Hour
    double agent_mdot_kgps;  // Agent flow rate (kg/s)
    double reward;           // Reinforcement learning reward
};
```

### GridPoint

```cpp
struct GridPoint {
    double x, y, z;         // Position (m)
    double T_K;             // Temperature (K)
    double u, v, w;         // Velocity (m/s)
    double rho_kg_m3;       // Density (kg/m³)
    double P_Pa;            // Pressure (Pa)
};
```

### Zone

```cpp
struct Zone {
    double volume_m3;           // Volume (m³)
    double height_m;            // Vertical extent (m)
    double T_K;                 // Temperature (K)
    double P_Pa;                // Pressure (Pa)
    std::vector<double> n_mol;  // Molar composition
    
    double heatContent_J() const;
    double density_kg_m3() const;
    double mass_kg() const;
};
```

### ComparisonStats

```cpp
struct ComparisonStats {
    double mean_error;      // Mean absolute error
    double max_error;       // Maximum absolute error
    double rmse;            // Root mean square error
    double correlation;     // Correlation coefficient
    int num_points;         // Number of comparison points
};
```

## Common Workflows

### Basic Fire Simulation

```cpp
vfep::Simulation sim;
sim.reset();
sim.ignite();

for (int i = 0; i < 1000; ++i) {
    sim.step(0.05);
    const auto& obs = sim.observe();
    
    if (obs.T_K > 500.0) {
        sim.enableSuppression();
    }
}
```

### Three-Zone Stratified Fire

```cpp
vfep::ThreeZoneModel model(3.0, 25.0, 5);
model.reset();

for (double t = 0; t < 60.0; t += 0.1) {
    double HRR = (t > 5.0) ? 100e3 : 0.0; // Ignite at t=5s
    model.step(0.1, HRR, 10e3, 2.0);
    
    std::cout << "t=" << t << "s, Upper T=" << model.upperZone().T_K << "K\n";
}
```

### Sensitivity Analysis

```cpp
vfep::SensitivityAnalyzer sa;

vfep::SensitivityAnalyzer::SweepParameters params;
params.dt_s = 0.05;
params.t_end_s = 30.0;
params.ignite_at_s = 2.0;

sa.setSweepParameters(params);

std::vector<double> hrr_values = {50e3, 75e3, 100e3, 125e3, 150e3};
auto results = sa.runSweep("heat_release_J_per_mol", hrr_values);

sa.exportCSV("sensitivity_results.csv", results);
```

### Uncertainty Quantification

```cpp
vfep::MonteCarloUQ uq;

vfep::MonteCarloUQ::ScenarioConfig scenario;
scenario.dt_s = 0.05;
scenario.t_end_s = 30.0;
scenario.ignite_at_s = 2.0;
uq.setScenario(scenario);

vfep::MonteCarloUQ::UQRanges ranges;
ranges.heat_release_J_per_mol = {75e3, 125e3};
ranges.h_W_m2K = {5.0, 15.0};
ranges.volume_m3 = {100.0, 150.0};
uq.setRanges(ranges);

auto summary = uq.runMonteCarlo(100);

std::cout << "Peak T: " << summary.peak_T_K.mean << " ± " 
          << summary.peak_T_K.std_dev << " K\n";
```

### CFD Comparison Workflow

```cpp
// 1. Generate mock CFD data
vfep::CFDInterface::generateMockCFD("cfd_result.vtk", 20, 20, 15, "room_fire");

// 2. Run VFEP simulation and collect results
std::vector<vfep::GridPoint> vfep_results;
// ... populate with VFEP data ...

// 3. Load CFD data
vfep::CFDInterface cfd;
cfd.importTemperatureField("cfd_result.vtk");

// 4. Compare
auto stats = cfd.compareTemperature(vfep_results, cfd_results);

// 5. Export comparison
cfd.exportComparisonCSV("comparison.csv", vfep_results, cfd_results);

std::cout << "Mean error: " << stats.mean_error << " K\n";
std::cout << "RMSE: " << stats.rmse << " K\n";
std::cout << "Correlation: " << stats.correlation << "\n";
```

## Testing

### Running Tests

```bash
cd build-mingw64
.\NumericIntegrity.exe    # 62/62 tests
.\ValidationSuite.exe      # 7/7 scenarios
```

### Test Coverage

- **57 Phase 1-7 tests**: Core integrity, physics, API safety
- **5 Phase 8 tests**: Three-zone model, CFD interface
- **7 validation scenarios**: Literature-validated fire scenarios

## Error Handling

All APIs use:
- Return values (`bool` for success/failure)
- Finite value checks (no NaN/Inf)
- Bounds checking on parameters
- Exception throwing for invalid construction

## Performance

- Single-zone simulation: ~0.3s per 60s real-time
- Three-zone model: ~0.5s per 60s real-time
- Monte Carlo (n=100): ~30s total
- All within <2s per 60s target ✅

## Build Requirements

- C++17 or later
- CMake 3.15+
- Compiler: MSVC, GCC, or MinGW

## See Also

- [User Guide](PHASE8_User_Guide.md) - Practical workflows
- [Technical Report](PHASE8_Technical_Report.md) - Validation details
- [Scenarios Catalog](PHASE8_Scenarios_Catalog.md) - Fire scenarios
