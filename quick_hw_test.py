#!/usr/bin/env python3
"""Quick h_W test results"""
import subprocess
import re

result = subprocess.run(
    [r"d:\Chemsi\build-mingw64\VFEP_Sim.exe"],
    capture_output=True,
    text=True,
    timeout=120,
    cwd=r"d:\Chemsi\build-mingw64"
)

output = result.stdout + result.stderr

# Look for validation results in output
if "ISO 9705" in output:
    print("h_W=5.0 Test Results:")
    print("=" * 60)
    # Extract validation summary
    lines = output.split('\n')
    for i, line in enumerate(lines):
        if 'ISO 9705' in line or 'NIST' in line or 'Suppression' in line or 'stratification' in line.lower():
            print(line)
else:
    print("No validation output found")
    print("Last 30 lines of output:")
    print('\n'.join(output.split('\n')[-30:]))
