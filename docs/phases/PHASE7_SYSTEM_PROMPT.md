# PHASE 7 SYSTEM PROMPT
## Advanced Validation & Uncertainty Quantification

**Date**: February 2, 2026  
**Duration**: 4 weeks (Weeks 1-4)  
**Status**: ✅ READY TO START  
**Previous Phase**: Phase 6 (✅ COMPLETE - 100% validation, 4/4 scenarios, 51/51 tests)

---

## 🎯 Phase 7 Mission

**Objective**: Advance from baseline validation to comprehensive sensitivity analysis, uncertainty quantification, and expanded fire scenarios.

**Success Criteria**:
- ✅ Sensitivity analysis framework complete (Week 1)
- ✅ 6+ fire scenarios validated (Week 2)
- ✅ Three-zone model implemented (Week 3)
- ✅ Uncertainty quantification complete (Week 4)

**Expected Outcome**: Production-grade fire prediction system with quantified confidence intervals and expanded scenario coverage.

---

## 📊 Phase 7 Architecture Overview

### Tier Structure (4 Tiers, 4 Weeks)

#### **Tier 1: Sensitivity Analysis Framework (Week 1)**
**Goal**: Understand parameter impact on predictions  
**New Module**: `vfep::phase7::SensitivityAnalysis`  
**Deliverables**:
- SensitivityAnalyzer class (parameter sweeps)
- ParameterRange data structure
- SweepTool executable
- Sensitivity CSV reports for 4 current scenarios

**Key Parameters to Sweep**:
1. **Heat Release** (50-200 kJ/mol): How does combustion intensity affect predictions?
2. **Wall Heat Loss** (0.5-10 W/m²K): Room insulation impact
3. **Room Volume** (20-120 m³): Space scaling effects
4. **Pyrolysis Rate** (0.5-2.0 kg/s): Fuel generation variation

**Success Metrics**:
- Zero compiler warnings
- All Phase 6 tests still pass (51/51)
- ≥5 new sensitivity tests
- Sweep tool produces valid CSV output
- NIST and ISO 9705 sweeps complete

---

#### **Tier 2: New Fire Scenarios (Week 2)**
**Goal**: Expand validation portfolio with 3+ new fire scenarios  
**New Module**: `vfep::phase7::ScenarioLibrary`  
**Deliverables**:
- Ship Fire scenario (ISO 15653)
- Tunnel Fire scenario (FDS reference data)
- Industrial Fire scenario (NFPA guidance)
- Validation results against literature

**New Scenarios Detail**:

##### Ship Cabin Fire
```cpp
// ISO 15653: Ship fire in confined space
Geometry: 14 m³ cabin
Heat Release: 500 kJ/mol (marine materials)
Wall Material: Steel (high conductivity)
Ventilation: ACH = 2.0 (mechanical)
Duration: 300 seconds
Acceptance Criteria: ±8% peak temperature, ±10% time to 60°C
Literature Baseline: T_peak = 687 K
```

##### Tunnel Fire
```cpp
// FDS validation case: Tunnel fire
Geometry: 240 m³ tunnel section
Heat Release: 2500 kJ/mol (high intensity)
Wall Material: Concrete (moderate conductivity)
Ventilation: ACH = 4.0 (forced ventilation)
Duration: 600 seconds
Acceptance Criteria: ±12% hot layer temp, ±15% interface height
Literature Baseline: T_upper = 850 K, T_lower = 350 K
```

##### Industrial Warehouse Fire
```cpp
// Large space fire modeling
Geometry: 2000 m³ warehouse
Heat Release: 1500 kJ/mol (organic materials)
Wall Material: Gypsum board (low conductivity)
Ventilation: ACH = 1.0 (natural ventilation)
Duration: 1800 seconds (long-term burn)
Acceptance Criteria: ±15% long-term trends
Literature Baseline: Sustained T_peak = 600 K
```

