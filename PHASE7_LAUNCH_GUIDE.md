# Phase 7: Launch Guide

**Date**: February 2, 2026  
**Status**: ✅ READY TO START  
**Previous Phase**: Phase 6 (✅ COMPLETE - 4/4 scenarios, 51/51 tests)

---

## 🚀 Quick Start (5 Minutes)

### Step 1: Verify Phase 6 Baseline
```bash
cd d:\Chemsi

# Build
cmake --build build-mingw64 --config Release

# Verify 4/4 scenarios pass
.\build-mingw64\ValidationSuite.exe
# Should show: TOTAL: 4/4 scenarios within literature uncertainty

# Verify 51/51 tests pass
.\build-mingw64\NumericIntegrity.exe
# Should show 51 [PASS] lines, 0 [FAIL]
```

### Step 2: Create Phase 7 Branch
```bash
git checkout -b phase7/advanced-validation
git status  # Should show clean working tree
```

### Step 3: Create Project Structure
```bash
mkdir -p cpp_engine\include\phase7
mkdir -p cpp_engine\src\phase7
mkdir -p tools\phase7
mkdir -p docs\phase7
mkdir -p cmake\phase7
```

### Step 4: Copy Session Template
```bash
copy PHASE7_SESSION1_TEMPLATE.md PHASE7_SESSION1_LOG.md
# Fill in the template as you work
```

### Done! You're Ready for Phase 7 🎉

---

## 📚 Documentation to Read

Read in this order (total time: ~1.5 hours):

1. **[PHASE7_STARTUP.md](PHASE7_STARTUP.md)** ← START HERE
   - **Time**: 30 min
   - Overview of all Phase 7 work
   - Tier-by-tier breakdown
   - Success criteria

2. **[PHASE7_ARCHITECTURE.md](PHASE7_ARCHITECTURE.md)**
   - **Time**: 40 min
   - Technical design details
   - Class hierarchies and data structures
   - Integration points

3. **[PHASE6_FINAL_COMPLETION.md](PHASE6_FINAL_COMPLETION.md)**
   - **Time**: 20 min
   - What Phase 6 achieved
   - Current baseline metrics
   - Locked parameters

4. **[PHASE6_CALIBRATION_PARAMETERS.md](PHASE6_CALIBRATION_PARAMETERS.md)**
   - **Time**: 20 min
   - All current parameters documented
   - Where they're used in code
   - Why these values were chosen

---

## ✅ Pre-Launch Checklist

### Code Status
- [ ] Git branch created: `phase7/advanced-validation`
- [ ] Working directory clean (no uncommitted changes)
- [ ] Phase 6 build verified (cmake --build build-mingw64)
- [ ] Phase 6 ValidationSuite passes 4/4 scenarios
- [ ] Phase 6 NumericIntegrity passes 51/51 tests

### Documentation Status
- [ ] Read PHASE7_STARTUP.md
- [ ] Read PHASE7_ARCHITECTURE.md
- [ ] Read Phase 6 final docs (2 files)
- [ ] Understand Phase 7 tiers and objectives

### Environment Setup
- [ ] Project directories created (phase7 subdirectories)
- [ ] Session log created (PHASE7_SESSION1_LOG.md)
- [ ] CMakeLists.txt Phase 7 option understood
- [ ] Development tools ready (compiler, git, editor)

### Knowledge Check
- [ ] Can explain Phase 7 tiers (4 tiers, 4 weeks)
- [ ] Know the 3 new scenarios (Ship, Tunnel, Industrial)
- [ ] Understand sensitivity analysis objective
- [ ] Know why three-zone model is valuable
- [ ] Understand backward compatibility requirements

---

## 📅 Week 1 Plan: Sensitivity Analysis Framework

### Day 1: Design & Setup (6-8 hours)
```
Morning:
  - [ ] Read all Phase 7 documentation (1.5 hours)
  - [ ] Project structure review (0.5 hours)
  - [ ] Design SensitivityAnalysis class (2 hours)

Afternoon:
  - [ ] Design ParameterRange struct (0.5 hours)
  - [ ] Design SweepTool architecture (1 hour)
  - [ ] Plan CMakeLists.txt changes (0.5 hours)
  - [ ] First session update in log (0.5 hours)

Total: ~7.5 hours
```

