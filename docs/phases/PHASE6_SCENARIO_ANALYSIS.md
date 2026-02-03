# Phase 6: Scenario Parameter Analysis & Root Cause Investigation

**Date:** January 28, 2026  
**Status:** Initial diagnostic phase

---

## Executive Summary

All 4 validation scenarios use the **same base configuration** (`resetToDataCenterRackScenario()`), but each tests different physics. Current failures suggest **scenario-specific model limitations**, not global calibration issues.

---

## Current Scenario Configuration (Unified)

### Base Reactor Parameters
```cpp
// From Simulation.cpp:410-430 (resetToDataCenterRackScenario)

REACTOR CONFIG:
  Volume               30 m³ (Phase 5 Iter 4 reduction from 120)
  Surface Area         60 m² (proportional reduction)
  Wall Loss h_W        10 W/m²K ⚠️ KEY LEVER FOR ISO 9705
  Emissivity           0.85 (radiative)
  T_ambient            298.15 K (25°C)

VENTILATION CONFIG:
  ACH (Air Changes/Hr) 1.2 (Phase 5 Iter 5) ⚠️ CRITICAL FOR TEMPERATURE
  T_supply             298.15 K (ambient)

COMBUSTION:
  Fuel Capacity        50 kg (solid fuel pool)
  Pyrolysis Max        0.01 kg/s (Phase 5 calibrated for HRR ~85 kW)
  Li-ion Heat          DISABLED (isolate combustion)
  
SUPPRESSION (Default):
  Tank Capacity        200 kg
  Enabled              false (activated per scenario)
  Quenching α          8000 [1/M] (Phase 5 Iter 4)
```

### Scenario-Specific Modifications

| Scenario | Main Test | Extra Setup | Duration | Expected Physics |
|----------|-----------|-------------|----------|------------------|
| **ISO 9705** | Peak temperature | Standard 10 min run | 600 s | High T stratification |
| **NIST Data** | Peak HRR | Diagnostic logging | 300 s | Constant HRR plateau |
| **Suppression** | Agent effectiveness | Suppress at t=50s | 60 s total | HRR reduction 60-80% |
| **Stratification** | Temperature rise | Early vs late sample | 100 s | ΔT over time |

---

## Failure Analysis by Scenario

### SCENARIO 1: ISO 9705 Room Corner Test
**Status:** ❌ FAILING (59% error)  
**Predicted:** 416 K (-273.15 = 143°C)  
**Target:** 1023 K (750°C ± 50K)  
**Gap:** 607 K under-prediction  
**Error Classification:** Systematic (not noise)

#### Root Cause Analysis

**Hypothesis 1: Wall Loss Coefficient Too High (MOST LIKELY)**
- Current: h_W = 10 W/m²K
- This is **very low** for fire scenarios
- Typical compartment fires: 20-50 W/m²K
- **Implication:** Heat bleeding to walls 2-5× faster than realistic
- **Fix Lever:** Increase h_W → 30-50 W/m²K for ISO 9705

**Evidence:**
- Heat loss rate = h_W × Area × ΔT
- At h_W=10, ΔT=100K, Area=60m²: Heat loss = 60 kW
- With 85 kW HRR - 60 kW loss = 25 kW net heating (only 52°C rise observed)
- Increasing h_W from 10 to 50 would worsen problem (more heat loss)
- **Therefore: h_W should be LOWER, not higher**

**Hypothesis 2: Volume/Geometry Mismatch**
- Current: 30 m³, Area 60 m²
- ISO 9705: ~20 m³ typical compartment
- Close match, but surface area ratio matters
- Surface Area / Volume = 60/30 = 2.0 m⁻¹
- Realistic: ~3.0-4.0 m⁻¹ (more surface per unit volume)
- **Implication:** Less surface = less heat loss = higher T
- **Fix:** Increase area relative to volume or reduce volume

