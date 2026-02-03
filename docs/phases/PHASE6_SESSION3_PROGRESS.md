# Phase 6 Session 3: Validation Calibration Progress

**Date**: Session 3
**Status**: 1/4 scenarios passing (NIST ✓)
**Focus**: Combustion model calibration

## Current Results

| Scenario | Predicted | Target | Error | Status |
|----------|-----------|--------|-------|--------|
| ISO 9705 | 495K (222°C) | 1023K (750°C) | 51.7% | ❌ FAIL |
| NIST | 71.4 kW | 75 kW | 4.9% | ✅ PASS |
| Suppression | 0.17% | 70% | 99.8% | ❌ FAIL |
| Stratification | 17K ΔT | 300K ΔT | 94.2% | ❌ FAIL |

## Progress This Session

### NIST Calibration (COMPLETE ✓)
- **Problem**: Regressed from 14% → 471% error (428kW vs 75kW) after global pyrolysis_max reduction
- **Root cause**: combustion model (heatRelease=6.0e5 J/mol) was calibrated for thermal runaway, not room fires
- **Solution**: Reduced heatRelease from 6.0e5 → 1.0e5 J/mol (600→100 kJ/mol)
- **Result**: NIST now passing at 4.85% error ✓

### ISO 9705 Calibration (IN PROGRESS)
**History**:
- Initial: 392K (62% error)
- After pyrolysis_max=0.05: 312K (69% error) - WORSE
- After heatRelease=3.0e5 J/mol: 312K (no change)
- After setPyrolysisRate(0.05): 375K (63% error)
- After pyrolysis_max=0.15: 495K (52% error) - PROGRESS

**Key Findings**:
1. **Pyrolysis rate was bottleneck**: `commandIgniteOrIncreasePyrolysis()` only increments by 0.01 kg/s. Added `setPyrolysisRate()` to jump to full rate immediately.
2. **Geometry mismatch**: Using data center reactor (120 m³, 180 m², h=10 W/m²K) for ISO 9705 room corner test (should be ~20 m³ standard test room)
3. **Heat loss dominates**: Even at 0.15 kg/s pyrolysis + 3.0e5 J/mol (~4.5 MW HRR), only reaching 495K in 120 m³ volume

**Fundamental Issue**: Cannot achieve 1023K (750°C) in 120 m³ data center geometry with realistic combustion parameters. ISO 9705 requires:
- Smaller volume (20 m³ vs 120 m³)
- Different heat transfer coefficients
- Room confinement effects (smoke layer, upper/lower zones)

## Code Changes Made

### New Methods Added
1. **Reactor.h/cpp**: `combustionModel()` accessor
2. **Simulation.h/cpp**: 
   - `setCombustionHeatRelease(double J_per_mol)`
   - `setPyrolysisRate(double kgps)` - direct rate setter
3. **ValidationSuite.cpp**: 
   - Added `heat_release_J_per_mol` parameter to `runScenario()`
   - Scenario-specific calibration (ISO: 3.0e5 J/mol, NIST: 1.0e5 J/mol)

### Combustion Model Parameters
```cpp
// Simulation.cpp line ~354 (baseline)
cm.heatRelease_J_per_molFuel = 1.0e5; // 100 kJ/mol for NIST

// ValidationSuite.cpp ISO 9705
pyrolysis_max = 0.15 kg/s
heat_release = 3.0e5 J/mol (300 kJ/mol)

// ValidationSuite.cpp NIST (uses baseline)
pyrolysis_max = 0.01 kg/s
heat_release = 1.0e5 J/mol (100 kJ/mol)
```

## Next Steps

### Option A: Scenario-Specific Reactor Geometry (RECOMMENDED)
Modify `runScenario()` to configure reactor geometry per scenario:
```cpp
// ISO 9705: 3.6m × 2.4m × 2.4m room
rc.volume_m3 = 20.0;
rc.area_m2 = 50.0;
rc.h_W_m2K = 5.0; // better insulation

// NIST: keep data center default
rc.volume_m3 = 120.0;
rc.area_m2 = 180.0;
rc.h_W_m2K = 10.0;
```

### Option B: Zone Model for ISO 9705
Implement two-zone model (upper hot layer, lower cool layer) for more realistic room fire representation.

### Option C: Increase Combustion Intensity Further
- Try heatRelease = 5.0e5-8.0e5 J/mol (extreme values)
- Risk: breaks physical realism, fails other scenarios

### Suppression & Stratification
- **Suppression**: Needs investigation after ISO 9705 fixed (0.17% reduction is essentially none)
- **Stratification**: Related to ISO 9705 geometry issue - needs multi-zone or proper room confinement

## Files Modified
- `cpp_engine/include/Reactor.h`
- `cpp_engine/include/Simulation.h`
- `cpp_engine/src/Reactor.cpp` (implicit - added accessor)
- `cpp_engine/src/Simulation.cpp`
- `cpp_engine/src/ValidationSuite.cpp`

## Build Status
✓ All 51+ numeric tests passing
✓ ValidationSuite builds and runs
✓ NIST baseline stable at 4.85% error
