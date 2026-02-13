# Phase 6: Complete Startup Package

**Date:** January 28, 2026  
**Status:** ✅ PHASE 6 READY TO EXECUTE  
**Build Status:** ✅ 40/40 tests passing  
**Documentation Status:** ✅ Complete

---

## Quick Navigation

### 📋 START HERE
- **[PHASE6_SESSION_INITIALIZATION.md](PHASE6_SESSION_INITIALIZATION.md)** — This session's summary (best overview)

### 🎯 Phase 6 Planning & Strategy
- **[PHASE6_SCAN_AND_PLAN.md](PHASE6_SCAN_AND_PLAN.md)** — Strategic roadmap, objectives, success criteria
- **[PHASE6_SCENARIO_ANALYSIS.md](PHASE6_SCENARIO_ANALYSIS.md)** — Root cause deep-dive per scenario

### 🔧 Tools & Diagnostics
- **[phase6_diagnostic.py](phase6_diagnostic.py)** — Run to see current status summary
- Use: `python phase6_diagnostic.py`

### 📊 Background Context
- **[PHASE5_FINAL_REPORT.md](PHASE5_FINAL_REPORT.md)** — Phase 5 calibration results (what we're building on)
- **[MASTER_PROGRAM_STATUS.md](MASTER_PROGRAM_STATUS.md)** — Full program history (Phases 1-2)
- **[CURRENT_STATE.md](CURRENT_STATE.md)** — Current build state, how to run tests

---

## Phase 6 At a Glance

### Current Validation Status
```
ISO 9705 Room Corner Test:     416 K → 1023 K target (59% error) ❌
NIST Data Center Rack Fire:    85.6 kW → 75 kW target (14% error) ✅ PASS
Suppression Effectiveness:     7.4% → 60-80% target (89% error) ❌
Temperature Stratification:    90 K → 200-400 K target (70% error) ❌
─────────────────────────────────────────────────────────────────
OVERALL:                       1/4 scenarios passing (25%) → TARGET: 50-75%
```

### What Needs to Happen

| Scenario | Root Cause | Fix | Effort | Priority |
|----------|-----------|-----|--------|----------|
| **ISO 9705** | h_W too high (10 W/m²K) | Reduce h_W → 5 W/m²K | 4-6h | 🔴 HIGH |
| **Suppression** | Mechanism not firing | Debug delivery & formula | 6-8h | 🔴 HIGH |
| **Stratification** | Single-zone model | Enable/tune multi-zone | 3-4h | 🟡 MEDIUM |
| **NIST Data** | ✅ Working! | HOLD (keep baseline) | 0h | 🟢 HOLD |

### Success Criteria
- ✅ At least 2 scenarios passing (50% minimum)
- ✅ Root causes documented
- ✅ All 40/40 tests still passing
- ✅ NIST baseline protected

---

## How to Use This Package

### For Quick Status Check
```powershell
# Show current validation status
python d:\Chemsi\phase6_diagnostic.py

# Expected output:
# - All 4 scenarios with predicted vs target
# - Root cause summary
# - Investigation plan
# - Quick tests
# - Next actions
```

### For Session Planning
1. Read: [PHASE6_SESSION_INITIALIZATION.md](PHASE6_SESSION_INITIALIZATION.md) (15 min)
2. Read: [PHASE6_SCENARIO_ANALYSIS.md](PHASE6_SCENARIO_ANALYSIS.md) (30 min)
3. Reference: [PHASE6_SCAN_AND_PLAN.md](PHASE6_SCAN_AND_PLAN.md) for detailed plan

### For Daily Work
1. Check: [PHASE6_SCENARIO_ANALYSIS.md](PHASE6_SCENARIO_ANALYSIS.md) for immediate tasks
2. Use: Quick hypothesis tests (TEST 1-3 in diagnostic)
3. Modify: Simulation.cpp parameters per Week 1 plan
4. Verify: NIST always passes after changes
5. Log: Results in weekly progress document

### For Implementation (Week 2)
1. Start: Priority 1 (ISO 9705 h_W fix)
2. Verify: NIST regression test
3. Then: Priority 2 (Suppression fix)
4. Then: Priority 3 (Stratification)
5. Final: All 4 scenarios validation sweep

---

## Build & Test Commands

### Verify Current State
```powershell
cd d:\Chemsi\build-mingw64
.\NumericIntegrity.exe
# Expected: [PASS] ... [PASS] ... (40 total)
```

### Rebuild After Code Changes
```powershell
cd d:\Chemsi
cmake --build build-mingw64 --config Release
```

### Run Validation & Check Results
```powershell
cd d:\Chemsi\build-mingw64
.\VFEP_Sim.exe
# Check: validation_results.csv for results
```

### Check Specific Scenario
Look for output in VFEP_Sim.exe console:
- ISO 9705 section (peak T)
- NIST section (peak HRR + diagnostics)
- Suppression section (HRR reduction)
- Stratification section (ΔT)

---

## Critical Do's and Don'ts

### ✅ DO:
- ✅ Always re-test NIST after changes (regression gate)
- ✅ Use diagnostic logging to verify changes
- ✅ Keep backups of working builds
- ✅ Document each change and result
- ✅ Run all 4 scenarios for final validation

### ❌ DON'T:
- ❌ Change pyrolysis max (0.01 kg/s) — proven working
- ❌ Change heat release (120 kJ/mol) — combustion validated
- ❌ Disable Li-ion heat completely without reason
- ❌ Assume changes help without testing all 4 scenarios
- ❌ Skip regression testing on NIST

---

## Investigation Roadmap

### Week 1: Diagnostic Phase (40-50 hours)
```
Day 1-2: h_W sensitivity sweep (ISO 9705)
  → Modify: h_W ∈ {5, 10, 15, 20, 30, 50} W/m²K
  → Measure: Peak T at each value
  → Goal: Find optimal h_W for 700-800K

Day 2-3: Suppression investigation
  → Add logging: inhibitor_kgm3_ time-series
  → Test: C_agent=1.0 (artificial)
  → Goal: Identify if delivery or formula broken

Day 3: Stratification verification
  → Check: Multi-zone model status
  → Compare: Single-zone vs three-zone
  → Goal: Understand stratification limitation

Day 4: Consolidate findings
  → Root cause confirmation
  → Prioritized fix list
  → Risk assessment
```

### Week 2: Implementation Phase (30-40 hours)
```
Priority 1: Apply h_W fix (8-10h)
  → Use optimal value from Week 1 sweep
  → Verify NIST holds ✓
  → Target: ISO 9705 PASS

Priority 2: Fix suppression (6-8h)
  → Implement identified fix
  → Verify delivery or formula
  → Target: 50%+ reduction

Priority 3: Activate multi-zone (3-4h)
  → Enable if needed
  → Tune zone parameters
  → Target: 150K+ stratification

Final validation (5-10h)
  → Re-run all 4 scenarios
  → NIST baseline check
  → Target: 2-3/4 passing
```

---

## Documentation Index

| Document | Purpose | Size | Read Time |
|----------|---------|------|-----------|
| **PHASE6_SESSION_INITIALIZATION.md** | Executive summary + session overview | 12 KB | 15 min |
| **PHASE6_SCAN_AND_PLAN.md** | Strategic plan + objectives | 47 KB | 40 min |
| **PHASE6_SCENARIO_ANALYSIS.md** | Root cause deep-dive + investigation | 42 KB | 45 min |
| **phase6_diagnostic.py** | Status visualization tool | 6 KB | Run only |
| **PHASE5_FINAL_REPORT.md** | Phase 5 calibration (background) | 30 KB | 30 min |

---

## Key Contacts & References

### VFEP Documentation
- Phase History: [MASTER_PROGRAM_STATUS.md](MASTER_PROGRAM_STATUS.md)
- Current Build: [CURRENT_STATE.md](CURRENT_STATE.md)
- Phase 5: [PHASE5_FINAL_REPORT.md](PHASE5_FINAL_REPORT.md)

### Code Locations
- **Simulation Scenarios:** cpp_engine/tests/ValidationSuite.cpp
- **Scenario Reset:** cpp_engine/src/Simulation.cpp (line 446+)
- **Wall Loss Config:** cpp_engine/src/Simulation.cpp (line 415)
- **Suppression Mechanism:** cpp_engine/src/Suppression.cpp

### Build System
- **CMake Config:** cpp_engine/CMakeLists.txt
- **Build Output:** build-mingw64/
- **Tests:** NumericIntegrity.exe (40 tests)
- **Main Sim:** VFEP_Sim.exe

---

## Success Indicators

### Week 1 Checkpoint (Day 4)
- ✅ h_W sensitivity sweep complete
- ✅ Suppression mechanism identified
- ✅ Multi-zone status confirmed
- ✅ Root causes documented
- ✅ Prioritized fix list created

### Week 2 Checkpoint (Day 10)
- ✅ ISO 9705 showing improvement (T > 500K)
- ✅ NIST still at 14.1% error (no regression)
- ✅ Suppression showing >20% reduction (progress)
- ✅ Stratification showing multi-zone effect (if active)

### Handoff to Phase 7
- ✅ 2-3/4 scenarios passing (50-75%)
- ✅ Root causes fully understood
- ✅ Parameters locked in code
- ✅ All 40/40 tests passing
- ✅ Build reproducible and committed

---

## Troubleshooting Quick Reference

### If build fails after code change
```powershell
# Clean rebuild
rm -r d:\Chemsi\build-mingw64\CMakeFiles
cmake --build d:\Chemsi\build-mingw64 --config Release
```

### If NIST validation regresses
```powershell
# Revert last change
git diff cpp_engine/src/Simulation.cpp
git checkout cpp_engine/src/Simulation.cpp
# Rebuild and re-test
```

### If tests fail
```powershell
cd d:\Chemsi\build-mingw64
.\NumericIntegrity.exe > test_log.txt
# Search log for [FAIL] entries
```

### If scenario looks wrong
```powershell
# Add diagnostic logging to ValidationSuite.cpp
# Run with VFEP_Sim.exe and capture console output
# Look for time-series (HRR, T, O2, inhibitor, etc.)
```

---

## Next Session Checklist

Before starting Phase 6 calibration work:

- [ ] Backup current `build-mingw64/` to `build-mingw64.backup/`
- [ ] Read [PHASE6_SESSION_INITIALIZATION.md](PHASE6_SESSION_INITIALIZATION.md)
- [ ] Read [PHASE6_SCENARIO_ANALYSIS.md](PHASE6_SCENARIO_ANALYSIS.md)
- [ ] Run `python phase6_diagnostic.py` for status
- [ ] Verify: `NumericIntegrity.exe` still shows 40/40 pass
- [ ] Start with TEST 1 (h_W=5 quick test)
- [ ] Create daily progress log
- [ ] Commit working builds to version control

---

## Session Statistics

**This Session (Diagnostic & Planning):**
- ✅ 4 comprehensive markdown documents created (128 KB total)
- ✅ 1 Python diagnostic tool created
- ✅ Build verified: 40/40 tests passing
- ✅ Root causes identified for 3 failing scenarios
- ✅ 2-week implementation plan created
- ✅ 0 lines of production code changed
- ⏱️ Estimated time to review all docs: 2 hours
- ⏱️ Estimated Phase 6 duration: 2 weeks (80 hours)

---

**Status:** 🟢 PHASE 6 READY TO EXECUTE

Next session begins with **Quick Test 1 (h_W sensitivity)** → 15 minutes  
Expected completion: 2 weeks (50-75% validation target)

---

*Document Package Created: January 28, 2026*  
*For Phase 6 Multi-Scenario Calibration*  
*VFEP Fire Safety Simulation Program*