### Day 2-3: Core Implementation (12-14 hours)
```
Day 2:
  - [ ] Create SensitivityAnalysis.h (1 hour)
  - [ ] Create SensitivityAnalysis.cpp skeleton (1 hour)
  - [ ] Implement heat release sensitivity (2 hours)
  - [ ] Implement h_W sensitivity (2 hours)
  - [ ] First compilation & debug (1 hour)
  
Day 3:
  - [ ] Implement geometry sensitivity (2 hours)
  - [ ] Implement pyrolysis sensitivity (2 hours)
  - [ ] CSV export method (1 hour)
  - [ ] Compilation & linking (1 hour)
  - [ ] Unit tests for sensitivity module (3 hours)

Total: ~14 hours
```

### Day 4-5: Integration & Validation (10-12 hours)
```
Day 4:
  - [ ] Create SweepTool.cpp (1 hour)
  - [ ] Command-line argument parsing (2 hours)
  - [ ] Integration with ValidationSuite (2 hours)
  - [ ] Testing the tool end-to-end (2 hours)

Day 5:
  - [ ] Run first NIST heat_release sweep (1 hour)
  - [ ] Run ISO 9705 h_W sweep (1 hour)
  - [ ] Analyze and document results (2 hours)
  - [ ] Write API documentation (1 hour)
  - [ ] Prepare for code review (0.5 hours)

Total: ~12 hours
```

### Week 1 Total: ~33.5 hours (achievable in 5-6 work days)

---

## 🎯 Week 1 Success Criteria

### Code Quality
- [ ] Zero compiler warnings
- [ ] All Phase 6 tests still pass (51/51)
- [ ] New sensitivity tests added (5+ tests)
- [ ] Code follows VFEP style guidelines

### Functionality
- [ ] SensitivityAnalyzer class complete
- [ ] SweepTool executable working
- [ ] CSV output format correct
- [ ] NIST scenario sensitivity sweep validated
- [ ] ISO 9705 scenario sensitivity sweep validated

### Documentation
- [ ] API documentation written
- [ ] Usage examples created
- [ ] Results analyzed and recorded
- [ ] Session log completed
- [ ] Findings documented

### Deliverable
- [ ] Git commit: `phase7/sensitivity-analysis-complete`
- [ ] CSV files: sensitivity results for all 4 current scenarios
- [ ] Report: Week 1 findings document

---

## 🔧 Development Environment Setup

### Compiler & Build
```bash
# Verify compiler
g++ --version    # Should be MinGW (part of MSYS2)

# Verify CMake
cmake --version  # Should be 3.20+

# Verify git
git --version    # Latest available
```

### IDE Setup (Recommended)
- **VS Code**: Use with C++ extension
- **Extensions to install**:
  - C/C++ (Microsoft)
  - CMake Tools (Microsoft)
  - Git Graph (mhutchie)
  - Better Comments (Aaron Bond)

### Build Configuration
```bash
# Create separate build directory for Phase 7 testing
mkdir build-phase7
cd build-phase7

# Configure with Phase 7 enabled
cmake .. -DENABLE_PHASE7=ON -G "MinGW Makefiles"

# Build
cmake --build . --config Release
```

---

## 📊 Progress Tracking

### Use the Session Log
Edit `PHASE7_SESSION1_LOG.md` daily to track:
- Tasks completed
- Time spent
- Issues encountered
- Key findings
- Next day priorities

### Weekly Review
Each Friday, create summary:
- Lines of code added
- Tests added (and passing)
- Performance metrics
- Blockers and solutions
- Confidence level for next week

### Metrics to Track
```
Week 1 Goal: SensitivityAnalysis module complete
  Lines of code: 500-800
  Tests added: 5+
  Build warnings: 0
  Test pass rate: 100%
```

---

## 🆘 Common Issues & Solutions