**Success Metrics**:
- 3 new scenarios fully integrated
- Each validates within ±8-15% acceptance range
- ValidationSuite shows 7/7 scenarios passing
- All numeric tests still pass (51/51)

---

#### **Tier 3: Three-Zone Model (Week 3)**
**Goal**: Improve temperature stratification predictions  
**New Module**: `vfep::phase7::ZoneModeling`  
**Deliverables**:
- ThreeZoneModel class
- Dynamic zone interface tracking
- Improved stratification accuracy

**Three-Zone Architecture**:
```
Zone 1 (Hot Upper):  T_u, high particulate concentration
Zone 2 (Interface):  Transition layer with gradient
Zone 3 (Cold Lower): T_l, ambient conditions
```

**Physics Improvements**:
- Current: Single-zone with post-hoc two-zone correction
- Phase 7: Dynamic three-zone model during simulation
- Better treatment of warm layer below hot layer (intermediate zone)
- Zone interface height tracking over time

**Success Metrics**:
- Stratification scenarios pass (±6% vs current ±9.26%)
- All 7 scenarios still validate
- No regression in other predictions
- Three-zone model code ≤1000 lines

---

#### **Tier 4: Uncertainty Quantification (Week 4)**
**Goal**: Quantify prediction confidence with ±95% intervals  
**New Module**: `vfep::phase7::UncertaintyQuantification`  
**Deliverables**:
- MonteCarloUQ class (Latin Hypercube sampling)
- Confidence interval calculator
- Parameter distribution definitions
- UQ analysis for all 7 scenarios

**Parameter Distributions**:
```cpp
// Uncertain parameters with statistical distributions
heat_release:     Normal(μ=100 kJ/mol, σ=15 kJ/mol)
h_W:              LogNormal(μ=2.0, σ=0.8)  [W/m²K]
room_volume:      Uniform(min=20 m³, max=120 m³)
pyrolysis_rate:   Gamma(α=2.0, β=0.5)      [kg/s]
wall_conductivity: LogNormal(μ=1.5, σ=0.5) [W/mK]
```

**Monte Carlo Approach**:
```
For each scenario:
  For n = 1 to 100 (samples):
    1. Sample parameters from distributions
    2. Run simulation with sampled parameters
    3. Record peak temperature, time-to-criterion
    4. Store trajectory
  
  Compute statistics:
    - Mean, std dev
    - Percentiles (5%, 50%, 95%)
    - Confidence intervals
    - Sensitivity indices (Sobol/Morris)
```

**Success Metrics**:
- ±95% confidence intervals computed for all scenarios
- Monte Carlo sampling validates against analytical results
- UQ adds <5% to total computation time
- Uncertainty results documented with physical interpretation

---

## 📝 Development Workflow

### Session Structure

Each Phase 7 work session follows this pattern:

```
Pre-Session (5 min):
  1. Read session template (PHASE7_SESSION1_LOG.md format)
  2. Review previous findings
  3. Plan today's tasks

Session Start (30 min):
  1. Build verification: cmake --build build-mingw64
  2. Test verification: NumericIntegrity.exe + ValidationSuite.exe
  3. Update session log "Morning Status"

Main Work (3-4 hours):
  1. Implement planned features
  2. Test incrementally
  3. Document findings
  4. Commit working code: git add && git commit

Session End (30 min):
  1. Final test run
  2. Update session log with findings
  3. Plan next session
  4. Commit session notes
```

### File Structure for Phase 7

