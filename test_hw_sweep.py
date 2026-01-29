#!/usr/bin/env python3
"""
Phase 6 TEST 1: h_W Sensitivity Sweep
Tests if h_W is the lever for ISO 9705 temperature control
"""

import subprocess
import re
import sys
from pathlib import Path

VFEP_ROOT = Path("d:/Chemsi")
CPP_FILE = VFEP_ROOT / "cpp_engine/src/Simulation.cpp"
BUILD_DIR = VFEP_ROOT / "build-mingw64"
VALIDATION_EXE = BUILD_DIR / "VFEP_Sim.exe"

# Test values for h_W
H_W_VALUES = [5.0, 10.0, 15.0, 20.0, 50.0]

def read_file_content(filepath):
    """Read file content"""
    return filepath.read_text()

def write_file_content(filepath, content):
    """Write file content"""
    filepath.write_text(content)

def update_hw_value(filepath, new_value):
    """Update h_W value in Simulation.cpp"""
    content = read_file_content(filepath)
    # Find and replace the h_W_m2K line
    old_line = "        rc.h_W_m2K    = 10.0;"
    new_line = f"        rc.h_W_m2K    = {new_value};"
    
    if old_line not in content:
        print(f"ERROR: Could not find h_W_m2K line in {filepath}")
        return False
    
    new_content = content.replace(old_line, new_line)
    write_file_content(filepath, new_content)
    print(f"  ✓ Updated h_W to {new_value}")
    return True

def build_project():
    """Rebuild the project"""
    print("\n  Building project...")
    result = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--config", "Release"],
        capture_output=True,
        text=True,
        cwd=str(VFEP_ROOT)
    )
    
    if result.returncode != 0:
        print(f"  ✗ Build failed!")
        print(result.stderr[-500:] if result.stderr else "")
        return False
    
    print(f"  ✓ Build successful")
    return True

def run_validation():
    """Run VFEP_Sim.exe and extract ISO 9705 temperature"""
    result = subprocess.run(
        [str(VALIDATION_EXE)],
        capture_output=True,
        text=True,
        timeout=60,
        cwd=str(BUILD_DIR)
    )
    
    output = result.stdout + result.stderr
    
    # Extract ISO 9705 peak temperature
    iso_match = re.search(r"ISO 9705.*?Predicted Peak T:\s*([\d.]+)\s*K", output, re.DOTALL)
    if iso_match:
        return float(iso_match.group(1))
    
    # Try alternative pattern
    iso_match = re.search(r"Peak T.*?:\s*([\d.]+)\s*K", output)
    if iso_match:
        return float(iso_match.group(1))
    
    print(f"  ✗ Could not parse ISO 9705 temperature from output")
    return None

def main():
    print("=" * 80)
    print("PHASE 6 TEST 1: h_W SENSITIVITY SWEEP")
    print("=" * 80)
    print("\nObjective: Determine if h_W controls ISO 9705 peak temperature")
    print(f"Target ISO 9705 temperature: 1023 K")
    print(f"Current ISO 9705 temperature: 416 K (59% error)")
    print(f"\nTesting h_W values: {H_W_VALUES}")
    print("\n" + "=" * 80)
    
    # Save original content
    original_content = read_file_content(CPP_FILE)
    results = []
    
    try:
        for h_w in H_W_VALUES:
            print(f"\n[TEST] h_W = {h_w} W/m²K")
            print("  " + "-" * 76)
            
            # Update h_W value
            if not update_hw_value(CPP_FILE, h_w):
                print("  ✗ Failed to update h_W")
                continue
            
            # Build
            if not build_project():
                print("  ✗ Build failed, skipping")
                continue
            
            # Run validation
            print("  Running validation...")
            iso_temp = run_validation()
            
            if iso_temp is not None:
                error_pct = abs(iso_temp - 1023.0) / 1023.0 * 100
                results.append((h_w, iso_temp, error_pct))
                status = "✓ PASS" if iso_temp >= 900 else ("⚠ CLOSE" if iso_temp >= 700 else "✗ FAIL")
                print(f"  ISO 9705 Peak T: {iso_temp:.1f} K (error: {error_pct:.1f}%) {status}")
            else:
                print(f"  ✗ Failed to get temperature reading")
    
    finally:
        # Restore original content
        print(f"\n[CLEANUP] Restoring original h_W = 10.0")
        write_file_content(CPP_FILE, original_content)
        print("  ✓ Original file restored")
    
    # Summary
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    
    if results:
        print(f"\n{'h_W (W/m²K)':<15} {'Peak T (K)':<15} {'Error %':<15} {'Status'}")
        print("-" * 60)
        for h_w, temp, err in results:
            status = "✓ PASS" if temp >= 900 else ("⚠ CLOSE" if temp >= 700 else "✗ FAIL")
            print(f"{h_w:<15.1f} {temp:<15.1f} {err:<15.1f} {status}")
        
        # Analysis
        print("\n" + "=" * 80)
        print("ANALYSIS")
        print("=" * 80)
        
        # Check if h_W is a lever
        temps = [r[1] for r in results]
        temp_range = max(temps) - min(temps)
        
        print(f"\nTemperature range: {min(temps):.1f} - {max(temps):.1f} K (ΔT = {temp_range:.1f} K)")
        
        if temp_range > 100:
            print("\n✅ h_W IS A LEVER (significant temperature response)")
            print("   → Sensitivity: ~{:.1f} K per W/m²K change".format(temp_range / (max(H_W_VALUES) - min(H_W_VALUES))))
            
            # Find optimal h_W
            optimal = min(results, key=lambda x: abs(x[1] - 1023.0))
            print(f"\n📌 OPTIMAL h_W: {optimal[0]:.1f} W/m²K")
            print(f"   → Predicted T: {optimal[1]:.1f} K")
            print(f"   → Error: {optimal[2]:.1f}%")
            
            if optimal[2] < 15:
                print(f"\n🎯 RECOMMENDATION: Use h_W = {optimal[0]:.1f} for ISO 9705")
        else:
            print("\n❌ h_W is NOT a significant lever (ΔT < 100 K)")
            print("   → Other factors control ISO 9705 temperature (ACH, volume, etc.)")
    else:
        print("No results collected - check build/validation output above")
    
    print("\n" + "=" * 80)

if __name__ == "__main__":
    sys.exit(main())
