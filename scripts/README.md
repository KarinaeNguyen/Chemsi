# VFEP Scripts

This directory contains utility scripts and test automation for the VFEP project.

## 📁 Directory Structure

```
scripts/
└── testing/           # Python test scripts and validation tools
    ├── phase6_*.py    # Phase 6 diagnostic and test scripts
    ├── create_test_stl.py
    ├── demo_verification.py
    └── test_hw_sweep.py
```

## 🧪 Testing Scripts

### Phase 6 Test Suite
- **phase6_ach_test.py** - ACH (Air Changes per Hour) parameter tests
- **phase6_ach_robust.py** - Robust ACH validation
- **phase6_diagnostic.py** - General diagnostic tools
- **phase6_geometry_test.py** - Geometry validation tests
- **phase6_hw_test.py** - Hardware/wall heat transfer tests
- **phase6_investigation.py** - Detailed investigation utilities
- **phase6_multizone_test.py** - Multi-zone model tests
- **phase6_quick_diag.py** - Quick diagnostic utilities

### Utility Scripts
- **create_test_stl.py** - Generate test STL geometry files
- **demo_verification.py** - Demonstration and verification script
- **test_hw_sweep.py** - Wall heat transfer parameter sweep

## 🚀 Usage

Most test scripts can be run directly with Python:

```bash
# Activate Python environment (if using venv)
.\.venv\Scripts\Activate.ps1

# Run a test script
python scripts\testing\phase6_diagnostic.py
```

## 📊 Test Results

Test results and outputs are stored in the `test_results/` directory at the project root.

## 🔧 Requirements

- Python 3.8+
- Dependencies as specified in project requirements
- VFEP C++ engine must be built before running integration tests