### Issue: CMake doesn't find new files
**Solution**: Regenerate build system
```bash
cd build-mingw64
rm -r CMakeFiles CMakeCache.txt
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

### Issue: Phase 6 tests break after changes
**Solution**: Ensure backward compatibility
```cpp
// Phase 7 code should NOT modify Phase 6 classes
// Use:
// - New namespace: vfep::phase7::*
// - New files in phase7/ subdirectories
// - Conditional compilation with ENABLE_PHASE7
// NOT:
// - Changes to existing Simulation class
// - Changes to ValidationSuite.cpp (only add new scenarios)
// - Changes to numeric test thresholds
```

### Issue: Performance degradation
**Solution**: Use profiler to identify bottlenecks
```cpp
PerformanceProfiler prof;
prof.startTimer("sensitivity_sweep");
analyzer.analyzeParameter(scenario, range, "peak_T_K");
prof.stopTimer("sensitivity_sweep");
prof.printReport();
```

---

## 📞 Getting Help

### Questions to Ask
1. **Technical**: "What's the best way to..."
2. **Architecture**: "Where should I implement..."
3. **Debugging**: "Why is this test failing..."

### Information to Provide
- What you're trying to do
- What you've already tried
- Error messages (full output)
- Code snippet (if relevant)
- Current status (which Phase 7 tier)

### Key Contacts (Internal Documentation)
- **Architecture Questions**: See PHASE7_ARCHITECTURE.md
- **Parameter Questions**: See PHASE6_CALIBRATION_PARAMETERS.md
- **API Questions**: See PHASE6_FINAL_COMPLETION.md (Section: Code Quality)

---

## 🎓 Learning Resources

### Code to Study
1. **[cpp_engine/src/ValidationSuite.cpp](cpp_engine/src/ValidationSuite.cpp)**
   - How existing scenarios are run
   - Pattern for new scenarios
   - CSV output generation

2. **[cpp_engine/src/Simulation.cpp](cpp_engine/src/Simulation.cpp)**
   - Combustion model implementation
   - How to override parameters
   - Main simulation loop

3. **[cpp_engine/tests/TestNumericIntegrity.cpp](cpp_engine/tests/TestNumericIntegrity.cpp)**
   - Testing patterns
   - Numeric verification approach
   - Edge case handling

### Concepts to Review
- **Sensitivity Analysis**: How to quantify parameter impact
- **Monte Carlo Methods**: Random sampling for uncertainty
- **Zone Models**: How three-zone fire model differs from single-zone
- **CFD Coupling**: Basic concepts (detailed in Phase 7 Week 4)

---

## ✨ Next Steps After Week 1

### If Week 1 Complete (Sensitivity Analysis Done)
→ Proceed to **Week 2: New Fire Scenarios**

### If Week 1 Delayed
→ Continue sensitivity analysis refinement

### If Issues Discovered
→ Document in session log and adjust timeline

---

## 📋 Approval Checklist

Before starting Phase 7 work, confirm:

### Phase 6 Complete?
- [ ] Yes, ValidationSuite shows 4/4 PASS
- [ ] Yes, NumericIntegrity shows 51/51 PASS
- [ ] Yes, 0 compiler warnings
- [ ] Yes, NIST baseline locked at 71.4 kW

### Documentation Read?
- [ ] Yes, PHASE7_STARTUP.md
- [ ] Yes, PHASE7_ARCHITECTURE.md
- [ ] Yes, PHASE6_FINAL_COMPLETION.md
- [ ] Yes, PHASE6_CALIBRATION_PARAMETERS.md

### Environment Ready?
- [ ] Yes, Phase 7 branch created
- [ ] Yes, project structure created
- [ ] Yes, session log created
- [ ] Yes, development tools verified

### Ready to Start?
- [ ] ✅ YES - Begin Phase 7 Week 1

---

## 🚀 Launch Commands

Ready to start? Run these commands:

```bash
# 1. Navigate to workspace
cd d:\Chemsi

# 2. Create and switch to Phase 7 branch
git checkout -b phase7/advanced-validation

# 3. Create project structure
mkdir -p cpp_engine\include\phase7
mkdir -p cpp_engine\src\phase7
mkdir -p tools\phase7

# 4. Copy session template
copy PHASE7_SESSION1_TEMPLATE.md PHASE7_SESSION1_LOG.md

# 5. Open in VS Code
code .

# 6. Read startup docs
# (Open PHASE7_STARTUP.md in editor)
```

---

## 📝 First Action Items

### Immediate (Next 30 minutes)
1. Read this launch guide (you are here ✅)
2. Open PHASE7_STARTUP.md in editor
3. Create session log from template
4. Verify Phase 6 baseline

### Today (First work session)
1. Read PHASE7_ARCHITECTURE.md
2. Review existing ValidationSuite.cpp
3. Design SensitivityAnalysis class
4. Create initial header file

### This Week (Full week of work)
1. Implement sensitivity analysis (Days 1-3)
2. Create sweep tool (Days 4-5)
3. Run and validate first parameter sweeps
4. Document findings

---

**Status**: ✅ READY FOR LAUNCH  
**Date**: February 2, 2026  
**Expected Start**: Immediately  
**Estimated Duration**: 4 weeks  
**Team Size**: 1-2 engineers  

🎉 **Welcome to Phase 7!** 🎉

Begin with PHASE7_STARTUP.md and proceed systematically through the tiers.