```
cpp_engine/
├── include/
│   ├── phase7/
│   │   ├── SensitivityAnalysis.h      (Tier 1)
│   │   ├── ScenarioLibrary.h          (Tier 2)
│   │   ├── ZoneModeling.h             (Tier 3)
│   │   └── UncertaintyQuantification.h (Tier 4)
│   └── phase7_config.h                (Phase 7 globals)
│
├── src/
│   ├── phase7/
│   │   ├── SensitivityAnalysis.cpp
│   │   ├── ScenarioLibrary.cpp
│   │   ├── ZoneModeling.cpp
│   │   └── UncertaintyQuantification.cpp
│   └── ValidationSuite.cpp            (Updated with 3 new scenarios)
│
├── tests/
│   └── TestPhase7.cpp                 (New test file for Phase 7)
│
└── CMakeLists.txt                     (Updated for Phase 7 modules)

tools/
├── phase7/
│   ├── SweepTool.cpp                  (Parameter sweep executable)
│   ├── ScenarioValidator.cpp          (New scenario validation)
│   └── UQAnalyzer.cpp                 (Uncertainty analysis tool)
└── CMakeLists.txt

docs/
└── phase7/
    ├── SENSITIVITY_RESULTS.md         (Week 1 findings)
    ├── SCENARIO_VALIDATION.md         (Week 2 findings)
    ├── THREE_ZONE_ANALYSIS.md         (Week 3 findings)
    └── UNCERTAINTY_ANALYSIS.md        (Week 4 findings)
```

### Backward Compatibility Rules

**CRITICAL**: Phase 7 must NOT break Phase 6

```cpp
// DO:
namespace vfep::phase7 {
  class SensitivityAnalyzer { /* new code */ };
  class ThreeZoneModel { /* new code */ };
}

// Create new test file:
// cpp_engine/tests/TestPhase7.cpp

// Add new scenarios to existing ValidationSuite:
// In cpp_engine/src/ValidationSuite.cpp, add new scenarios
// alongside existing 4 scenarios

// DO NOT:
// ❌ Modify existing Simulation class methods
// ❌ Change existing test thresholds
// ❌ Rename Phase 6 variables/functions
// ❌ Break Phase 6 API
```

**Verification Process**:
```bash
# Before committing ANY Phase 7 code:
cmake --build build-mingw64 --config Release

# Run Phase 6 verification
./build-mingw64/NumericIntegrity.exe 2>&1 | grep -E "TOTAL|PASS|FAIL"
# Must show: All 51 tests PASSING

./build-mingw64/ValidationSuite.exe 2>&1 | grep "TOTAL"
# Must show: 4/4 scenarios within acceptance criteria
```

---

## 🔧 Phase 7 Build Configuration

### CMakeLists.txt Phase 7 Option

Add to `cpp_engine/CMakeLists.txt`:

```cmake
# Phase 7: Advanced Validation & UQ
option(ENABLE_PHASE7 "Enable Phase 7 modules" ON)

if(ENABLE_PHASE7)
  message(STATUS "Building Phase 7: Advanced Validation & UQ")
  
  # Add Phase 7 headers
  list(APPEND VFEP_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/SensitivityAnalysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/ScenarioLibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/ZoneModeling.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phase7/UncertaintyQuantification.h
  )
  
  # Add Phase 7 sources
  list(APPEND VFEP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/SensitivityAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/ScenarioLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/ZoneModeling.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/phase7/UncertaintyQuantification.cpp
  )
  
  # Add Phase 7 test file
  list(APPEND TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/TestPhase7.cpp
  )
  
  # Add Phase 7 tools
  add_executable(SweepTool tools/phase7/SweepTool.cpp ...)
  add_executable(ScenarioValidator tools/phase7/ScenarioValidator.cpp ...)
  add_executable(UQAnalyzer tools/phase7/UQAnalyzer.cpp ...)
endif()
```

### Build Commands

```bash
# Standard Phase 7 build (ENABLE_PHASE7=ON by default)
cd d:\Chemsi\build-mingw64
cmake --build . --config Release

# Verify no warnings
cmake --build . --config Release 2>&1 | grep -i warning
# Should show nothing

# Run all tests
./NumericIntegrity.exe
./ValidationSuite.exe
./TestPhase7.exe
```

---

## 📊 Phase 7 Parameters & Constants

