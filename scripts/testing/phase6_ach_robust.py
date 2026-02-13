#!/usr/bin/env python3
"""
Phase 6: ACH (Ventilation) Sensitivity - Robust Version
"""

import subprocess
import re
import sys
from pathlib import Path

VFEP_ROOT = Path("d:/Chemsi")
CPP_FILE = VFEP_ROOT / "cpp_engine/src/Simulation.cpp"
BUILD_DIR = VFEP_ROOT / "build-mingw64"

def read_file(filepath):
    return filepath.read_text()

def write_file(filepath, content):
    filepath.write_text(content)

def update_ach(new_value):
    """Update ACH value in Simulation.cpp - robust version"""
    content = read_file(CPP_FILE)
    # Match the pattern with variable spacing and any numeric value
    pattern = r"vc\.ACH\s*=\s*[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?;"
    replacement = f"vc.ACH        = {new_value};"

    if not re.search(pattern, content):
        print(f"    ERROR: Could not find ACH line in {CPP_FILE}")
        return False

    new_content = re.sub(pattern, replacement, content, count=1)
    write_file(CPP_FILE, new_content)
    return True

def rebuild_core():
    """Rebuild only core and VFEP_Sim"""
    result = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--config", "Release", 
         "--target", "ValidationSuite", "-j", "4"],
        capture_output=True,
        text=True,
        cwd=str(VFEP_ROOT)
    )
    return result.returncode == 0

def run_simulation() -> dict:
    """Run VFEP_Sim and extract metrics"""
    result = subprocess.run(
        [str(BUILD_DIR / "ValidationSuite.exe")],
        capture_output=True,
        text=True,
        timeout=120,
        cwd=str(BUILD_DIR)
    )

    output = result.stdout + result.stderr
    metrics = {
        'iso_temp': None,
        'success': ('VFEP RIGOROUS VALIDATION SUITE' in output) or ('Predicted Peak T' in output)
    }

    # Parse ISO 9705 peak temperature from ValidationSuite output
    match = re.search(r"Predicted Peak T:\s*([\d.]+)\s*K", output)
    if match:
        try:
            metrics['iso_temp'] = float(match.group(1))
        except:
            pass

    return metrics

def main():
    print("=" * 80)
    print("PHASE 6: ACH SENSITIVITY TEST (ROBUST)")
    print("=" * 80)
    print("\nTest: Does reducing ventilation increase peak temperature?")
    print("Target: 1023 K | Baseline: ~416 K\n")
    
    ach_values = [0.5, 1.0, 3.0, 5.0]
    original_content = read_file(CPP_FILE)
    results = []
    
    try:
        for i, ach in enumerate(ach_values, 1):
            print(f"[{i}/{len(ach_values)}] ACH = {ach}", end=" ... ")
            
            if not update_ach(ach):
                print("UPDATE FAILED")
                continue
            
            if not rebuild_core():
                print("BUILD FAILED")
                continue
            
            metrics = run_simulation()
            if metrics['success'] and metrics['iso_temp']:
                temp_k = metrics['iso_temp']
                error = abs(temp_k - 1023) / 1023 * 100
                results.append((ach, temp_k, error))
                status = "✓PASS" if error < 20 else ("⚠" if error < 40 else "✗")
                print(f"T={temp_k:.0f}K (err={error:.0f}%) {status}")
            else:
                print("PARSE FAILED")
    
    finally:
        write_file(CPP_FILE, original_content)
        print(f"\nRestored original code")
    
    # Summary
    print("\n" + "=" * 80)
    if results:
        print(f"\n{'ACH':<10} {'T (K)':<12} {'Error %':<12} {'Note'}")
        print("-" * 50)
        for ach, temp, err in results:
            note = "✓ BEST" if min(results, key=lambda x: x[2])[2] == err else ""
            print(f"{ach:<10.1f} {temp:<12.0f} {err:<12.1f} {note}")
        
        # Analysis
        if len(results) >= 2:
            temps = [r[1] for r in results]
            t_range = max(temps) - min(temps)
            print(f"\nTemperature range: {t_range:.0f} K")
            
            if t_range > 100:
                print("✅ ACH affects temperature significantly")
                best = min(results, key=lambda x: x[2])
                print(f"   → Best ACH: {best[0]} → {best[1]:.0f}K (error {best[2]:.0f}%)")
            else:
                print("❌ ACH has minimal effect")
    else:
        print("No valid results")

if __name__ == "__main__":
    main()
