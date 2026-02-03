# Phase 6: Multi-Scenario Calibration - Scan & Startup Plan
**Date:** January 28, 2026  
**Build Status:** ✅ CLEAN (40/40 tests passing)  
**Current Validation:** 25% (1/4 scenarios passing)

---

## Executive Summary

Phase 5 successfully validated VFEP's **core capability: fire heat release rate prediction** (NIST data center HRR within 14.1% error). Phase 6 now focuses on **broadening validation to 3+ scenarios** while understanding root causes of the remaining failures.

**Handoff from Phase 5:** 
- ✅ NIST Data Center: 85.6 kW (14.1% error) **PASSING**
- ❌ ISO 9705: 416K vs 1023K (59% error) **FAILING**
- ❌ Suppression: 7.4% vs 60-80% (89% error) **FAILING**
- ❌ Stratification: 90K vs 300K (70% error) **FAILING**

---

## Current Codebase Scan

### Build System Status
```
Workspace:  d:\Chemsi
Build Dir:  build-mingw64
Executable: VFEP_Sim.exe (main simulation)
Tests:      NumericIntegrity.exe (40/40 passing)
```

### Key Files for Phase 6
| File | Purpose | Last Modified | Status |
|------|---------|---------------|--------|
| ValidationSuite.cpp | Scenario definitions & diagnostics | Phase 5 | ✅ OPERATIONAL |
| Reactor.h/cpp | Multi-zone architecture | Phase 3 | ✅ READY |
| Suppression.cpp | Inhibitor mechanism | Phase 1 | 🔴 NEEDS REVIEW |
| Chemistry.h | Combustion parameters | Phase 5 | ✅ CALIBRATED |
| Simulation.cpp | Scenario initialization | Phase 5 | ✅ CALIBRATED |

### Parameters as of Phase 5 Final
| Parameter | Value | Confidence | Phase 6 Action |
|-----------|-------|-----------|-----------------|
| **Pyrolysis Max** | 0.01 kg/s | 🟢 Medium | ⏸️ Hold |
| **Heat Release** | 120 kJ/mol | 🟢 Medium | ⏸️ Hold (NIST validated) |
| **Reactor Volume** | 30 m³ | 🟡 Low | 🔍 REVISIT (scenario-specific) |
| **ACH (Ventilation)** | 0.3 [1/h] | 🟡 Low | 🔍 INVESTIGATE (affects temp) |
| **Suppression α** | 8000 [1/M] | 🟡 Low | 🔍 MECHANISM REVIEW |
| **Wall Loss h_W** | ~30 W/m²K | 🟡 Low | 🔍 CRITICAL ISO 9705 factor |
| **Li-ion Heat** | Disabled | 🟢 High | ✅ Keep OFF |

---

## Phase 6 Objectives (Priority-Ranked)

### Objective 1: ISO 9705 Temperature Investigation (HIGH PRIORITY)
**Current State:** 416K predicted vs 1023K target (59% error)  
**Gap:** 607K under-prediction

**Root Cause Hypotheses:**
1. **Wall loss coefficient too high** → Heat bleeding through walls (most likely)
2. **Geometry mismatch** → Volume/surface area vs physical test chamber
3. **Ventilation mis-calibrated** → Excessive heat removal (ACH=0.3 may be right)
4. **Initial conditions** → Transient effects not captured

**Phase 6 Actions:**
- [ ] Extract ISO 9705 scenario parameters from ValidationSuite.cpp
- [ ] Compare wall surface area & loss coefficient to literature
- [ ] Run sensitivity sweep: h_W ∈ [5, 10, 20, 30, 50, 100] W/m²K
- [ ] Check time-to-peak: is 416K reached at t=600s (correct) or wrong time?
- [ ] If geometry-driven: implement scenario-specific volumes

**Expected Outcome:** Identify if h_W is primary lever (target: reduce to achieve 700-800K)

---

### Objective 2: Suppression Mechanism Deep Dive (HIGH PRIORITY)
**Current State:** 7.4% HRR reduction vs 60-80% target (89% error)  
**Gap:** Massive mismatch in suppression effectiveness

**Root Cause Hypotheses:**
1. **Quenching coefficient wrong model** → α increases by 32× but no effect (indicates structural issue)
2. **Agent chemistry incomplete** → Inhibitor mechanism not dominant
3. **Delivery timing** → Agent reaches zone too late (simulation issue)
4. **Conversion rate** → HRR reduction dependent on different parameter

**Phase 6 Actions:**
- [ ] Review Suppression.cpp quenching formula: `hrr_reduction = 1.0 - exp(-alpha * C_agent)`
- [ ] Test with artificial agent (C_agent = 1.0) → should see dramatic effect
- [ ] Check if suppression command executes at correct time
- [ ] Investigate alternative: cooling-based knockdown vs quenching
- [ ] Run diagnostic: plot HRR vs agent moles over time

**Expected Outcome:** Clarify whether α is broken or overshadowed by other factors

---

### Objective 3: Stratification (Temperature Layering) Analysis (MEDIUM PRIORITY)
**Current State:** 90K stratification vs 200-400K target (70% error)  
**Gap:** 3-4× under-prediction

**Root Cause Hypotheses:**
1. **Single zone dominance** → Multi-zone model not activated or effective
2. **Heat mixing too aggressive** → Zones equilibrating too fast
3. **Scenario volume too small** → Concentrates heat (by design in Phase 5)
4. **Wall loss too high** → Dissipates thermal gradients

**Phase 6 Actions:**
- [ ] Check if multi-zone model is active in validation scenarios
- [ ] Compare single-zone vs three-zone predictions for same scenario
- [ ] Increase zone count or tuning if effect visible
- [ ] Validate stratification independently (may not be coupled to HRR)