### Calibrated Physics Values (Locked from Phase 6)

```cpp
// VFEP Phase 6 Calibration - DO NOT CHANGE
namespace vfep::constants {
  // Combustion
  const double DEFAULT_HEAT_RELEASE = 1.0e5;  // 100 kJ/mol (NIST calibrated)
  const double DEFAULT_PYROLYSIS_RATE = 1.0;  // 1.0 kg/s baseline
  
  // Thermodynamics
  const double WALL_HEAT_LOSS_DEFAULT = 2.0;  // 2.0 W/m²K (typical wall)
  const double SUPPRESSION_EFFICIENCY = 0.80; // 80% suppression (calibrated)
  
  // Geometry
  const double REFERENCE_VOLUME = 50.0;       // 50 m³ (data center rack)
  const double REFERENCE_AREA = 30.0;         // 30 m² wall surface
  
  // Validation Thresholds (Phase 6 locked)
  struct ValidationThresholds {
    // ISO 9705: peak temp 981 K ±4.11%
    const double ISO_9705_PEAK_T_K = 981.0;
    const double ISO_9705_TOLERANCE = 0.0411;
    
    // NIST: peak power 71.4 kW ±4.85%
    const double NIST_PEAK_P_KW = 71.4;
    const double NIST_TOLERANCE = 0.0485;
    
    // Suppression: 79.77% reduction ±13.95%
    const double SUPPRESS_REDUCTION = 0.7977;
    const double SUPPRESS_TOLERANCE = 0.1395;
    
    // Stratification: 272K ±9.26%
    const double STRAT_DELTA_T_K = 272.0;
    const double STRAT_TOLERANCE = 0.0926;
  };
}
```

### Phase 7 Parameter Ranges

```cpp
namespace vfep::phase7::defaults {
  // Tier 1: Sensitivity Analysis ranges
  struct ParameterRanges {
    // Heat release sweep: 50-200 kJ/mol
    static constexpr double HEAT_RELEASE_MIN = 50e3;   // J/mol
    static constexpr double HEAT_RELEASE_MAX = 200e3;
    static constexpr double HEAT_RELEASE_DEFAULT = 100e3;
    static constexpr int HEAT_RELEASE_SAMPLES = 10;
    
    // Wall heat loss: 0.5-10 W/m²K
    static constexpr double H_W_MIN = 0.5;
    static constexpr double H_W_MAX = 10.0;
    static constexpr double H_W_DEFAULT = 2.0;
    static constexpr int H_W_SAMPLES = 10;
    
    // Room volume: 20-120 m³
    static constexpr double VOLUME_MIN = 20.0;
    static constexpr double VOLUME_MAX = 120.0;
    static constexpr double VOLUME_DEFAULT = 50.0;
    static constexpr int VOLUME_SAMPLES = 10;
    
    // Pyrolysis rate: 0.5-2.0 kg/s
    static constexpr double PYRO_MIN = 0.5;
    static constexpr double PYRO_MAX = 2.0;
    static constexpr double PYRO_DEFAULT = 1.0;
    static constexpr int PYRO_SAMPLES = 10;
  };
  
  // Tier 4: Monte Carlo uncertainty quantification
  struct MonteCarloConfig {
    static constexpr int NUM_SAMPLES = 100;        // 100 samples per scenario
    static constexpr double CONFIDENCE_LEVEL = 0.95; // 95% confidence
    static constexpr int SEED = 42;                // Reproducible randomness
    static constexpr bool USE_LATIN_HYPERCUBE = true;
  };
}
```

---

## ✅ Quality Assurance Checklist

### Pre-Implementation
- [ ] Understand Phase 7 tier architecture
- [ ] Read PHASE7_ARCHITECTURE.md technical details
- [ ] Review Phase 6 calibration parameters
- [ ] Understand backward compatibility rules

