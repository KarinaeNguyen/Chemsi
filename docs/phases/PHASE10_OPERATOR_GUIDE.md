# VFEP Operator Quick Guide - Phase 10

**VFEP**: Versatile Fire Engineering Platform  
**Version**: Phase 10 Field Validation Build  
**Date**: February 12, 2026

---

## Quick Start

### Running a Validation Scenario

```bash
# Option 1: Run all scenarios
cd python_interface
python run_validation.py

# Option 2: Run single scenario (requires C++ integration)
cd build-mingw64
./VFEP_Sim.exe --scenario SC001 --duration 600
```

### Understanding Results

Results are saved to:
- `validation_report.csv` - Pass/fail summary for all scenarios
- `simulation_results.json` - Detailed time series and metrics
- Individual scenario outputs (if using VFEP_Sim directly)

---

## Scenario Selection Guide

### Scenario Classes

| Class | Description | Example Scenarios | Use When |
|-------|-------------|-------------------|----------|
| **thermal_buildup** | Slow temperature rise, no flashover | SC001, SC010, SC015 | Assessing long-duration heating |
| **flashover_timing** | Rapid transition to fully developed fire | SC003, SC006, SC012 | Predicting flashover time |
| **suppression_effectiveness** | Fire control via sprinklers/suppression | SC002, SC005, SC011, SC014 | Evaluating suppression systems |
| **ventilation_limited** | Fuel-rich, O2-starved combustion | SC004 | Low ACH or closed compartments |
| **ventilation_controlled** | High ACH, fuel-limited combustion | SC007, SC013 | High ventilation spaces |
| **multi_compartment** | Fire spread across rooms | SC008, SC009 | Buildings with multiple zones |

### Choosing a Scenario

1. **Match your compartment size**: 
   - Small (< 100 m³): SC001, SC002, SC011, SC012, SC013
   - Medium (100-200 m³): SC003, SC004, SC005, SC008, SC009, SC014, SC015
   - Large (> 200 m³): SC006, SC007, SC010

2. **Match your fire load**:
   - Light (< 1000 MJ): SC001, SC002, SC011, SC012, SC013, SC014
   - Moderate (1000-3000 MJ): SC003, SC004, SC005, SC008, SC009, SC015
   - Heavy (> 3000 MJ): SC006, SC007, SC010

3. **Match your ventilation** (ACH):
   - Low (< 2): SC001, SC004, SC006, SC012
   - Medium (2-4): SC003, SC005, SC008, SC009, SC014, SC015
   - High (> 4): SC007, SC013

4. **Match suppression strategy**:
   - No suppression: SC001, SC003, SC004, SC006, SC007, SC008, SC010, SC012, SC013, SC015
   - Early suppression (< 3 min): SC002, SC011
   - Delayed suppression (> 3 min): SC005, SC009, SC014

---

## Interpreting Validation Results

### Status Codes

- **PASS**: Prediction within acceptable bounds (green)
- **FAIL**: Prediction outside bounds, but model may still be useful (yellow)
- **MISSING**: Metric not computed - check simulation logs (red)

### Key Metrics

#### Thermal Metrics
- `peak_HUL_temp` (K): Maximum upper layer temperature
  - Flashover threshold: ~873 K (600°C)
  - Typical office: 600-700 K
  - Fully developed: 900-1200 K

#### Heat Release Rate
- `peak_HRR` (kW): Maximum fire power
  - Small fire: < 500 kW
  - Room fire: 500-2000 kW
  - Large fire: > 2000 kW

#### Time-Based
- `time_to_flashover` (s): Time to 873 K upper layer
  - Fast flashover: < 300 s
  - Moderate: 300-600 s
  - Slow/no flashover: > 600 s

#### Species Concentrations
- `final_O2_concentration` (fraction): O₂ at end time
  - Normal air: 0.21
  - Ventilation-limited: < 0.15
  - Dangerous for occupants: < 0.12

#### Suppression
- `suppression_time` (s): Time to control fire after activation
  - Effective: < 60 s
  - Moderate: 60-180 s
  - Delayed: > 180 s

---

## Common Issues & Troubleshooting

### Issue: High FAIL Rate in Validation Report

**Causes**:
- Mock simulation model (Phase 10 development) - not yet tuned
- Scenario parameters don't match field conditions exactly
- Measurement uncertainty in field targets

**Actions**:
- Check relative error - if < 20%, prediction may be acceptable
- Review scenario description and verify input parameters
- Document discrepancies for calibration refinement

### Issue: Simulation Crashes or Hangs

**Causes**:
- Invalid input parameters (negative values, zero volume)
- Numerical instability (very small timestep required)
- Memory allocation failure (large scenarios)

**Actions**:
- Verify all inputs are physical (positive values, reasonable ranges)
- Reduce simulation duration or increase timestep
- Check available RAM (at least 4GB)

### Issue: Missing Metrics in Results

**Causes**:
- Metric not applicable to scenario (e.g., suppression_time with no suppression)
- Simulation ended before event occurred
- Feature not yet implemented in VFEP_Sim

**Actions**:
- Check scenario definition (suppression_time_s = "none"?)
- Increase simulation duration
- Review VFEP_Sim CLI help for supported outputs

---

## Performance Expectations

| Scenario Size | Simulation Time (600s real-time) | Hardware Recommendation |
|---------------|----------------------------------|-------------------------|
| Small (SC001, SC002) | < 5 s | Any modern CPU |
| Medium (SC003-SC009) | 5-10 s | Quad-core CPU |
| Large (SC010) | 10-30 s | 8+ core CPU, 8GB RAM |

*Note: Times are for Release build with LTO on modern hardware. Debug builds are ~10x slower.*

---

## Best Practices

1. **Always run fast_test.ps1 before production runs** to ensure code stability
2. **Document scenario modifications** if you create custom scenarios
3. **Archive validation reports** with date/version for reproducibility
4. **Report unexpected behavior** to development team with scenario ID
5. **Use uncertainty bands** when communicating results to stakeholders

---

## Support & Contact

- **Technical Documentation**: `docs/phases/PHASE10_PLAN.md`
- **Data Schema**: `docs/phases/PHASE10_DATA_SCHEMA.md`
- **Code Repository**: See `REPOSITORY_ORGANIZATION.md`
- **Issue Tracker**: [TBD - internal tracking system]

---

**Last Updated**: February 12, 2026  
**Operator Training Required**: Yes (2-hour orientation recommended)  
**Certification Level**: Phase 10 Field Validation
