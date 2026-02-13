# Phase 5 Calibration: Status Report
**Date:** January 28, 2026  
**Status:** **IN PROGRESS - DEEPER INVESTIGATION REQUIRED**

---

## Parameters Adjusted (Iteration 1)

| Parameter | Original | Target | New Value | Rationale |
|-----------|----------|--------|-----------|-----------|
| **Arrhenius A** | 2e6 | 3.5e5 | 3.5e5 [1/s] | ÷5.7 for HRR reduction |
| **Quenching α** | 250 | 2000 | 2000 [1/M] | ×8 for suppression boost |
| **Pyrolysis Max** | 0.06 | 0.0105 | 0.0105 [kg/s] | ÷5.7 for HRR cap |
| **Li-ion Runaway** | Enabled | Disabled | false | Isolate combustion HRR |

---

## Results: NO IMPROVEMENT

### Validation Scores (Unchanged)
- ISO 9705 Peak T: **59% error** (unchanged)
- NIST HRR: **470% error** (unchanged)
- Suppression: **89% error** (unchanged)
- Stratification: **70% error** (unchanged)

### Predicted Values (Identical to Pre-Calibration)
- Peak HRR: **427.7 kW** (target: 60-90 kW)
- Peak T: **143°C** (target: 700-800°C)
- Suppression: **7.4% reduction** (target: 60-80%)
- ΔT: **90.6 K** (target: 200-400 K)

---

## Root Cause Analysis

### Why Calibration Had Zero Effect

**Hypothesis 1: Validation scenarios override base parameters**
- The `resetToDataCenterRackScenario()` method may restore hard-coded values
- Changes to base parameters in Chemistry.h ignored
- ✅ **Verified:** pyrolysisMax change IS in reset method (line 452)
- ✅ **Verified:** Li-ion disabled in reset method (line 473)

**Hypothesis 2: Cached/stale executable**
- Build system not detecting source changes
- ❌ **Rejected:** CMake rebuilt Simulation.cpp (confirmed in build log)

**Hypothesis 3: Physics model structural issue**
- HRR is dominated by non-combustion sources
- Temperature rise limited by over-ventilation
- Suppression effectiveness depends on different mechanism

**Hypothesis 4: Validation test design mismatch**
- DataCenterRack scenario not comparable to literature benchmarks
- ISO 9705 (20 m³ room) vs data center rack (different geometry)
- Time scales mismatched (validation runs too short/long)

---

## Diagnostic: What's Actually Happening?

### HRR Behavior
- Predicted 428 kW consistently across all scenarios
- Pyrolysis max reduced from 0.06 → 0.0105 kg/s (5.7x reduction)
- **Expected HRR with 0.0105 kg/s:**
  - Fuel heat of combustion ≈ 20 MJ/kg
  - HRR = 0.0105 kg/s × 20 MJ/kg = **210 kW**
- **Still predicting 428 kW** → Something else adding 218 kW

**Possible sources of extra HRR:**
1. Li-ion runaway (disabled but may have residual)
2. Initial transient heat release
3. Measurement artifact (when HRR sampled)
4. Different pyrolysis ramp mechanism

### Temperature Behavior
- Stuck at ~143°C regardless of HRR
- Target: 700-800°C
- **Indicates:** Massive heat loss or ventilation

**Ventilation check needed:**
- ACH (air changes per hour) may be too high
- Cooling coefficient may be over-tuned
- Need to reduce heat removal by 5x

### Suppression Behavior
- 7% HRR reduction (target: 60-80%)
- Quenching α increased 250 → 2000 (8x)
- **No improvement** → α not controlling suppression in this scenario

**Alternative suppression mechanisms:**
1. Exposure-based knockdown (Phase 2B/2C)
2. Cooling rate dominance
3. Agent delivery rate limitation

---

## Phase 5 Next Steps

### Immediate Action Required

**1. Diagnose HRR Source** (1 hour)
   - Add debug prints to ValidationSuite
   - Log: combustionHRR_W, liionHeat_W_, pyrolysis_kgps, fuel consumed
   - Identify what's contributing the 428 kW

**2. Diagnose Temperature Cap** (1 hour)
   - Check ventilation ACH in scenario
   - Review cooling coefficient application
   - Calculate expected steady-state T for 428 kW input

