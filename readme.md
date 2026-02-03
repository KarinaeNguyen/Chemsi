# CHEMSI / VFEP Simulation

**Ventilated Fire Evolution Predictor** - Advanced fire dynamics simulation with multi-zone physics

**Current Status**: 🎉 **Phase 8 COMPLETE - Production Ready** 🎉

## 🚀 Quick Navigation

### 📋 Start Here
- **New to project?** → [docs/VFEP_SIMULATION_OVERVIEW.md](docs/VFEP_SIMULATION_OVERVIEW.md)
- **Quick start?** → [QUICKSTART.md](QUICKSTART.md)
- **Phase 8 complete?** → [docs/phases/PHASE8_COMPLETION.md](docs/phases/PHASE8_COMPLETION.md)
- **Phase 9 roadmap?** → [docs/phases/PHASE9_STARTUP.md](docs/phases/PHASE9_STARTUP.md)

### 🎯 Current Status (February 3, 2026)
- **Validation Rate**: 100% (7/7 scenarios, mean error 9.02% ✅)
- **Numeric Tests**: 62/62 passing (100% ✅)
- **Build Status**: ✅ Clean, zero warnings, zero errors
- **Performance**: ~0.3-0.5s per 60s simulation (exceeds <2s target by 4-6×)
- **Documentation**: 4 comprehensive guides complete ✅

### 📊 Phase 8 Complete (All Objectives Achieved)
- ✅ **Three-Zone Model** - Stratified fire physics operational
- ✅ **CFD Interface** - Comparison framework with VTK import/export
- ✅ **5 New Tests** - 8A1-8A3 (three-zone), 8B1-8B2 (CFD)
- ✅ **3 New Scenarios** - Ship, Tunnel, Industrial fires validated
- ✅ **Complete Documentation** - API, User Guide, Technical Report, Scenarios Catalog

### 📊 All Validation Results
| Scenario | Predicted | Target | Error | Status |
|----------|-----------|--------|-------|--------|
| ISO 9705 | 981K | 1023K ±50K | 4.11% | ✅ PASS |
| NIST Data Center | 71.4 kW | 75 kW | 4.85% | ✅ PASS |
| Suppression | 79.77% | 60-80% | 13.95% | ✅ PASS |
| Stratification | 272K ΔT | 200-400K | 9.26% | ✅ PASS |
| Ship Fire | 967.8K | 800-1000K | 7.54% | ✅ PASS |
| Tunnel Fire | 1070 kW | 500-2000 kW | 14.38% | ✅ PASS |
| Industrial | 500.1K | 500-650K | 9.06% | ✅ PASS |

## 📁 Repository Structure

```
VFEP/
├── cpp_engine/          # C++ simulation engine
│   ├── include/        # Public API headers
│   ├── src/            # Core simulation code
│   ├── vis/            # Visualization (ImGui + OpenGL)
│   └── tools/          # Utilities (SweepTool, etc.)
├── docs/               # Documentation
│   ├── phases/         # Phase development logs
│   ├── guides/         # User guides and tutorials
│   └── *.md            # Technical documentation
├── scripts/            # Python test scripts
│   └── testing/        # Test automation
├── test_results/       # Output files and test results
├── assets/             # Geometry files (STL) and configs
├── proto/              # Protocol buffer definitions
├── python_interface/   # Python bindings (if available)
└── Testing/            # Test data and fixtures
```

## 🏗️ Building & Running

```bash
# Configure (MinGW64 on Windows)
cmake -G "MinGW Makefiles" -B build-mingw64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build-mingw64 --config Release

# Run tests
cd build-mingw64
.\NumericIntegrity.exe    # 57/57 tests
.\ValidationSuite.exe      # 7/7 scenarios
```

See [QUICKSTART.md](QUICKSTART.md) for detailed build instructions.

## 📚 Documentation

- **[docs/](docs/)** - Complete documentation index
- **[docs/phases/](docs/phases/)** - Phase-by-phase development logs
- **[docs/guides/](docs/guides/)** - User guides and tutorials

## 🔬 Phase History
- **Phase 1-2**: Foundation (combustion, thermodynamics) ✅
- **Phase 3-4**: Integration (suppression, ventilation) ✅
- **Phase 5**: Initial calibration (NIST baseline) ✅
- **Phase 6**: Multi-scenario validation ✅
- **Phase 7**: Sensitivity analysis & UQ ✅
- **Phase 8**: Three-zone model & CFD ✅ **COMPLETE - Production Ready**
- **Phase 9**: Advanced physics & multi-scale (radiation, multi-compartment, ML) - Ready to start

## 🤝 Contributing

This is a research project in active development. See phase documentation in [docs/phases/](docs/phases/) for current objectives and roadmap.