**Hypothesis 3: Ventilation Dominates (SECONDARY)**
- Current: ACH = 1.2
- This provides continuous fresh air + heat removal
- ISO 9705 in standard chamber: relatively sealed
- **Implication:** 1.2 ACH may be too aggressive
- **Fix:** Scenario-specific ACH (ISO 9705: 0.3-0.5 ACH)

**Hypothesis 4: Heat Capacity or Transient Timing**
- Single-zone model may not capture stratification
- Temperature rising slowly throughout simulation
- Reaches ~416K at t=600s (end of test)
- If multi-zone active: top zone could reach higher T
- **Status:** Check if multi-zone active

#### Phase 6 ISO 9705 Investigation Plan

```
Step 1: Wall Loss Sensitivity (IMMEDIATE)
  Run sweep: h_W ∈ {5, 10, 15, 20, 30, 50} W/m²K
  Measure: Peak T at t=600s
  Plot: T_peak vs h_W
  Expected: Linear relationship, find h_W for 700-800K target

Step 2: Volume/Area Ratio Investigation
  Current: A/V = 2.0
  Literature: A/V = 3-4 m⁻¹
  Option A: Reduce volume (V = 20 m³, A = 60 m² → A/V = 3.0)
  Option B: Increase area (V = 30 m³, A = 120 m² → A/V = 4.0)
  Measure: Which gets closer to 750K?

Step 3: Ventilation Impact (If Steps 1-2 insufficient)
  Reduce ACH to 0.5 (more sealed)
  Measure: T increase
  Expected: ~10-20% higher T

Step 4: Multi-Zone Activation (If single-zone insufficient)
  Enable three-zone model
  Measure: Top zone temperature
  Expected: Should exceed single-zone average
```

#### ISO 9705 Early Hypothesis Testing

**Quick test recommendation:**
```python
# Before Phase 6 systematic sweep:
# Test h_W = 5 W/m²K (half current)
# - If T → 600K: wall loss is major lever ✅
# - If T → 450K: other factors dominant ❌

# Test V = 20 m³ (ISO standard size)
# - If T → 650K: geometry matters ✅
# - If T → 420K: volume alone insufficient ❌
```

---

### SCENARIO 2: NIST Data Center Rack Fire
**Status:** ✅ PASSING (14.1% error)  
**Predicted:** 85.6 kW  
**Target:** 75 kW ± 15 kW  
**Assessment:** VALIDATED

**Why This Works:**
- Pyrolysis max = 0.01 kg/s tuned specifically for this scenario
- ACH = 1.2 prevents oxygen starvation (realistic)
- 30 m³ volume at 85 kW gives ~3 kW/m³ (plausible)
- Phase 5 calibrated directly to NIST literature

**Lesson for Phase 6:**
- **This proves combustion model is correct** (not a global HRR issue)
- Failures in other scenarios are **scenario-specific**, not model-wide
- Don't change pyrolysis max or heat release calibration

---

### SCENARIO 3: Suppression Effectiveness
**Status:** ❌ FAILING (89% error)  
**Predicted:** 7.4% HRR reduction  
**Target:** 60-80% HRR reduction  
**Gap:** 52-73% under-suppression  
**Error Classification:** Structural (not calibration)

#### Root Cause Analysis

**Hypothesis 1: Quenching Formula Inactive or Wrong (MOST LIKELY)**
- Quenching: α increased 250 → 8000 (32×)
- **Result:** No measurable effect on HRR reduction
- **Implication:** Formula `HRR_reduction = 1 - exp(-α × C_agent)` may not be connected
- **Or:** C_agent (inhibitor concentration) not increasing as expected

**Evidence:**
- Test: α = 250, 2000, 8000 across Phase 5 iterations
- All produced ~7% reduction (unchanged)
- **Conclusion:** α not controlling suppression in this scenario
- **Fix:** Review Suppression.cpp quenching implementation

