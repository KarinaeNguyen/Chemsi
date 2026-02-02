# CHEMSI / VFEP Simulation

**Current Project Phase**: Phase 6 - ✅ **COMPLETE (4/4 Scenarios Passing)**

## Quick Navigation

### 📋 Start Here
- **New to project?** → [VFEP_SIMULATION_OVERVIEW.md](VFEP_SIMULATION_OVERVIEW.md)
- **Run quickly?** → [QUICKSTART.md](QUICKSTART.md)
- **Phase 6 complete?** → [PHASE6_FINAL_COMPLETION.md](PHASE6_FINAL_COMPLETION.md)

### 🎯 Current Status
- **Validation Rate**: 100% (4/4 scenarios passing)
- **Numeric Tests**: 51/51 passing
- **Build Status**: ✅ Clean, no warnings
- **Ready for**: Phase 7 - Advanced validation

### 📊 Phase 6 Results
| Scenario | Predicted | Target | Error | Status |
|----------|-----------|--------|-------|--------|
| ISO 9705 | 981K | 1023K ±50K | 4.11% | ✅ PASS |
| NIST Data Center | 71.4 kW | 75 kW | 4.85% | ✅ PASS |
| Suppression | 79.77% | 60-80% | 13.95% | ✅ PASS |
| Stratification | 272K ΔT | 200-400K | 9.26% | ✅ PASS |

## Key Paths
- Core simulation: [cpp_engine/src](cpp_engine/src)
- Public API: [cpp_engine/include](cpp_engine/include)
- Visualization: [cpp_engine/vis/main_vis.cpp](cpp_engine/vis/main_vis.cpp)
- Build scripts: [launch_all.sh](launch_all.sh) and [cpp_engine/launch_all.bat](cpp_engine/launch_all.bat)
- Protos: [proto](proto)

## Phase History
- **Phase 1-2**: Foundation (combustion, thermodynamics)
- **Phase 3-4**: Integration (suppression, ventilation)
- **Phase 5**: Initial calibration (NIST baseline)
- **Phase 6**: Multi-scenario validation (✅ COMPLETE)
- **Phase 7**: Advanced validation (planned)

Historical logs and verification notes are archived to keep documentation concise.
