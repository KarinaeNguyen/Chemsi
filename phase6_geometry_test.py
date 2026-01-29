#!/usr/bin/env python3
"""
Phase 6: Geometry Sensitivity Test
Tests whether compartment volume/area ratio controls peak temperature.
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


def update_geometry(volume_m3: float, area_m2: float) -> bool:
    content = read_file(CPP_FILE)
    vol_pattern = r"rc\.volume_m3\s*=\s*[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?;"
    area_pattern = r"rc\.area_m2\s*=\s*[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?;"

    if not re.search(vol_pattern, content) or not re.search(area_pattern, content):
        print("    ERROR: Could not find volume/area lines")
        return False

    content = re.sub(vol_pattern, f"rc.volume_m3  = {volume_m3};", content, count=1)
    content = re.sub(area_pattern, f"rc.area_m2    = {area_m2};", content, count=1)
    write_file(CPP_FILE, content)
    return True


def rebuild_core() -> bool:
    result = subprocess.run(
        ["cmake", "--build", str(BUILD_DIR), "--config", "Release", "--target", "VFEP_Sim", "-j", "4"],
        capture_output=True,
        text=True,
        cwd=str(VFEP_ROOT)
    )
    return result.returncode == 0


def run_simulation():
    result = subprocess.run(
        [str(BUILD_DIR / "VFEP_Sim.exe")],
        capture_output=True,
        text=True,
        timeout=120,
        cwd=str(BUILD_DIR)
    )

    output = result.stdout + result.stderr
    success = "Simulation ended" in output

    temps = re.findall(r"\|\s+T=\s+([\d.e+-]+)\s+C\s+\|", output)
    if temps:
        try:
            temps_c = [float(t) for t in temps]
            max_c = max(temps_c)
            return success, max_c + 273.15
        except Exception:
            return success, None

    return success, None


def main():
    print("=" * 80)
    print("PHASE 6: GEOMETRY SENSITIVITY TEST")
    print("=" * 80)
    print("\nTest: Does volume/area ratio affect peak temperature?")
    print("Baseline: volume=120 m³, area=180 m²\n")

    # Keep area fixed; vary volume
    tests = [
        (30.0, 180.0),
        (60.0, 180.0),
        (120.0, 180.0),
        (200.0, 180.0),
    ]

    original = read_file(CPP_FILE)
    results = []

    try:
        for i, (vol, area) in enumerate(tests, 1):
            print(f"[{i}/{len(tests)}] volume={vol} m³, area={area} m²", end=" ... ")

            if not update_geometry(vol, area):
                print("UPDATE FAILED")
                continue

            if not rebuild_core():
                print("BUILD FAILED")
                continue

            success, temp_k = run_simulation()
            if success and temp_k:
                error = abs(temp_k - 1023.0) / 1023.0 * 100
                results.append((vol, area, temp_k, error))
                status = "✓PASS" if error < 20 else ("⚠" if error < 40 else "✗")
                print(f"T={temp_k:.0f}K (err={error:.0f}%) {status}")
            else:
                print("PARSE FAILED")

    finally:
        write_file(CPP_FILE, original)
        print("\nRestored original code")

    print("\n" + "=" * 80)
    if results:
        print(f"\n{'Volume':<10} {'Area':<10} {'T (K)':<12} {'Error %':<12}")
        print("-" * 50)
        for vol, area, temp, err in results:
            print(f"{vol:<10.1f} {area:<10.1f} {temp:<12.0f} {err:<12.1f}")

        temps = [r[2] for r in results]
        t_range = max(temps) - min(temps)
        print(f"\nTemperature range: {t_range:.0f} K")
        if t_range > 100:
            best = min(results, key=lambda x: x[3])
            print(f"✅ Geometry affects temperature. Best: V={best[0]} A={best[1]} → {best[2]:.0f}K")
        else:
            print("❌ Geometry has minimal effect (constant area)")
    else:
        print("No valid results")


if __name__ == "__main__":
    sys.exit(main())
