# Phase 9 Quick Reference Card

**Print this! Keep at your desk.** One-page developer reference for Phase 9 work.

---

## Phase 9 at a Glance

| Aspect | Details |
|--------|---------|
| **Status** | Ready to Launch |
| **Foundation** | 62/62 tests ✅ 7/7 scenarios ✅ |
| **Tracks** | 5 parallel (Radiation, Multi-Comp, CFD, Flame, ML) |
| **Target** | 70+ tests, 10 scenarios, <10s perf |
| **Start** | Radiation model (recommended) |
| **Duration** | 12-16 weeks total |

---

## Build Commands

```bash
# Build all
cmake --build build-mingw64 --config Release

# Run tests
cd build-mingw64
.\NumericIntegrity.exe      # 62+ tests should pass
.\ValidationSuite.exe        # 7+ scenarios should pass

# Clean rebuild
cmake --build build-mingw64 --config Release --clean-first
```

---

## File Organization

```
cpp_engine/
├── include/
│   ├── ThreeZoneModel.h      ← Phase 8 foundation
│   ├── CFDInterface.h         ← Phase 8 foundation
│   ├── RadiationModel.h       ← Phase 9 (create)
│   ├── CompartmentNetwork.h   ← Phase 9 (create)
│   └── ...
├── src/
│   ├── ThreeZoneModel.cpp
│   ├── CFDInterface.cpp
│   ├── RadiationModel.cpp     ← Create to match header
│   └── ...
└── tests/
    └── TestNumericIntegrity.cpp ← Add 9A1, 9B1, etc.

docs/
├── PHASE8_*.md               ← Reference material
└── phases/
    ├── PHASE9_STARTUP.md     ← Master roadmap
    └── PHASE9_SESSION_INIT.md ← Today's checklist
```

---

## Test Naming Convention

```
Phase 8 tests: [8A1-8A3, 8B1-8B2]  (5 total)
Phase 9 tests: [9A1-9A3, 9B1-9B3, 9C1-9C2, 9D1-9D3, 9E1-9E2]  (13 total)

Pattern: [Phase][Track][Number]
- Phase: 8 or 9
- Track: A=Radiation, B=Multi-Comp, C=CFD, D=Flame, E=ML
- Number: 1,2,3 (up to 3 per track)
```

---

## Radiation Model (Start Here)

### Header Template
```cpp
// cpp_engine/include/RadiationModel.h
namespace vfep {
class RadiationModel {
public:
    void reset();
    void calculateViewFactors();
    void updateRadiation(float T_zone, float T_walls);
    float getRadiativeHeatFlux();
private:
    // ...
};
}
```

### Test Template
```cpp
TEST(PhaseNine, RadiationViewFactors_9A1) {
    // Test view factor calculation
    EXPECT_TRUE(/* conditions */);
}
```

---

## Integration Checklist

- [ ] Feature header created with documentation
- [ ] Implementation file created and built
- [ ] First test written (before implementation)
- [ ] Code compiles without warnings
- [ ] All 62+ tests still pass
- [ ] New tests pass (2-3 new per feature)
- [ ] Comments explain the "why"
- [ ] Design doc created/updated

---

## Code Style Reminders

```cpp
// ✅ Good
float calculateViewFactor(float area1, float area2);

// ❌ Avoid
float calcVF(float a, float a2);

// ✅ Comments explain why
// Account for convective heat transfer enhancement
//  due to buoyancy (Rayleigh number > 10^7)
float convection = ...;

// ❌ Avoid
float convection = ...; // convection
```

---

## Testing Pyramid

```
          Integration Tests (ValidationSuite.exe)
              ↑          ↑          ↑
    Unit Tests (NumericIntegrity.exe)
         ↑    ↑    ↑    ↑    ↑
     Method   Method   Method
     Tests    Tests    Tests
```

**Rule**: Write unit tests first, integration second.

---

## Performance Benchmarking

```bash
# Time a single test
START time /T
.\NumericIntegrity.exe
END time /T

# Should see: ~1-2 seconds for all 62+ tests
```

**Targets per Phase 9 feature**:
- Radiation calc: <10ms per step
- Single 3-room: <2s per 60s sim
- Full system: <10s per 60s sim

---

## Common Errors & Fixes

| Error | Fix |
|-------|-----|
| `undefined reference` | Add library to CMakeLists.txt target_link_libraries() |
| `compilation warning` | Fix it before pushing (no warnings allowed) |
| Tests fail | Verify Phase 8 first: `.\NumericIntegrity.exe` |
| Build hangs | Clean rebuild: `cmake ... --clean-first` |

---

## Git Workflow

```bash
# Start feature
git checkout -b phase9-radiation

# After changes
git add .
git commit -m "Phase 9: Radiation - view factor calculation"
git push origin phase9-radiation

# Create PR when ready for review
# Title: "Phase 9: Radiation Model (9A1-9A3 tests)"
```

---

## Documentation Links

| Doc | Purpose |
|-----|---------|
| [PHASE8_API_Reference.md](docs/PHASE8_API_Reference.md) | API you're extending |
| [PHASE8_Technical_Report.md](docs/PHASE8_Technical_Report.md) | Physics background |
| [PHASE9_STARTUP.md](docs/phases/PHASE9_STARTUP.md) | Full roadmap |
| [PHASE9_SESSION_INIT.md](docs/phases/PHASE9_SESSION_INIT.md) | Today's setup |

---

## Track Priority Guide

**Recommended Start**: Radiation Model
- Self-contained
- No external dependencies
- Foundation for other features
- Medium difficulty

**Next**: Multi-Compartment
- Extends single-room to buildings
- Uses existing ThreeZoneModel
- Clear integration path

**After That**: CFD Coupling or Flame Spread
- CFD: Requires external solver
- Flame: Complex ignition logic

**Final**: Machine Learning
- Data-heavy, needs training data
- Best done after other tracks work

---

## Success This Week

| Day | Goal |
|-----|------|
| Today | ✅ All Phase 9 docs ready, skeleton files created |
| Tomorrow | ✅ RadiationModel.h header complete |
| Day 3 | ✅ First test (9A1) written |
| Day 4 | ✅ Basic implementation, test passing |
| Day 5 | ✅ Code review, documentation complete |

---

## Quick Help

**Stuck on view factors?**
→ See PHASE8_Technical_Report.md, Section 3

**Need API example?**
→ See PHASE8_API_Reference.md, Appendix A

**How to structure tests?**
→ See cpp_engine/tests/TestNumericIntegrity.cpp, lines 200-300

**Build error?**
→ Check cpp_engine/CMakeLists.txt for recent changes

---

## One-Minute Summary

🎯 **You are here**: Phase 9 launching, Phase 8 solid foundation in place

📦 **What you have**: 62 passing tests, 7 validation scenarios, clean build

🚀 **What to do**: 
1. Pick a track (Radiation recommended)
2. Create header with full documentation
3. Write test first
4. Implement code to pass test
5. Verify all 62+ tests still pass
6. Commit and repeat

✅ **Success**: Each track gets 2-3 tests + implementation + docs

**Let's build the future of fire modeling!** 🔥

---

**Created**: February 3, 2026  
**Last Updated**: Now  
**Status**: Active  
**Next**: Create RadiationModel.h skeleton
