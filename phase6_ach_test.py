#!/usr/bin/env python3
"""
Phase 6: ACH (Ventilation) Sensitivity Test
Tests if reducing air changes per hour increases peak temperature
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

def update_ach(new_value):
    """Update ACH value in Simulation.cpp"""
    content = read_file(CPP_FILE)
    old_line = "        vc.ACH        = 3.0;"
    new_line = f"        vc.ACH        = {new_value};"
    
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
        'success': False
    }
    
    # Parse ISO 9705 temperature from console output
    temps = re.findall(r"\| T=\s+([\d.e+-]+)\s+C", output)
    if temps:
        try:
            max_temp_c = max([float(t) for t in temps if 200 < float(t) < 1000])
            metrics['iso_temp'] = max_temp_c + 273.15
        except:
            pass
    
    metrics['success'] = 'Simulation ended' in output
    return metrics

def main():
    print("=" * 80)
    print("PHASE 6: ACH (VENTILATION) SENSITIVITY TEST")
    print("=" * 80)
    print("\nHypothesis: High ventilation (ACH=3.0) removes heat too fast")
    print("Test: Reduce ACH → Should increase peak temperature\n")
    print("✓ Target: 1023 K | Current: ~416 K")
    print("✓ Test lever: ACH (lower = less ventilation = higher T)\n")
    
    # Test values: lower ACH = less heat removal = higher temperature
    ach_values = [0.5, 1.0, 3.0, 5.0]  # Include baseline 3.0
    original_content = read_file(CPP_FILE)
    results: List[Tuple[float, float]] = []
    
    try:
        for i, ach in enumerate(ach_values, 1):
            print(f"[{i}/{len(ach_values)}] Testing ACH = {ach} (air changes/hour)")
            print("  " + "-" * 76)
            
            # Update code
            if not update_ach(ach):
                print("  ✗ Failed to update ACH, skipping")
                continue
            
            # Rebuild
            print("  Building...", end=" ", flush=True)
            if not rebuild_core():
                print("✗ FAILED")
                continue
            print("✓")
            
            # Run
            print("  Running...", end=" ", flush=True)
            metrics = run_simulation()
            print("✓")
            
            if metrics['success'] and metrics['iso_temp']:
                temp_k = metrics['iso_temp']
                error_pct = abs(temp_k - 1023.0) / 1023.0 * 100
                results.append((ach, temp_k))
                
                status = "✓ PASS" if temp_k >= 900 else ("⚠ CLOSE" if temp_k >= 700 else "✗ FAIL")
                print(f"  Peak T: {temp_k:.1f} K | Error: {error_pct:.1f}% {status}")
            else:
                print("  ✗ Failed to parse results")
    
    finally:
        # Restore original
        print(f"\n[CLEANUP] Restoring ACH = 3.0")
        write_file(CPP_FILE, original_content)
    
    # Summary
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    
    if results:
        print(f"\n{'ACH (1/h)':<15} {'Peak T (K)':<15} {'Error %':<15} {'Status'}")
        print("-" * 60)
        for ach, temp in results:
            error = abs(temp - 1023.0) / 1023.0 * 100
            status = "✓ PASS" if temp >= 900 else ("⚠ CLOSE" if temp >= 700 else "✗ FAIL")
            print(f"{ach:<15.1f} {temp:<15.1f} {error:<15.1f} {status}")
        
        # Analysis
        print("\n" + "=" * 80)
        print("ANALYSIS")
        print("=" * 80)
        
        temps_only = [r[1] for r in results]
        temp_range = max(temps_only) - min(temps_only)
        
        print(f"\nTemperature range: {min(temps_only):.1f} - {max(temps_only):.1f} K (ΔT = {temp_range:.1f} K)")
        
        # Find trend
        low_ach = results[0] if results[0][0] == min(r[0] for r in results) else None
        high_ach = results[-1] if results[-1][0] == max(r[0] for r in results) else None
        
        if low_ach and high_ach:
            slope = (high_ach[1] - low_ach[1]) / (high_ach[0] - low_ach[0])
            print(f"\nTrend: {slope:.1f} K per ACH increase")
            
            if slope < -50:  # Negative slope = lower ACH gives higher T
                print("✅ ACH IS A CONTROL LEVER (inverse relationship)")
                
                # Find optimal
                optimal = min(results, key=lambda x: abs(x[1] - 1023.0))
                print(f"\n📌 RECOMMENDED: ACH = {optimal[0]:.1f}")
                print(f"   → Predicted: {optimal[1]:.1f} K")
                print(f"   → Error: {abs(optimal[1] - 1023.0) / 1023.0 * 100:.1f}%")
                
                if abs(optimal[1] - 1023.0) / 1023.0 < 0.20:
                    print(f"\n✅ APPLY FIX: Reduce ACH to {optimal[0]}")
                    return 0
            elif temp_range > 100:
                print("✅ ACH AFFECTS TEMPERATURE (but maybe not enough)")
            else:
                print("❌ ACH has minimal effect")
        
    else:
        print("No results collected")
        return 1

if __name__ == "__main__":
    sys.exit(main())