**3. Diagnose Suppression Mechanism** (1 hour)
   - Check if suppression command actually activates
   - Review knockdown calculation (exposure-based vs instant quenching)
   - Verify agent delivery rate is non-zero

### Medium-Term Strategy

**Option A: Fix Validation Test Design** (2-3 days)
- Create custom scenarios that match literature exactly
- ISO 9705: 20.7 m³, specific ventilation, exact fuel load
- NIST: Match geometry, initial conditions precisely
- **Pros:** More defensible comparison
- **Cons:** Requires validation suite rewrite

**Option B: Accept Model Limitations & Document** (1 day)
- Acknowledge model is "data center rack fire" specialized
- Compare only to data center fire literature (not general compartment fires)
- Adjust target ranges to match actual scenario type
- **Pros:** Fast, honest
- **Cons:** Narrow applicability

**Option C: Systematic Parameter Sweep** (1 week)
- Run 100+ scenarios varying all parameters
- Use optimization algorithm (gradient descent, genetic algorithm)
- Find parameter set that minimizes error across all 4 benchmarks
- **Pros:** Thorough, may find global optimum
- **Cons:** Time-intensive, may overfit

---

## Credibility Assessment

### Current: **45%** (unchanged from Phase 4)
- Architecture sound ✅
- Numeric integrity maintained (40/40 tests passing) ✅
- Calibration attempted but ineffective ⚠️
- Validation still 0/4 scenarios ❌

### Blocker to 90%+
**Cannot improve credibility without:**
1. Understanding WHY calibration had zero effect
2. Fixing root cause (physics model or test design)
3. Achieving at least 2/4 scenarios within bounds

---

## Honest Assessment for Deadline

### What We Know ✅
- Model is numerically stable
- Multi-zone architecture functional
- Validation framework operational

### What We Don't Know ❌
- Why HRR stays at 428 kW despite 5.7x pyrolysis reduction
- Why temperature caps at 143°C with 428 kW input
- Why suppression has 7% effect despite 8x α increase
- Whether validation tests match model's intended use case

### Recommendation
**PAUSE calibration, pivot to diagnostic phase:**
1. Add instrumentation to understand current behavior (2-3 hours)
2. Document model limitations honestly (1 hour)
3. Present "architectural validation" not "quantitative validation"

### Alternative Framing for Investors
> "We've built a structurally sound multi-zone fire model validated against 40 numeric integrity tests. Our Phase 4 literature benchmarking revealed systematic discrepancies (2x-10x) that require **model-scenario alignment** investigation before parameter calibration. The validation framework is production-ready; we need 2-3 days of diagnostic work to understand whether the model needs recalibration or the validation tests need redesign to match the data center fire scenario the model was architected for."

---

## Files Modified (Phase 5 Iteration 1)

1. `Chemistry.h` - Arrhenius A: 2e6 → 3.5e5
2. `Simulation.cpp` - Arrhenius A: 2e6 → 3.5e5
3. `ZoneExchange.h` - Quenching α: 250 → 2000
4. `Simulation.cpp` - Pyrolysis max: 0.06 → 0.0105
5. `Simulation.cpp` - Li-ion enabled: true → false

**Build Status:** ✅ All compiled, 40/40 tests passing  
**Runtime Status:** ✅ No crashes, identical output to pre-calibration  
**Validation Status:** ❌ 0/4 scenarios, NO improvement

---

## Next Session Action Plan

**Priority 1: Diagnostic (Critical)**
```cpp
// Add to ValidationSuite.cpp after each sim.step():
std::cout << "t=" << (i*0.05) << "s HRR=" << obs.HRR_W 
          << " T=" << obs.T_K << " pyro=" << /* need accessor */
          << " fuel=" << obs.fuel_kg << std::endl;
```

**Priority 2: Understand Scenario**
- Read `resetToDataCenterRackScenario()` completely
- Document all initial conditions
- Compare to ISO 9705 / NIST benchmarks

**Priority 3: Decide Path**
- If scenario matches literature → fix parameters deeper
- If scenario doesn't match → redesign validation tests
- If model is inherently limited → document scope

---

**END OF PHASE 5 STATUS REPORT**  
**Conclusion: Calibration ineffective, diagnostic phase required before iteration 2**
