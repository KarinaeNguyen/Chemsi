#!/usr/bin/env python3
"""
Phase 6: Systematic Investigation
Test each hypothesis and log results
"""

import subprocess
import json
import re
from pathlib import Path
from dataclasses import dataclass
from typing import Dict, List

@dataclass
class ValidationResult:
    iso_t_K: float
    iso_error: float
    nist_hrr_kW: float
    nist_error: float
    supp_reduction: float
    supp_error: float
    strat_K: float
    strat_error: float
    passing: int
    
    def display(self):
        print(f"\n{'='*70}")
        print(f"ISO 9705:     {self.iso_t_K:6.1f} K   error: {self.iso_error*100:5.1f}%")
        print(f"NIST HRR:     {self.nist_hrr_kW:6.1f} kW  error: {self.nist_error*100:5.1f}% {'✓ PASS' if self.nist_error < 0.15 else '✗ FAIL'}")
        print(f"Suppression:  {self.supp_reduction:6.1f} %  error: {self.supp_error*100:5.1f}%")
        print(f"Stratification:{self.strat_K:6.1f} K   error: {self.strat_error*100:5.1f}%")
        print(f"Passing:      {self.passing}/4 scenarios")
        print(f"{'='*70}\n")

def parse_validation_output(output: str) -> ValidationResult:
    """Parse ValidationSuite.exe output"""
    
    # Extract values using regex
    iso_t = re.search(r"Predicted Peak T: ([\d.]+) K", output)
    iso_err = re.search(r"Relative Error: ([\d.]+)%.*?\n.*Within Uncertainty: (YES|NO)", output)
    
    nist_hrr = re.search(r"Predicted Peak HRR: ([\d.]+) kW", output)
    nist_err = re.search(r"(?:Peak HRR.*?\n.*?)Relative Error: ([\d.]+)%", output)
    
    supp = re.search(r"Predicted HRR Reduction: ([\d.]+)%", output)
    supp_err = re.search(r"(?:Reduction.*?\n.*?)Relative Error: ([\d.]+)%", output)
    
    strat = re.search(r"Predicted Δ?T .*?\): ([\d.]+) K", output)
    strat_err = re.search(r"(?:Dynamics.*?\n.*?)Relative Error: ([\d.]+)%", output)
    
    passing = 1 if "1/4" in output else (2 if "2/4" in output else (3 if "3/4" in output else 0))
    
    return ValidationResult(
        iso_t_K=float(iso_t.group(1)) if iso_t else 0,
        iso_error=float(iso_err.group(1))/100 if iso_err else 0,
        nist_hrr_kW=float(nist_hrr.group(1)) if nist_hrr else 0,
        nist_error=float(nist_err.group(1))/100 if nist_err else 0,
        supp_reduction=float(supp.group(1)) if supp else 0,
        supp_error=float(supp_err.group(1))/100 if supp_err else 0,
        strat_K=float(strat.group(1)) if strat else 0,
        strat_error=float(strat_err.group(1))/100 if strat_err else 0,
        passing=passing
    )

def run_validation() -> ValidationResult:
    """Run ValidationSuite.exe and parse output"""
    result = subprocess.run(
        r"d:\Chemsi\build-mingw64\ValidationSuite.exe",
        capture_output=True,
        text=True,
        timeout=120,
        cwd=r"d:\Chemsi\build-mingw64"
    )
    return parse_validation_output(result.stdout + result.stderr)

print("\n" + "="*70)
print("PHASE 6: SYSTEMATIC INVESTIGATION")
print("="*70)

print("\n[1] BASELINE (Phase 5 values)")
baseline = run_validation()
baseline.display()

print(f"\nBASELINE INTERPRETATION:")
print(f"  • NIST passing at 14.1% (combustion model validated ✓)")
print(f"  • ISO 9705 at 375K (not responding to h_W/ACH as expected)")
print(f"  • Hypothesis: Single-zone averaging temperature, NOT stratification")
print(f"  • Next: Need to check if multi-zone model is active")

print("\nINVESTIGATION PLAN:")
print("  1. Check Reactor multi-zone configuration")
print("  2. Enable multi-zone if needed")
print("  3. Compare single-zone vs multi-zone peaks")
print("  4. Adjust volume or area ratios")
print("  5. Run full h_W/ACH sensitivity matrix")

print("\nNote: ValidationSuite is the source of truth for Phase 6")
print("      (not ValidationResults.csv)")
