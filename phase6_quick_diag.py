#!/usr/bin/env python3
"""Quick Phase 6 diagnostic: Parse VFEP_Sim console output"""

import re
import subprocess

# Run VFEP_Sim and capture output
result = subprocess.run(
    r"d:\Chemsi\build-mingw64\VFEP_Sim.exe",
    capture_output=True,
    text=True,
    timeout=60
)

output = result.stdout + result.stderr

# Extract key metrics
print("\n" + "="*80)
print("PHASE 6 QUICK DIAGNOSTIC - VFEP_Sim Output Analysis")
print("="*80 + "\n")

# Search for ISO 9705 results
iso_section = output[output.find("ISO 9705"):output.find("ISO 9705")+2000] if "ISO 9705" in output else ""
if iso_section:
    print("ISO 9705 SECTION:")
    iso_lines = iso_section.split('\n')[:15]
    for line in iso_lines:
        if any(k in line for k in ["Peak T", "Literature", "Error", "Predicted", "Within"]):
            print(f"  {line.strip()}")

# Search for NIST results
nist_section = output[output.find("NIST"):output.find("NIST")+2000] if "NIST" in output else ""
if nist_section:
    print("\nNIST DATA CENTER SECTION:")
    nist_lines = nist_section.split('\n')[:20]
    for line in nist_lines:
        if any(k in line for k in ["Peak HRR", "Literature", "Error", "Predicted", "Within", "|", "Time to Peak"]):
            print(f"  {line.strip()}")

# Search for suppression results
supp_section = output[output.find("Suppression"):output.find("Suppression")+1500] if "Suppression" in output else ""
if supp_section:
    print("\nSUPPRESSION SECTION:")
    supp_lines = supp_section.split('\n')[:12]
    for line in supp_lines:
        if any(k in line for k in ["Reduction", "Literature", "Error", "Predicted", "Within", "Temperature"]):
            print(f"  {line.strip()}")

print("\n" + "="*80)
