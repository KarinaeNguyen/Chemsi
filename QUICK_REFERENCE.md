# Repository Quick Reference

**Last Updated**: February 12, 2026  
**Status**: Professionally cleaned and organized

---

## 🚀 Quick Commands

```powershell
# Fast incremental build (5-8x faster)
.\fast_build.ps1

# Run all tests (C++ + Python)
.\fast_test.ps1

# Run validation workflow
cd python_interface
python run_validation.py

# Launch visualizer
.\run_vfep.bat
# or
build-mingw64\VFEP.exe
```

---

## 📂 Directory Structure

| Directory | Purpose | Key Files |
|-----------|---------|-----------|
| **archive/** | Historical files | old_test_outputs/, old_builds/ |
| **assets/** | Runtime assets | geometry/, imgui.ini |
| **build-mingw64/** | Active build | *.exe, tests/ |
| **cpp_engine/** | C++ source | include/, src/, tests/ |
| **data/** | Validation data | scenario_library.csv, field_targets.csv |
| **docs/** | Documentation | phases/, guides/ |
| **python_interface/** | Python API | validation_dataset.py, run_validation.py |
| **scripts/** | Utilities | testing/ |

---

## 📖 Documentation Index

### Getting Started
- [readme.md](readme.md) - Main project README
- [QUICKSTART.md](QUICKSTART.md) - Quick start guide

### Current Phase (Phase 10)
- [Phase 10 Quick Reference](docs/phases/PHASE10_QUICK_REF.md) ⭐
- [Operator Guide](docs/phases/PHASE10_OPERATOR_GUIDE.md)
- [Deployment Checklist](docs/phases/PHASE10_DEPLOYMENT_CHECKLIST.md)
- [Stakeholder Summary](docs/phases/PHASE10_STAKEHOLDER_SUMMARY.md)
- [Data Schema](docs/phases/PHASE10_DATA_SCHEMA.md)
- [Completion Report](docs/phases/PHASE10_COMPLETION.md)

### Technical Documentation
- [Phase 8 Technical Report](docs/PHASE8_Technical_Report.md)
- [Phase 8 User Guide](docs/PHASE8_User_Guide.md)  
- [Phase 9 Quick Reference](docs/PHASE9_QUICK_REF.md)

### Repository Organization
- [REPOSITORY_ORGANIZATION.md](REPOSITORY_ORGANIZATION.md) - Full directory structure
- [CLEANUP_SUMMARY.md](CLEANUP_SUMMARY.md) - Recent cleanup actions

---

## 🧪 Testing

### Python Tests (11 total)
```powershell
cd python_interface
python -m unittest discover -p "test_*.py" -v
```

**Test Coverage**:
- Phase 9E: Surrogate model (2 tests)
- Phase 10: Validation infrastructure (9 tests)

### C++ Tests (75 total)
```powershell
build-mingw64\tests\NumericIntegrity.exe
```

**Test Coverage**:
- Phase 8: Zone models, CFD interface
- Phase 9: Radiation, multi-compartment, CFD coupling, flame spread

---

## 🎯 Key Entry Points

### For Users
1. Start: [QUICKSTART.md](QUICKSTART.md)
2. Run: `.\run_vfep.bat`
3. Guide: [docs/phases/PHASE10_OPERATOR_GUIDE.md](docs/phases/PHASE10_OPERATOR_GUIDE.md)

### For Developers
1. Build: `.\fast_build.ps1`
2. Test: `.\fast_test.ps1`
3. Code: [cpp_engine/](cpp_engine/)

### For Validators
1. Run: `python python_interface\run_validation.py`
2. Scenarios: [data/scenario_library.csv](data/scenario_library.csv)
3. Report: [python_interface/validation_report.csv](python_interface/validation_report.csv)

---

## 🔧 Maintenance

### Monthly Cleanup
```powershell
# Remove old test outputs (if any accumulate)
Move-Item cpp_engine\*.txt archive\old_test_outputs\ -ErrorAction SilentlyContinue

# Clean build and rebuild
Remove-Item build-mingw64 -Recurse -Force
.\fast_build.ps1
```

### Validation Refresh
```powershell
cd python_interface
python run_validation.py
# Review: validation_report.csv
```

---

## 🏗️ Build System

**Active Build**: build-mingw64/
- **Compiler**: MinGW GCC 15.2.0
- **Generator**: Makefile (was Ninja, changed to Makefile for compatibility)
- **Optimizations**: Parallel builds (8 cores), Precompiled headers, LTO

**Performance**: ~5-30 seconds for full build with 8 cores

---

## 📊 Current Status

| Component | Status | Tests |
|-----------|--------|-------|
| **C++ Engine** | ✅ Phase 9 Complete | 75/75 passing |
| **Python API** | ✅ Phase 10 Complete | 11/11 passing |
| **Validation** | ✅ Infrastructure Ready | 15 scenarios |
| **Documentation** | ✅ Complete | 5 Phase 10 docs |
| **Build System** | ✅ Optimized | 5-8x faster |

---

## 🆘 Support

- **Build Issues**: Check [cpp_engine/CMakeLists.txt](cpp_engine/CMakeLists.txt)
- **Test Failures**: See [test_results/](test_results/)
- **Documentation**: [docs/README.md](docs/README.md)
- **Validation**: [docs/phases/PHASE10_OPERATOR_GUIDE.md](docs/phases/PHASE10_OPERATOR_GUIDE.md)

---

**Version**: Phase 10 (February 2026)  
**Last Cleanup**: February 12, 2026  
**Next Phase**: Integration & Calibration (Phase 11)