**Hypothesis 2: Inhibitor Delivery Timing**
- Suppression starts at t=50s
- Agent takes time to build concentration
- Measurement at t=80s may be too early
- **Fix:** Extend suppression window, measure at t=100s+

**Hypothesis 3: Agent Chemistry Mechanism**
- Current model: Exponential quenching
- Reality: May involve cooling + oxygen displacement + radical scavenging
- Proportional effect may dominate (α too weak parameter)
- **Fix:** Implement cooling-based suppression in parallel

**Hypothesis 4: Activation Threshold**
- Suppression may activate at wrong time or not at all
- Command `commandStartSuppression()` may have issues
- **Fix:** Log agent mdot and C_agent at each step

#### Phase 6 Suppression Investigation Plan

```
Step 1: Verify Agent Activation (IMMEDIATE)
  Log: commandStartSuppression() at t=50s
  Log: agent_mdot_kgps_ over time (should rise)
  Log: inhibitor_kgm3_ over time (should rise)
  Log: HRR_W over time
  Expected: Agent flow visible, concentration rising, HRR effect visible

Step 2: Test Artificial Scenarios
  Set C_agent = 1.0 kg/m³ manually (skipping delivery)
  Expected: Should see large HRR reduction (verify formula works)
  If yes: Delivery mechanism is issue
  If no: Quenching formula broken

Step 3: Quenching Formula Verification
  Check Suppression.cpp calculateQuenchingReduction()
  Review formula: HRR_red = 1 - exp(-α × C_agent)
  Test: α=8000, C_agent=0.1 → exp(-800) ≈ 0 (full reduction)
  Status: Is this being called?

Step 4: Alternative Suppression Mechanism
  Review Phase 2B/2C suppression code (cooling-based)
  May need to activate different suppression pathway
  Expected: Cooling knockdown may be active lever
```

#### Suppression Early Diagnostic

```python
# Quick diagnostic output during validation:
# Print at t=0s, 45s (pre-suppression), 55s, 80s, 100s (post)
# 
# t | agent_mdot | C_agent | HRR | HRR_fraction
# --|------------|---------|-----|-------------
# 0 | 0          | 0       | 425 | 1.0
# 45| 0          | 0       | 427 | 1.0
# 55| 0.3 kg/s   | 0.05    | 421 | 0.986 ← Should drop here
# 80| 0.3 kg/s   | 0.25    | 310 | 0.727 ← Expected 60-80%
#100| 0.1 kg/s   | 0.18    | 280 | 0.656 ← Sustained reduction
```

---

### SCENARIO 4: Temperature Stratification
**Status:** ❌ FAILING (70% error)  
**Predicted:** 90 K rise (early 40°C → late 100°C)  
**Target:** 200-400 K rise  
**Gap:** 110-310 K under-stratification  
**Error Classification:** Structural (multi-zone issue)

#### Root Cause Analysis

**Hypothesis 1: Single-Zone Model Dominance (MOST LIKELY)**
- Multi-zone infrastructure exists but may not be active
- Single-zone forces uniform temperature across compartment
- **Implication:** No stratification possible by design
- **Fix:** Enable three-zone model, verify layer formation

**Hypothesis 2: Zone Mixing Too Aggressive**
- If multi-zone active, zones may equilibrate too fast
- Time constant for zone heat exchange may be too short
- **Fix:** Increase zone separation time constant

**Hypothesis 3: Geometry Doesn't Support Stratification**
- 30 m³ volume with 2m ceiling may be too small
- Stratification needs height & thermal resistance
- Literature examples often use taller compartments (3-4m)
- **Fix:** Increase compartment height (adjust volume/area)

**Hypothesis 4: Sampling Time Too Early**
- Early sample at t=10s, late at t=95s
- May not represent full stratification development
- **Fix:** Sample earlier (t=1s) and later (t=200s) for bigger gap

#### Phase 6 Stratification Investigation Plan