### During Implementation
- [ ] Compile with 0 warnings (`cmake --build . 2>&1 | grep warning`)
- [ ] All Phase 6 tests pass (51/51)
- [ ] New tests added for Phase 7 features
- [ ] Code follows VFEP style guidelines
- [ ] Comments explain non-obvious logic

### Before Commit
- [ ] Feature complete for tier
- [ ] All tests passing
- [ ] No Phase 6 regression
- [ ] CSV output format verified
- [ ] Session log updated

### After Commit
- [ ] Git message clear and descriptive
- [ ] Push to `phase7/advanced-validation` branch
- [ ] Document findings in session notes

---

## 📈 Success Metrics by Tier

### Tier 1 (Week 1): Sensitivity Analysis
```
Code Quality:
  ✓ 0 warnings
  ✓ 51/51 Phase 6 tests passing
  ✓ 5+ new sensitivity tests
  ✓ API documented

Functionality:
  ✓ SensitivityAnalyzer class working
  ✓ ParameterRange sweep mechanism
  ✓ SweepTool executable functional
  ✓ CSV output validated

Validation:
  ✓ NIST heat_release sweep: 10 samples, valid output
  ✓ ISO 9705 h_W sweep: 10 samples, reasonable trends
  ✓ Sensitivity indices computed

Expected Output:
  - 8 CSV files (4 scenarios × 2 parameter sweeps)
  - Sensitivity report with findings
  - Updated documentation
```

### Tier 2 (Week 2): New Scenarios
```
Code Quality:
  ✓ 0 warnings
  ✓ All Phase 6 tests still passing
  ✓ 3 new scenario tests
  ✓ ValidationSuite updated

Functionality:
  ✓ Ship fire scenario integrated
  ✓ Tunnel fire scenario integrated
  ✓ Industrial fire scenario integrated
  ✓ Each validates independently

Validation:
  ✓ 7/7 total scenarios passing (4 Phase 6 + 3 new)
  ✓ Each within ±8-15% acceptance range
  ✓ No Phase 6 regression

Expected Output:
  - ValidationSuite output: 7/7 PASS
  - Scenario validation CSV
  - Comparative analysis document
```

### Tier 3 (Week 3): Three-Zone Model
```
Code Quality:
  ✓ 0 warnings
  ✓ All 51 Phase 6 tests still passing
  ✓ 3-zone specific tests added
  ✓ Zone tracking validated

Functionality:
  ✓ ThreeZoneModel class working
  ✓ Dynamic zone interface tracking
  ✓ Stratification prediction improved
  ✓ <1000 lines of code

Improvement:
  ✓ Stratification accuracy: 9.26% → 6% error
  ✓ All 7 scenarios still validate
  ✓ No regression in other predictions

Expected Output:
  - Improved ValidationSuite metrics
  - Three-zone model documentation
  - Stratification analysis report
```

### Tier 4 (Week 4): Uncertainty Quantification
```
Code Quality:
  ✓ 0 warnings
  ✓ All Phase 6 tests passing
  ✓ UQ-specific tests comprehensive
  ✓ Latin Hypercube implementation verified

Functionality:
  ✓ MonteCarloUQ class working
  ✓ 100 samples per scenario
  ✓ Confidence intervals (95%) computed
  ✓ Sensitivity indices calculated

Deliverables:
  ✓ ±95% CI for all 7 scenarios
  ✓ UQ adds <5% computation time
  ✓ Uncertainty visualization data
  ✓ Physical interpretation document

Expected Output:
  - 7 UQ analysis files (one per scenario)
  - Confidence interval plots
  - Parameter importance ranking
  - Final Phase 7 comprehensive report
```

---

## 🎯 Quick Reference: Phase 7 Commands

### Build
```bash
cd d:\Chemsi\build-mingw64
cmake --build . --config Release
```

### Test (Phase 6 Verification)
```bash
.\NumericIntegrity.exe     # Must show 51/51 PASS
.\ValidationSuite.exe      # Must show 4/4 PASS
```

