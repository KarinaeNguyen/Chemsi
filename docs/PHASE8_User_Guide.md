# VFEP User Guide - Phase 8

**Ventilated Fire Evolution Predictor**  
**Date**: February 3, 2026

---

## Quick Start

### Building VFEP

```bash
# Clone repository
git clone <your-repo-url>
cd VFEP

# Configure
cmake -G "MinGW Makefiles" -B build-mingw64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build-mingw64 --config Release

# Test
cd build-mingw64
.\NumericIntegrity.exe    # 62/62 tests
.\ValidationSuite.exe      # 7/7 scenarios
```

### Your First Simulation

```cpp
#include "Simulation.h"
#include <iostream>

int main() {
    vfep::Simulation sim;
    sim.reset();
    sim.ignite();
    
    for (int i = 0; i < 1200; ++i) {  // 60 seconds at dt=0.05
        sim.step(0.05);
        
        if (i % 20 == 0) {  // Print every second
            const auto& obs = sim.observe();
            std::cout << "t=" << obs.time_s << "s, T=" << obs.T_K 
                     << "K, HRR=" << obs.HRR_W << "W\n";
        }
    }
    
    return 0;
}
```

---

## Core Workflows

### 1. Basic Fire Simulation

Run a simple fire scenario:

```cpp
vfep::Simulation sim;
sim.reset();        // Initialize to ambient conditions
sim.ignite();       // Start fire

for (int i = 0; i < 1000; ++i) {
    sim.step(0.05); // Advance by 0.05s timestep
    
    const auto& obs = sim.observe();
    
    // Monitor temperature
    if (obs.T_K > 600.0) {
        std::cout << "High temperature detected!\n";
        sim.enableSuppression();  // Activate suppression
    }
}
```

### 2. Three-Zone Stratified Fire

Model fire with vertical stratification:

```cpp
#include "ThreeZoneModel.h"

vfep::ThreeZoneModel model(
    3.0,   // Height (m)
    25.0,  // Floor area (m²)
    5      // Number of species
);

model.reset();

for (double t = 0; t < 60.0; t += 0.1) {
    double HRR_W = (t > 5.0) ? 100e3 : 0.0;  // Start fire at t=5s
    double cooling_W = 10e3;                  // Wall heat loss
    double ACH = 2.0;                         // Ventilation rate
    
    model.step(0.1, HRR_W, cooling_W, ACH);
    
    // Query zone temperatures
    std::cout << "Upper: " << model.upperZone().T_K << "K, "
              << "Middle: " << model.middleZone().T_K << "K, "
              << "Lower: " << model.lowerZone().T_K << "K\n";
}

std::cout << "Smoke layer height: " << model.smokeLayerHeight_m() << " m\n";
```

### 3. Parameter Sensitivity Analysis

Understand how parameters affect outcomes:

```cpp
#include "SensitivityAnalysis.h"

vfep::SensitivityAnalyzer sa;

// Configure base scenario
vfep::SensitivityAnalyzer::SweepParameters params;
params.dt_s = 0.05;
params.t_end_s = 30.0;
params.ignite_at_s = 2.0;
params.base_heat_release_J_per_mol = 100e3;
params.base_h_W_m2K = 10.0;
params.base_volume_m3 = 120.0;
params.base_pyrolysis_max_kgps = 0.02;

sa.setSweepParameters(params);

// Sweep heat release rate
std::vector<double> hrr_values = {50e3, 75e3, 100e3, 125e3, 150e3};
auto results = sa.runSweep("heat_release_J_per_mol", hrr_values);

// Export results
sa.exportCSV("hrr_sensitivity.csv", results);

std::cout << "Peak temperatures for HRR sweep:\n";
for (size_t i = 0; i < results.parameter_values.size(); ++i) {
    std::cout << "HRR=" << results.parameter_values[i] / 1000.0 << " kJ/mol, "
              << "Peak T=" << results.peak_temperatures[i] << " K\n";
}
```

### 4. Uncertainty Quantification

Quantify uncertainty with Monte Carlo:

```cpp
#include "UncertaintyQuantification.h"

vfep::MonteCarloUQ uq;

// Define scenario
vfep::MonteCarloUQ::ScenarioConfig scenario;
scenario.dt_s = 0.05;
scenario.t_end_s = 30.0;
scenario.ignite_at_s = 2.0;
scenario.pyrolysis_max_kgps = 0.02;
scenario.heat_release_J_per_mol = 100e3;

uq.setScenario(scenario);

// Define uncertainty ranges
vfep::MonteCarloUQ::UQRanges ranges;
ranges.heat_release_J_per_mol = {75e3, 125e3};  // ±25%
ranges.h_W_m2K = {5.0, 15.0};                   // Wall heat transfer
ranges.volume_m3 = {100.0, 150.0};              // Room volume
ranges.pyrolysis_max_kgps = {0.015, 0.025};     // Fuel burn rate

uq.setRanges(ranges);

// Run Monte Carlo with 100 samples
auto summary = uq.runMonteCarlo(100);

std::cout << "Peak Temperature:\n";
std::cout << "  Mean: " << summary.peak_T_K.mean << " K\n";
std::cout << "  Median: " << summary.peak_T_K.median << " K\n";
std::cout << "  Std Dev: " << summary.peak_T_K.std_dev << " K\n";
std::cout << "  95% CI: [" << summary.peak_T_K.ci_lower_95 << ", "
          << summary.peak_T_K.ci_upper_95 << "] K\n";

std::cout << "\nPeak HRR:\n";
std::cout << "  Mean: " << summary.peak_HRR_W / 1000.0 << " kW\n";
std::cout << "  95% CI: [" << summary.peak_HRR_W.ci_lower_95 / 1000.0 << ", "
          << summary.peak_HRR_W.ci_upper_95 / 1000.0 << "] kW\n";
```