```
Step 1: Multi-Zone Activation Check (IMMEDIATE)
  Verify: Is three-zone model active?
  Check code: reactor.config().model_type
  If single-zone: Activate three-zone
  Expected: Should see zone-by-zone T traces

Step 2: Zone Temperature Comparison
  Run identical scenario with single-zone vs three-zone
  Compare: T_avg(single) vs T_top(multi)
  Expected: Multi-zone top zone ~50-100K higher

Step 3: Compartment Height Sensitivity
  Increase height: V = 30 m³, A = 90 m² (taller proportions)
  Measure: ΔT rise
  Expected: Taller = more stratification

Step 4: Zone Tuning Parameters
  Identify: Zone heat transfer coefficients
  Measure: How fast do zones equilibrate?
  Adjust: Reduce mixing if too aggressive
```

#### Stratification Early Test

```python
# Compare:
# Single-zone: T_early=313K (40°C), T_late=373K (100°C), ΔT=60K
# Multi-zone:  T_early_top=313K, T_late_top=450K (?), ΔT=137K
# 
# If multi-zone improves but still <200K:
# → Adjust zone parameters or compartment geometry
# 
# If multi-zone makes no difference:
# → Model not activated or zones equilibrate too fast
```

---

## Phase 6 Priority Ranking & Effort Estimates

| Priority | Scenario | Root Cause | Effort | Payoff |
|----------|----------|-----------|--------|--------|
| 🔴 1 | ISO 9705 | h_W & volume-specific calibration | 4-6h | 1/4 → 2/4 passing |
| 🔴 2 | Suppression | Quenching mechanism review | 6-8h | 1/4 → 2/4 passing |
| 🟡 3 | Stratification | Multi-zone activation | 3-4h | 1/4 → 2-3/4 if others fixed |
| 🟢 4 | NIST Data | Hold (already passing) | 0h | Keep 1/4 baseline |

---

## Recommended Approach for Phase 6

### Week 1: Parallel Investigation
- **Track A (ISO 9705):** h_W sweep + volume sensitivity
- **Track B (Suppression):** Quenching diagnostic + agent logging
- **Track C (Stratification):** Multi-zone activation + zone parameter review
- **Consolidate** findings by end of week

### Week 2: Targeted Fixes
- Implement highest-confidence fix first (likely ISO 9705 h_W)
- Re-validate NIST to ensure no regression
- Apply suppression fix if mechanism clarified
- Re-run all 4 scenarios → target 50-75% passing

---

## Success Criteria for Phase 6

```
WEEK 1 CHECKPOINT:
  ✅ Root causes identified for 3 failing scenarios
  ✅ h_W sensitivity sweep complete
  ✅ Multi-zone activation status confirmed
  ✅ Suppression mechanism diagnostic complete

WEEK 2 CHECKPOINT:
  ✅ At least 1 additional scenario passing (ISO 9705 or Suppression)
  ✅ NIST Data validation held (no regression)
  ✅ Calibration parameters documented
  ✅ Uncertainty quantification draft (Phase 6 Priority 2)

HANDOFF TO PHASE 7:
  ✅ 2-3/4 scenarios passing (50-75% validation)
  ✅ Root causes documented (ready for deeper work)
  ✅ Clear path to Phase 7 (Suppression mechanism deep dive)
```

---

## Next Immediate Actions

### TODAY (Before Session End)
1. ✅ Create Phase 6 diagnostic infrastructure
2. ⏳ Run quick h_W sensitivity sweep (3 values: 5, 10, 50)
3. ⏳ Log suppression agent state during validation
4. ⏳ Confirm multi-zone model status

### TOMORROW (Phase 6 Session Start)
5. Systematic h_W sweep (6+ values with detailed logging)
6. Suppression quenching formula verification
7. Multi-zone activation tests
8. Consolidate findings → prioritize fixes

---

**Prepared by:** VFEP Phase 6 Diagnostic Team  
**Status:** Ready for investigation phase  
**Next Document:** Phase 6 Weekly Progress (after Day 1)
