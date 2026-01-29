#!/usr/bin/env python3
"""
Phase 6: Harmonious Parameter Calibration
Efficiently tests parameter values without visualization build overhead
"""

import subprocess
import re
import sys
from pathlib import Path
from typing import List, Tuple

VFEP_ROOT = Path("d:/Chemsi")
CPP_FILE = VFEP_ROOT / "cpp_engine/src/Simulation.cpp"
BUILD_DIR = VFEP_ROOT / "build-mingw64"

def read_file(filepath):
    return filepath.read_text()

def write_file(filepath, content):
    filepath.write_text(content)

def update_h_w(new_value):
    """Update h_W value in Simulation.cpp"""
    content = read_file(CPP_FILE)
    old_line = "        rc.h_W_m2K    = 10.0;"
    new_line = f"        rc.h_W_m2K    = {new_value};"
    
    if old_line not in content:
        return False
    
    write_file(CPP_FILE, content.replace(old_line, new_line))
    return True

def rebuild_core():
    """Rebuild only core and VFEP_Sim (skip visualization)"""
    result = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--config", "Release", 
         "--target", "VFEP_Sim", "-j", "4"],
        capture_output=True,
        text=True,
        cwd=str(VFEP_ROOT)
    )
    return result.returncode == 0

def run_simulation() -> dict:
    """Run VFEP_Sim and extract key metrics"""
    result = subprocess.run(
        [str(BUILD_DIR / "VFEP_Sim.exe")],
        capture_output=True,
        text=True,
        timeout=120,
        cwd=str(BUILD_DIR)
    )
    
    output = result.stdout + result.stderr
    metrics = {
        'iso_temp': None,
        'nist_hrr': None,
        'success': False
    }
    
    # Parse ISO 9705 temperature from console output (last peak T value before "Simulation ended")
    temps = re.findall(r"\| T=\s+([\d.e+-]+)\s+C", output)
    if temps:
        try:
            # Get the maximum temperature reached
            max_temp_c = max([float(t) for t in temps if float(t) < 1000])  # Filter out outliers
            metrics['iso_temp'] = max_temp_c + 273.15  # Convert to K
        except:
            pass
    
    # Try to find NIST HRR (if validation output present)
    hrr_match = re.search(r"HRR=\s+([\d.e+-]+)\s+kW", output)
    if hrr_match:
        metrics['nist_hrr'] = float(hrr_match.group(1))
    
    metrics['success'] = 'Simulation ended' in output
    return metrics

def main():
    print("=" * 80)
    print("PHASE 6: HARMONIOUS h_W SENSITIVITY TEST")
    print("=" * 80)
    print("\n✓ Building ONLY core + VFEP_Sim (skipping visualization)")
    print("✓ Testing h_W impact on ISO 9705 peak temperature")
    print("✓ Target: 1023 K | Current: ~416 K\n")
    
    # Test values
    h_w_values = [5.0, 10.0, 15.0, 20.0]
    original_content = read_file(CPP_FILE)
    results: List[Tuple[float, float]] = []
    
    try:
        for i, h_w in enumerate(h_w_values, 1):
            print(f"[{i}/{len(h_w_values)}] Testing h_W = {h_w} W/m²K")
            print("  " + "-" * 76)
            
            # Update code
            if not update_h_w(h_w):
                print("  ✗ Failed to update h_W, skipping")
                continue
            
            # Rebuild
            print("  Building...", end=" ", flush=True)
            if not rebuild_core():
                print("✗ FAILED")
                print("  Skipping (build error)")
                continue
            print("✓")
            
            # Run
            print("  Running...", end=" ", flush=True)
            metrics = run_simulation()
            print("✓")
            
            if metrics['success'] and metrics['iso_temp']:
                temp_k = metrics['iso_temp']
                error_pct = abs(temp_k - 1023.0) / 1023.0 * 100
                results.append((h_w, temp_k))
                
                status = "✓ PASS" if temp_k >= 900 else ("⚠ CLOSE" if temp_k >= 700 else "✗ FAIL")
                print(f"  Peak T: {temp_k:.1f} K | Error: {error_pct:.1f}% {status}")
            else:
                print("  ✗ Failed to parse results")
    
    finally:
        # Restore original
        print(f"\n[CLEANUP] Restoring h_W = 10.0")
        write_file(CPP_FILE, original_content)
    
    # Summary
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    
    if results:
        print(f"\n{'h_W (W/m²K)':<15} {'Peak T (K)':<15} {'Error %':<15} {'Status'}")
        print("-" * 60)
        for h_w, temp in results:
            error = abs(temp - 1023.0) / 1023.0 * 100
            status = "✓ PASS" if temp >= 900 else ("⚠ CLOSE" if temp >= 700 else "✗ FAIL")
            print(f"{h_w:<15.1f} {temp:<15.1f} {error:<15.1f} {status}")
        
        # Analysis
        print("\n" + "=" * 80)
        print("ANALYSIS")
        print("=" * 80)
        
        temps_only = [r[1] for r in results]
        temp_range = max(temps_only) - min(temps_only)
        
        print(f"\nTemperature range: {min(temps_only):.1f} - {max(temps_only):.1f} K (ΔT = {temp_range:.1f} K)")
        
        if temp_range > 50:
            print("\n✅ h_W IS A CONTROL LEVER")
            print(f"   Sensitivity: {temp_range / (max(h_w_values) - min(h_w_values)):.1f} K per W/m²K")
            
            # Find optimal
            optimal = min(results, key=lambda x: abs(x[1] - 1023.0))
            print(f"\n📌 RECOMMENDED: h_W = {optimal[0]:.1f} W/m²K")
            print(f"   → Predicted: {optimal[1]:.1f} K")
            print(f"   → Error: {abs(optimal[1] - 1023.0) / 1023.0 * 100:.1f}%")
            
            if abs(optimal[1] - 1023.0) / 1023.0 < 0.15:
                print(f"\n✅ APPLY FIX: Update h_W to {optimal[0]}")
                return 0
            else:
                print(f"\n⚠️  h_W helps but insufficient alone (still {abs(optimal[1]-1023)/1023*100:.0f}% error)")
                return 1
        else:
            print("\n❌ h_W is NOT a significant lever (ΔT < 50 K)")
            print("   → Other factors dominate (ACH, volume, geometry)")
            return 1
    else:
        print("No results collected")
        return 1

if __name__ == "__main__":
    sys.exit(main())