### 5. CFD Comparison

Compare VFEP with CFD results:

```cpp
#include "CFDInterface.h"

// Generate mock CFD data for testing
vfep::CFDInterface::generateMockCFD(
    "cfd_room_fire.vtk",
    20, 20, 15,      // Grid dimensions
    "room_fire"      // Scenario
);

// Load CFD data
vfep::CFDInterface cfd;
cfd.importTemperatureField("cfd_room_fire.vtk");
cfd.importVelocityField("cfd_room_fire.vtk");

// Query at specific locations
double T_center = cfd.interpolateTemperature(2.5, 2.5, 1.5);
std::cout << "CFD Temperature at room center: " << T_center << " K\n";

double u, v, w;
cfd.interpolateVelocity(2.5, 2.5, 1.5, u, v, w);
std::cout << "CFD Velocity: (" << u << ", " << v << ", " << w << ") m/s\n";

// Compare with VFEP results
std::vector<vfep::GridPoint> vfep_results;
// ... populate with VFEP simulation results ...

auto stats = cfd.compareTemperature(vfep_results, cfd_results);

std::cout << "\nComparison Statistics:\n";
std::cout << "  Mean error: " << stats.mean_error << " K\n";
std::cout << "  Max error: " << stats.max_error << " K\n";
std::cout << "  RMSE: " << stats.rmse << " K\n";
std::cout << "  Correlation: " << stats.correlation << "\n";

// Export comparison for plotting
cfd.exportComparisonCSV("vfep_vs_cfd.csv", vfep_results, cfd_results);
```

---

## Advanced Topics

### Custom Scenarios

Create custom fire scenarios:

```cpp
vfep::Simulation sim;

// Configure custom geometry and properties
sim.resetToScenario({
    .volume_m3 = 100.0,
    .area_m2 = 80.0,
    .h_W_m2K = 12.0,     // Steel walls
    .ACH = 3.0,          // Ventilation
    .pyrolysis_kgps = 0.15,
    .heatRelease_J_per_molFuel = 450e3  // Hydrocarbon fuel
});

sim.ignite();

// Run simulation...
```

### Batch Processing

Run multiple scenarios:

```cpp
std::vector<ScenarioConfig> scenarios = {
    {"Scenario1", 100.0, 2.0, 100e3},
    {"Scenario2", 150.0, 3.0, 150e3},
    {"Scenario3", 120.0, 2.5, 125e3}
};

for (const auto& config : scenarios) {
    vfep::Simulation sim;
    sim.resetToScenario(config);
    sim.ignite();
    
    for (int i = 0; i < 1000; ++i) {
        sim.step(0.05);
    }
    
    const auto& obs = sim.observe();
    std::cout << config.name << ": Peak T = " << obs.T_K << " K\n";
}
```

---

## Troubleshooting

### Build Issues

**Problem**: CMake can't find compiler  
**Solution**: Ensure MinGW is in PATH or specify compiler explicitly

**Problem**: Missing dependencies  
**Solution**: Check that all required libraries are installed

### Runtime Issues

**Problem**: NaN or Inf values  
**Solution**: Check timestep size (try smaller dt), verify initial conditions

**Problem**: Simulation unstable  
**Solution**: Reduce timestep, check parameter ranges

### Performance

**Problem**: Simulation too slow  
**Solution**: 
- Increase timestep (if stable)
- Use Release build configuration
- Profile with larger timesteps for long simulations

---

## Best Practices

1. **Timestep Selection**: Start with dt=0.05s, adjust based on stability
2. **Validation**: Always compare against known benchmarks
3. **Parameter Ranges**: Keep within physically realistic bounds
4. **Testing**: Run NumericIntegrity.exe after changes
5. **Documentation**: Comment custom scenarios and parameter choices

---

## Support

- **Documentation**: See [docs/](../docs/) folder
- **Examples**: Check [docs/phases/](../docs/phases/) for detailed examples
- **API Reference**: [PHASE8_API_Reference.md](PHASE8_API_Reference.md)
- **Technical Details**: [PHASE8_Technical_Report.md](PHASE8_Technical_Report.md)

---

## Next Steps

1. Try the quick start example
2. Run validation scenarios
3. Experiment with parameter sweeps
4. Explore uncertainty quantification
5. Compare with CFD data (if available)

Happy simulating! 🔥
