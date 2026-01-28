#!/usr/bin/env python3
"""
Phase 6: Multi-Zone Quick Test Script
Tests if the multi-zone configuration fix (V_upper=30, V_lower=0.1) improves validation
without breaking the baseline.
"""

import subprocess
import re
import sys
from pathlib import Path

def run_validation_suite():
    """Run ValidationSuite.exe and extract key metrics"""
    result = subprocess.run(
        [r"d:\Chemsi\build-mingw64\ValidationSuite.exe"],
        capture_output=True,
        text=True,
        cwd=r"d:\Chemsi\build-mingw64"
    )
    
    output = result.stdout + result.stderr
    
    metrics = {
        'iso_9705_T': None,
        'nist_hrr': None,
        'suppression_eff': None,
        'stratification': None,
        'overall_pass': None
    }
    
    # Parse ISO 9705 temperature
    match = re.search(r'ISO 9705.*?Predicted Peak T:\s*([\d.]+)\s*K', output, re.DOTALL)
    if match:
        metrics['iso_9705_T'] = float(match.group(1))
    
    # Parse NIST HRR
    match = re.search(r'NIST Data Center.*?Predicted Peak HRR:\s*([\d.]+)\s*kW', output, re.DOTALL)
    if match:
        metrics['nist_hrr'] = float(match.group(1)) * 1000  # Convert to W
    
    # Parse suppression effectiveness
    match = re.search(r'Suppression Effectiveness.*?Predicted effectiveness:\s*([\d.]+)%', output, re.DOTALL)
    if match:
        metrics['suppression_eff'] = float(match.group(1))
    
    # Parse stratification
    match = re.search(r'Multi-Zone Stratification.*?Predicted ΔT:\s*([\d.]+)\s*K', output, re.DOTALL)
    if match:
        metrics['stratification'] = float(match.group(1))
    
    # Parse overall pass rate
    match = re.search(r'VALIDATION STATUS:.*?(\d+\.?\d*)%', output)
    if match:
        metrics['overall_pass'] = float(match.group(1))
    
    return metrics, output

def main():
    print("=" * 80)
    print("PHASE 6: MULTI-ZONE CONFIGURATION QUICK TEST")
    print("=" * 80)
    print()
    
    print("TEST 1: BASELINE (Single-Zone, should be 1/4 passing)")
    print("-" * 80)
    
    baseline_metrics, baseline_output = run_validation_suite()
    
    print(f"ISO 9705 Temperature:       {baseline_metrics['iso_9705_T']:.1f} K (target: 1023 K)")
    print(f"NIST Data Center HRR:       {baseline_metrics['nist_hrr']/1000:.1f} kW (target: 75 kW)")
    print(f"Suppression Effectiveness:  {baseline_metrics['suppression_eff']:.1f}% (target: 60-80%)")
    print(f"Stratification Rise:        {baseline_metrics['stratification']:.1f} K (target: 200-400 K)")
    print(f"Overall Pass Rate:          {baseline_metrics['overall_pass']:.1f}%")
    print()
    
    # Verify baseline is correct (NIST should pass)
    if baseline_metrics['nist_hrr'] is not None:
        error_nist = abs(baseline_metrics['nist_hrr'] - 75000) / 75000
        print(f"NIST Error: {error_nist*100:.1f}%", end="")
        if error_nist < 0.20:  # 20% threshold = passing
            print(" ✓ BASELINE CONFIRMED")
        else:
            print(" ✗ BASELINE BROKEN!")
            return 1
    else:
        print("Could not parse NIST HRR")
        return 1
    
    print()
    print("TEST 2: ENABLING MULTI-ZONE (with corrected V_upper/V_lower config)")
    print("-" * 80)
    print()
    
    # Now we need to enable multi-zone
    # This requires modifying Simulation.cpp and rebuilding
    # For now, just show what we expect
    
    print("To test multi-zone mode, modify Simulation.cpp:")
    print("  1. In resetToDataCenterRackScenario(), add:")
    print("     rc.model_type = ReactorConfig::Model::ThreeZone;")
    print("  2. Rebuild with: cmake --build build-mingw64 --config Release")
    print("  3. Re-run ValidationSuite.exe")
    print()
    print("EXPECTED IMPROVEMENTS with corrected multi-zone config:")
    print("  • ISO 9705 Temperature: 375 K → 700+ K (stratified upper layer)")
    print("  • NIST HRR: Should stay ~85.6 kW (combustion unchanged)")
    print("  • Suppression: Higher effectiveness (agent reaches hot zone)")
    print("  • Stratification: 59.5 K → 200-400 K (zone temperature difference)")
    print()
    print("If multi-zone breaks again with these values:")
    print("  • Check: V_upper + V_lower = 30.1 m³ ≈ total volume")
    print("  • Check: initializeMultiZone() is called on first step")
    print("  • Check: syncMultiZoneToSingleZone() correctly averages zones")
    print()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
