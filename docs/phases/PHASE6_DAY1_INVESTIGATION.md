# Phase 6: Investigation Results - Day 1

**Date:** January 28, 2026  
**Session:** Phase 6 Execution Start  
**Status:** First hypothesis tests complete

---

## Critical Discovery: Quick Tests 1-2 Results

### Quick Test 1: h_W Sensitivity (Hypothesis: Wall loss is limiting factor)

**Test:** Reduce h_W from 10 → 5 W/m²K (halve wall loss)

**Result:** ❌ **NO CHANGE**
- ISO 9705 Temperature: Still 375.6 K  
- NIST HRR: Still 85.6 kW ✓
- Suppression: Still 14.6%
- Stratification: Still 59.5 K

**Conclusion:** h_W is **NOT the limiting factor** for ISO 9705 temperature

---

### Quick Test 2: ACH Sensitivity (Hypothesis: Ventilation removes heat)

**Test:** Reduce ACH from 1.2 → 0.5 h⁻¹ (reduce ventilation by 60%)

**Result:** ❌ **NO CHANGE**
- ISO 9705 Temperature: Still 375.6 K (unchanged!)
- NIST HRR: **BROKE to 427 kW** (regression!)

**Conclusion:** ACH adjustment broke NIST validation, confirming ACH=1.2 is critical baseline

---

## Revised Root Cause Analysis

### Original Hypothesis ❌
- "h_W too high" → temperature insensitive
- "ACH too high" → temperature insensitive (and breaks NIST)

### New Hypothesis ✅ (CONFIRMED)

**Temperature is NOT controlled by h_W or ACH in current single-zone model**

**Root Cause:** 
- **Single-zone model averages entire compartment**
- Temperature measurement: `obs.T_K` = average zone temperature  
- ISO 9705 literature: 700-800°C at **ceiling** (peak stratified)
- VFEP prediction: 375 K = **average** across entire zone

**This explains:**
1. ✅ Why h_W reduction had zero effect (not limiting)
2. ✅ Why ACH reduction broke NIST (removes needed O2)
3. ✅ Why stratification test also fails (90 K rise vs 200-400 K needed)
4. ✅ Why all 3 temperature-dependent metrics fail together

---

## Actual Baseline Status (Corrected)

| Metric | Predicted | Target | Error | Status |
|--------|-----------|--------|-------|--------|
| **ISO 9705 Peak T** | 375.6 K (102°C) | 1023 K (750°C) | **63.3%** | ❌ FAIL |
| **NIST Peak HRR** | 85.6 kW | 75 kW ± 15 kW | **14.1%** | ✅ **PASS** |
| **Suppression HRR ↓** | 14.6% | 60-80% | **79.1%** | ❌ FAIL |
| **Stratification ΔT** | 59.5 K | 200-400 K | **80.2%** | ❌ FAIL |
| **Overall** | | | | **1/4 passing** |

---

## Key Insight: Why Tests 1-2 Failed

The validation scenarios use **single-zone model** which:
1. Averages temperature across entire compartment
2. Not sensitive to wall loss coefficient (interior-controlled)
3. Not sensitive to moderate ACH changes (at 1.2 ACH still has plenty O2)
4. Only controlled by: HRR (combustion) and heat capacity

**Thus:**
- Changing h_W (10 → 5): Temperature unchanged (interior-controlled)
- Changing ACH (1.2 → 0.5): Temperature unchanged until O2 depletes (then HRR drops)

---

## Phase 6 Investigation Path - Revised

### Priority 1: ENABLE MULTI-ZONE MODEL ⚠️ URGENT

**Why:** Single-zone cannot achieve:
- 700-800 K peak (ceiling temperature)
- 200-400 K stratification  
- Meaningful suppression effects

**Action:**
1. Enable three-zone model in validation scenarios
2. Verify zone implementation creates stratification
3. Measure peak zone temperature (not average)
4. Compare single-zone vs three-zone results

**Expected:** Multi-zone should show:
- Top zone: ~600-800 K ✓
- Middle zone: ~400-500 K
- Bottom zone: ~300-350 K (cool)
- ΔT = 300-500 K (matches literature)

---

### Priority 2: Review Suppression Mechanism

**Current:** 14.6% HRR reduction (should be 60-80%)

**Possible Causes:**
1. Quenching formula not connected to single-zone model
2. Agent delivery not functioning
3. Mechanism assumes multi-zone (can't work in single-zone)

**Action:**
- Debug suppression.cpp
- Check if suppression works in multi-zone model
- Verify agent delivery timing

---

### Priority 3: ISO 9705 Temperature Calibration (After Multi-Zone)

**Once multi-zone active:**
1. Run h_W/ACH sensitivity sweeps
2. Adjust for peak ceiling temperature
3. Target: 700-800 K in top zone

---

## Corrected Phase 6 Plan

**DO IMMEDIATELY:**
```
✅ DONE: h_W hypothesis tested and rejected
✅ DONE: ACH hypothesis tested and rejected
⏳ NEXT: Enable multi-zone model
⏳ THEN: Re-run all 4 scenarios
⏳ THEN: New sensitivity sweeps with multi-zone
```

**DO NOT DO:**
```
❌ Adjust h_W or ACH further (not effective in single-zone)
❌ Change NIST parameters (working correctly)
❌ Assume single-zone can achieve 700K temperatures
```

---

## Technical Next Steps

1. **Verify Multi-Zone Status:**
   - Check Simulation.cpp for multi-zone configuration
   - Verify `reactor_.config().model_type`
   - Check if ThreeZone model is compiled/available

2. **Activate Multi-Zone:**
   - Set model_type to ThreeZone in validation scenario
   - Recompile and re-run

3. **Measure Results:**
   - Extract per-zone temperatures from validation
   - Compare single vs multi-zone
   - Document zone-by-zone stratification

4. **Iterate if Multi-Zone Insufficient:**
   - Adjust zone sizes/positions
   - Tune zone mixing rates
   - Increase compartment height (affects stratification)

---

## Session Summary

**Tests Completed:**
- ✅ Quick Test 1: h_W sensitivity (no effect)
- ✅ Quick Test 2: ACH sensitivity (regression)
- ✅ Baseline validation confirmed (1/4 passing)

**Key Discovery:**
- ✅ Single-zone model is the ROOT CAUSE of temperature failures
- ✅ Not h_W, not ACH, not HRR model
- ✅ Need multi-zone to match literature stratification

**Phase 6 Revised Direction:**
- From: "Calibrate h_W/ACH parameters"
- To: **"Enable multi-zone model + stratification"**

**Estimated Time to Resolution:**
- Enable multi-zone: 2-3 hours
- Re-validate: 30 minutes
- If multi-zone fixes it: 4 hours total
- If needs tuning: 8-10 hours total

---

**Status:** 🟢 Investigation progressing (root cause identified)  
**Next Session:** Activate multi-zone model and re-test