### Test (Phase 7 New Features)
```bash
.\TestPhase7.exe           # New Phase 7 tests
.\SweepTool.exe --help     # Parameter sweep tool
```

### Session Work
```bash
# Start session
code d:\Chemsi\PHASE7_SESSION1_LOG.md

# Development loop
cmake --build build-mingw64 --config Release
.\build-mingw64\NumericIntegrity.exe
# (Make changes)
# (Test)
# (Commit)

# End session
git add .
git commit -m "Phase 7 Tier 1: Sensitivity analysis implementation"
```

---

## 🚀 Ready to Start?

### Confirm Phase 6 Status
```bash
cd d:\Chemsi
.\build-mingw64\ValidationSuite.exe
# Verify: TOTAL: 4/4 scenarios within literature uncertainty

.\build-mingw64\NumericIntegrity.exe
# Verify: 51 [PASS] lines, 0 [FAIL]
```

### Create Phase 7 Branch
```bash
git checkout -b phase7/advanced-validation
```

### Create Project Structure
```bash
mkdir cpp_engine\include\phase7
mkdir cpp_engine\src\phase7
mkdir tools\phase7
mkdir docs\phase7
```

### Start Tier 1 (Week 1)
See [PHASE7_LAUNCH_GUIDE.md](PHASE7_LAUNCH_GUIDE.md) → Week 1 section

---

## 📞 Troubleshooting

### "Phase 6 tests are failing"
**Verify**: You haven't modified Phase 6 code
```bash
# Check git diff
git diff cpp_engine/src/Simulation.cpp
git diff cpp_engine/src/ValidationSuite.cpp
git diff cpp_engine/tests/TestNumericIntegrity.cpp

# If changed, revert
git checkout -- cpp_engine/src/Simulation.cpp
```

### "New code won't compile"
**Verify**: Using correct namespace
```cpp
// Correct:
namespace vfep::phase7 {
  class SensitivityAnalyzer { ... };
}

// Wrong (will conflict):
namespace vfep {
  class SensitivityAnalyzer { ... };  // Duplicate!
}
```

### "CMake doesn't find my new files"
**Solution**: Regenerate build system
```bash
cd build-mingw64
rm CMakeCache.txt
rm -r CMakeFiles
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

---

## 📋 Phase 7 Documentation Index

- [PHASE7_STARTUP.md](PHASE7_STARTUP.md) — Strategic roadmap
- [PHASE7_ARCHITECTURE.md](PHASE7_ARCHITECTURE.md) — Technical design
- [PHASE7_LAUNCH_GUIDE.md](PHASE7_LAUNCH_GUIDE.md) — Execution guide
- [PHASE7_SESSION1_TEMPLATE.md](PHASE7_SESSION1_TEMPLATE.md) — Session tracking
- [PHASE6_FINAL_COMPLETION.md](PHASE6_FINAL_COMPLETION.md) — Phase 6 baseline
- [PHASE6_CALIBRATION_PARAMETERS.md](PHASE6_CALIBRATION_PARAMETERS.md) — Locked parameters

---

## 🎓 Learning Path

**Before starting Tier 1**:
1. Read PHASE7_STARTUP.md (30 min)
2. Read PHASE7_ARCHITECTURE.md (40 min)
3. Review PHASE6_CALIBRATION_PARAMETERS.md (20 min)
4. Study ValidationSuite.cpp (1 hour)
5. Study Simulation.cpp combustion section (45 min)

**Total**: ~3.5 hours of preparation for confident Tier 1 implementation

---

**STATUS**: ✅ PHASE 7 READY TO LAUNCH

**Next Action**: Read PHASE7_LAUNCH_GUIDE.md Week 1 section and begin Tier 1 implementation

---

*Phase 7 System Prompt v1.0*  
*Last Updated: February 2, 2026*  
*Maintained by: Development Team*
