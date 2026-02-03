# VFEP Documentation

This directory contains comprehensive documentation for the Ventilated Fire Evolution Predictor (VFEP) project.

## 📁 Directory Structure

```
docs/
├── phases/              # Phase-by-phase development documentation
│   ├── PHASE5_*.md     # Phase 5: Multi-zone baseline
│   ├── PHASE6_*.md     # Phase 6: Advanced scenarios & calibration
│   ├── PHASE7_*.md     # Phase 7: Sensitivity analysis & UQ
│   └── PHASE8_*.md     # Phase 8: Three-zone model & CFD interface
├── guides/              # User guides and tutorials
│   ├── STL_QUICK_REF.md
│   ├── STL_IMPORT_GUIDE.md
│   └── PHASE7_LAUNCH_GUIDE.md
├── CODE_QUALITY_VERIFICATION.md
├── CODE_REVIEW_FULL_SCAN.md
├── IMPROVEMENTS_APPLIED.md
├── NIST_TEST_VERIFICATION_REPORT.md
├── PROGRESS.md
├── VFEP_IMPROVEMENTS_FINAL.md
└── VFEP_SIMULATION_OVERVIEW.md
```

## 📚 Key Documents

### Getting Started
- **[QUICKSTART.md](../QUICKSTART.md)** - Quick start guide for building and running VFEP
- **[README.md](../readme.md)** - Main project README

### Phase Documentation
- **[Phase 8 Startup](phases/PHASE8_STARTUP.md)** - Current phase objectives and roadmap
- **[Phase 8 Quick Reference](phases/PHASE8_QUICK_REF.md)** - Quick reference for Phase 8
- **[Phase 8 Week 1 Complete](phases/PHASE8_WEEK1_COMPLETE.md)** - Week 1 completion report

### Technical Documentation
- **[VFEP Simulation Overview](VFEP_SIMULATION_OVERVIEW.md)** - Comprehensive overview of simulation physics
- **[NIST Test Verification](NIST_TEST_VERIFICATION_REPORT.md)** - Validation against NIST benchmarks
- **[Code Quality Verification](CODE_QUALITY_VERIFICATION.md)** - Code quality metrics and standards

### User Guides
- **[STL Import Guide](guides/STL_IMPORT_GUIDE.md)** - Guide for importing STL geometry
- **[STL Quick Reference](guides/STL_QUICK_REF.md)** - Quick reference for STL features

## 🎯 Current Status (Phase 8)

**Tests**: 57/57 passing ✅  
**Validation Scenarios**: 7/7 passing ✅  
**Phase Progress**: Week 1 complete (25%)

### Next Steps
- Implement Three-Zone Model (Week 2-3)
- CFD Interface implementation (Week 3-4)
- Complete documentation suite (Week 4)

## 📊 Validation Results

All 7 fire scenarios validated within literature uncertainty bounds:
- ISO 9705 Room Corner Test: 4.11% error ✅
- NIST Data Center Fire: 4.85% error ✅
- Suppression Effectiveness: 13.95% error ✅
- Temperature Stratification: 9.26% error ✅
- Ship Fire (Confined): 7.54% error ✅
- Tunnel Fire (Flow-Driven): 14.38% error ✅
- Industrial Fire (Warehouse): 9.06% error ✅

## 🔧 For Developers

See [phases/](phases/) for detailed development logs and session notes for each phase of the project.