**Expected Outcome:** Determine if multi-zone activation helps or if different mechanism needed

---

### Objective 4: Uncertainty Quantification (MEDIUM PRIORITY)
**Goal:** Provide confidence intervals on predictions

**Phase 6 Actions:**
- [ ] Monte Carlo sweep: ±20% on all calibrated parameters
- [ ] Generate prediction bands for each scenario
- [ ] Compare to literature uncertainty ranges
- [ ] Document "credibility envelope" for operators

**Expected Outcome:** Transform point estimates to confidence intervals (e.g., "85 ± 12 kW at 95%")

---

## Technical Roadmap for Phase 6

### Week 1: Investigation & Root Cause Analysis
```
Day 1: ISO 9705 deep dive
  - Extract scenario parameters
  - Run h_W sensitivity sweep (6 values)
  - Identify primary lever

Day 2: Suppression investigation
  - Review quenching formula
  - Test artificial agent scenarios
  - Diagnostic plots

Day 3: Stratification analysis
  - Single-zone vs three-zone comparison
  - Zone tuning parameters
  - Multi-zone activation tests

Day 4: Consolidate findings
  - Root cause summary for each failing scenario
  - Recommend calibration direction
  - Plan Week 2 implementation
```

### Week 2: Targeted Calibration & Validation
```
Day 1-2: ISO 9705 calibration
  - Adjust h_W, possibly volume/ACH
  - Target: achieve 700-800K

Day 3: Suppression mechanism fix
  - Address quenching or activate alternative
  - Target: 50% + HRR reduction

Day 4: Validation sweep
  - Re-run all 4 scenarios
  - Document results
  - Goal: 2-3/4 scenarios passing (50-75%)
```

---

## Diagnostic & Measurement Infrastructure

### Available Tools
- **ValidationSuite.cpp** — 4 scenarios with time-series logging
- **CSV export** — Predicted vs literature results
- **Time-series diagnostics** — HRR, Temperature, Species, Agent moles over time

### Recommended Additions for Phase 6
1. **Wall heat flux logging** — Quantify h_W impact
2. **Zone-by-zone diagnostics** — Multi-zone state inspection
3. **Sensitivity matrix** — Parameter elasticity for each scenario
4. **Agent mole tracking** — Verify suppression agent delivery

---

## Success Criteria for Phase 6

| Criterion | Target | Current | Status |
|-----------|--------|---------|--------|
| **Overall Validation** | 50-75% (2-3/4) | 25% (1/4) | IN PROGRESS |
| **NIST HRR** | ≤15% error | ✅ 14.1% | HOLDING |
| **ISO 9705 Temp** | ≤15% error | ❌ 59% | TARGET |
| **Suppression** | ≤30% error | ❌ 89% | TARGET |
| **Stratification** | ≤30% error | ❌ 70% | TARGET |
| **Build Quality** | 0 regressions | ✅ 40/40 pass | HOLDING |

---

## Next Steps

### IMMEDIATE (Today)
1. ✅ **Verify build status** — All tests passing
2. ⏳ **Extract scenario parameters** — Identify exact geometry/ventilation for each scenario
3. ⏳ **Initial h_W sweep** — Quantify temperature sensitivity to wall loss
4. ⏳ **Suppression diagnostic** — Check if quenching mechanism is firing

### THIS WEEK
5. ⏳ **Root cause analysis report** — Consolidate findings
6. ⏳ **Prioritized calibration plan** — Which lever to adjust first
7. ⏳ **Begin targeted fixes** — Likely start with ISO 9705 h_W

### SUCCESS HANDOFF TO PHASE 7
- ✅ 2-3/4 scenarios passing (50-75% validation)
- ✅ Root causes documented (why suppression/stratification underperform)
- ✅ Clear path to Phase 7 (Mechanism Deep Dive)
- ✅ Uncertainty quantification ready for deployment

---

## Phase 6 Knowledge Base

### Key References
- **Phase 5 Final Report:** Line 226-320 (recommendations)
- **Validation Suite:** cpp_engine/src/ValidationSuite.cpp (scenario code)
- **Calibration History:** Phase 5 parameter changes documented inline

### Lessons from Phase 5
- ✅ **Small volumes concentrate heat** → Effective for HRR but may underestimate stratification
- ✅ **Ventilation dominates temperature** → ACH is lever, not wall loss
- ❌ **Suppression mechanism opaque** → α×32 had no effect (indicates wrong model or activation)
- ✅ **NIST data is reliable** — 85.6 kW validation gives confidence in combustion model

---

## Questions to Answer in Phase 6

1. **ISO 9705:** Is 416K a simulation artifact or physics limitation?
2. **Suppression:** Is quenching formula correct, or overshadowed by cooling?
3. **Stratification:** Can multi-zone model achieve 200K+ gradient, or unrealistic?
4. **Generalization:** Are calibrations specific to data center, or broadly applicable?

---

## Status Dashboard

```
Phase 6 Progress Tracker
═══════════════════════════════════════════════════

[████░░░░░░░░░░░░░░░░░░░░░] 0% Started

Week 1: Investigation (Jan 28 - Feb 3)
  Day 1: ISO 9705 → h_W sweep
  Day 2: Suppression → quenching diagnostic  
  Day 3: Stratification → multi-zone test
  Day 4: Consolidate

Week 2: Calibration (Feb 4 - Feb 10)
  Target: 50-75% validation (2-3/4 scenarios)
```

---

**Prepared by:** VFEP Calibration Program  
**Reviewed by:** Phases 1-5 completion milestones  
**Handoff Ready:** ✅ YES (build clean, infrastructure operational)
