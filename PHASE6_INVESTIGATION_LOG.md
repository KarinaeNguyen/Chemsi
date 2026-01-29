# Phase 6: Initial Calibration Investigation
**Date:** January 29, 2026  
**Status:** ⏳ Investigation Phase - Test Leverage Identification

---

## Current Baseline (h_W = 10.0, ACH = 3.0)

```
Validation Scenario             Predicted       Target          Error       Status
─────────────────────────────────────────────────────────────────────────────────
ISO 9705 Room Corner Test       416 K           1023 K          59.3%       ❌ FAIL
NIST Data Center Rack Fire      85.6 kW         75 kW           14.1%       ✅ PASS
Suppression Effectiveness       7.4%            60-80%          89%         ❌ FAIL
Temperature Stratification      90 K            200-400 K       70%         ❌ FAIL
```

---

## Test Results: h_W Sensitivity

### Test 1: Does h_W (Wall Loss Coefficient) Control ISO 9705?

**Hypothesis:** h_W is too high (10 W/m²K), causing excessive heat loss → Lower h_W should increase peak temperature

**Test:** h_W = 5.0 W/m²K (50% reduction)

**Result:**
- Peak temperature: **373 K** (decreased from 416 K)
- Error from target: **63.5%** (worse than baseline)

**Conclusion:** ❌ **h_W is NOT the control lever**
- Reducing wall loss coefficient makes temperature WORSE
- h_W = 5.0 is *worse* than h_W = 10.0
- Suggests h_W near optimal or other factors dominate

### Implications
- Wall loss is not limiting peak temperature in ISO 9705
- Real problem: **Either**
  1. **Ventilation (ACH)** removes heat too fast
  2. **Room geometry** (volume/area ratio) is unfavorable
  3. **Reaction kinetics** fire isn't hot enough by design

---

## Next Investigative Steps

### Phase 6.1: ACH (Ventilation) Leverage Check
**Hypothesis:** ACH = 3.0 is too high → removes hot gases too fast  
**Test:** ACH ∈ {0.5, 1.0, 3.0, 5.0}  
**Expected outcome:**
- If ACH=0.5 → T >> 416K: ACH is a lever ✅
- If ACH=0.5 → T ≈ 416K: ACH isn't significant lever ❌

**Effort:** 20 minutes (4 builds + 4 runs)

### Phase 6.2: Room Geometry Impact
**If ACH isn't a lever:** Test volume/area ratio
**Leverage options:**
- Reduce volume (120 m³ → 50 m³)
- Increase area (proportionally)
- Test iso-surface temperature scaling

### Phase 6.3: Reaction Kinetics
**If ventilation & geometry aren't levers:** Check combustion model
- Heat release rate (currently 120 kJ/mol)
- Pyrolysis rate (currently 0.01 kg/s max)
- Fuel mass (currently tuned for NIST, may not work for ISO 9705)

---

## Numeric Integrity Test Suite Status

✅ **ALL 40/40 NUMERIC INTEGRITY TESTS PASSING**
- Phase 1B: dt-robustness (10 tests) ✅
- Phase 1C: state transitions (5 tests) ✅
- Phase 2A: physical consistency (12 tests) ✅
- Phase 3: interface & API (10 tests) ✅
- Phase 4A: suppression intensity (6 tests) ✅
- Soak variants: long-duration stability ✅

**Build Status:** ✅ CLEAN (core + tests)

**Note:** Visualization code (main_vis.cpp) has unrelated compilation errors - does not impact Phase 6 work

---

## Phase 6 Approach: Harmonious & Pragmatic

1. ✅ **Systematic parameter testing** using build automation
2. ✅ **Skip visualization** to avoid unrelated build issues
3. ✅ **Focus on ISO 9705 first** (highest-impact scenario)
4. ✅ **Preserve NIST passing** (regression test after each change)
5. ⏳ **Next: ACH leverage test** (today, 20 min)
6. ⏳ **Then: Geometry tuning** (if needed, 30 min)
7. ⏳ **Finally: Suppression & stratification** (multi-zone tuning)

---

## Critical Findings

### h_W is Counter-intuitive
- **Reducing** h_W from 10 to 5 **decreased** temperature
- This is opposite to initial diagnostic hypothesis
- Suggests: Wall loss may actually be helping (radiant + convection losses balance heat generation)

### Implication for Phase 6
- Cannot use h_W as the tuning lever for ISO 9705
- Must investigate other parameters (ACH, geometry, kinetics)
- Diagnostic analysis from Phase 5 may have been incomplete

---

## Commands for Next Session

```powershell
# Test ACH sensitivity (20 minutes)
cd d:\Chemsi
python phase6_ach_robust.py

# If ACH works: Apply best value and verify
cmake --build build-mingw64 --config Release --target VFEP_Sim

# If ACH doesn't work: Test geometry
# (manual edits to volume/area or try kinetics)

# Always verify NIST doesn't regress
cd build-mingw64
.\VFEP_Sim.exe
# Check console output: NIST HRR should be ~85 kW
```

---

## Success Criteria for Phase 6

- [ ] At least **2 of 4 scenarios passing** (50% validation)
- [ ] Root causes documented for each failure
- [ ] All 40+ numeric tests still passing
- [ ] NIST baseline protected (14.1% error or better)
- [ ] Build clean and reproducible

**Current:** 1/4 passing → **Target by end of Phase 6: 2-3/4 passing**

---

## Files Modified/Created

- `phase6_hw_test.py` - h_W sensitivity test (completed)
- `phase6_ach_robust.py` - ACH sensitivity test (ready to run)
- `NIST_TEST_VERIFICATION_REPORT.md` - Test suite verification (completed)

