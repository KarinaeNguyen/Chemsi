# Phase 6: Multi-Zone Crash Root Cause Analysis

## The Issue: Why MultiZone Broke

When we enabled `ThreeZone` model, ALL metrics collapsed to zero:
- Peak T: 295.2 K (ambient)
- Peak HRR: 0.9 kW (essentially zero)
- All 4 scenarios failed completely

## Root Cause Discovered: Configuration Mismatch

### Single-Zone Config (Working)
```cpp
rc.volume_m3  = 30.0;   // Total compartment volume
rc.area_m2    = 60.0;   // Proportional surface area
// Used by: setConfig() in resetToDataCenterRackScenario()
```

### Multi-Zone Config (Using Default Values)
```cpp
// From ReactorConfig header (default initialization)
double V_upper_m3  = 30.0;    // Zone 0: upper smoke layer
double V_lower_m3  = 80.0;    // Zone 1: lower ambient
// Total: 110 m³ (NOT matching single-zone 30 m³!)
```

### The Crash Sequence

1. **User sets single-zone config:**
   ```cpp
   rc.volume_m3 = 30.0;      // ✓ Correct
   reactor_.setConfig(rc);   // Sets single-zone only
   ```

2. **User changes model type:**
   ```cpp
   rc.model_type = ReactorConfig::Model::ThreeZone;
   // ⚠️ BUT: V_upper_m3 and V_lower_m3 still have defaults (30+80=110)
   // ⚠️ AND: setConfig() may not have been called with V_upper/V_lower values
   ```

3. **Multi-zone initialization (in stepMultiZone):**
   ```cpp
   // Line 149 & 157 in Reactor.cpp
   zones_[0].V_m3 = cfg_.V_upper_m3;  // Gets 30.0 m³ (correct by accident)
   zones_[1].V_m3 = cfg_.V_lower_m3;  // Gets 80.0 m³ (WRONG! Should be 0 m³ for lower in 30 m³ space)
   ```

4. **Combustion happens in upper zone only:**
   ```cpp
   // Line 355: Chemistry reaction occurs ONLY in zones_[0]
   zones_[0].T_K = 295.15 + 10.0 = 305.15 K  // Initial temp
   
   // Lower zone 1 gets 80 m³ volume and is cooled hard
   // Plume zone 2 is empty
   ```

5. **Heat generation insufficient:**
   ```cpp
   // If HRR_actual = 85.6 kW distributed across 110 m³ total
   // But upper zone is only 30 m³ = 27% of total
   // The heat is DILUTED by larger total volume
   // Also: syncMultiZoneToSingleZone() does volume-weighted average
   // T_avg = (305K × 30 + 295K × 80) / 110 = 296K ≈ 295K (ambient!)
   ```

## The Problem in Code

**File:** `cpp_engine/src/Reactor.cpp` Line 192-204

```cpp
void Reactor::syncMultiZoneToSingleZone() noexcept {
    // ...
    // Temperature: volume-weighted average
    double V_total = zones_[0].V_m3 + zones_[1].V_m3 + zones_[2].V_m3;
    if (V_total > 0.0) {
        T_K_ = (zones_[0].T_K * zones_[0].V_m3 + 
                zones_[1].T_K * zones_[1].V_m3 + 
                zones_[2].T_K * zones_[2].V_m3) / V_total;
                // ^^^ This averages 305K upper zone with 295K lower zone
                // Result: ~296K (appears ambient despite upper zone heating!)
    }
}
```

## Mathematical Proof of Crash

With current mismatched config:
```
Total Volume:           110 m³ (30 upper + 80 lower + variable plume)
Heat Release Rate:      85.6 kW
Initial Upper Zone:     305.15 K (30 m³)
Initial Lower Zone:     295.15 K (80 m³)
Heat Capacity (air):    ~1000 J/kg/K @ 295K

After 1 second:
  Q_net_upper = 85.6 kW - losses
  dT_upper = (85,600 - losses) * 1 / (Cp_mass_upper)
  If no losses: dT_upper ≈ 85.6 kW / (30 m³ × 1.2 kg/m³ × 1000 J/kg/K)
               = 85,600 / 36,000 = 2.4 K/s
  
  After 1s: zones_[0].T_K ≈ 307.6 K

Volume-weighted back to single-zone:
  T_avg = (307.6 × 30 + 295.15 × 80 + 295.15 × 0) / 110
        = (9228 + 23612) / 110
        = 32,840 / 110
        = 298.5 K ≈ 295K (appears room temperature!)
```

**Validation sees:** T ≈ 295K (ambient) because volume averaging drowns out the heated upper zone!

## The Fix Required

### Option 1: Correct Multi-Zone Volume Config (BEST)

In `resetToDataCenterRackScenario()` when setting model to ThreeZone:

```cpp
// BEFORE: Only set single-zone volume
rc.volume_m3  = 30.0;
reactor_.setConfig(rc);

// AFTER: Also set multi-zone volumes when enabling ThreeZone
if (model_type == ReactorConfig::Model::ThreeZone) {
    rc.V_upper_m3 = 30.0;   // All heat goes here
    rc.V_lower_m3 = 0.0;    // Or very small (epsilon)
    // This way upper zone is full compartment
    // Volume average = upper zone only = actual temperature
}
```

### Option 2: Change Observation API

Instead of averaging zones, measure peak zone:

```cpp
double observe().T_K_peak() const {
    return std::max({zones_[0].T_K, zones_[1].T_K, zones_[2].T_K});
}
```

But this requires changing ValidationSuite.cpp observation logic.

### Option 3: Fix Sync Logic

In `syncMultiZoneToSingleZone()`, weight by mass instead of volume:

```cpp
double m_total = 0.0;
for (size_t i = 0; i < 3; ++i) {
    m_total += zones_[i].totalMass(sp_);
}
if (m_total > 0.0) {
    T_K_ = (zones_[0].T_K * zones_[0].totalMass(sp_) + 
            zones_[1].T_K * zones_[1].totalMass(sp_) + 
            zones_[2].T_K * zones_[2].totalMass(sp_)) / m_total;
}
```

## Recommendation

**Use Option 1: Fix Configuration**

Why:
1. Simplest change: 1 line in Simulation.cpp
2. Maintains backward compatibility
3. Aligns with Phase 3 intent (upper zone = hot layer, lower = minimal/none)
4. Validates thermodynamics correctly

**Action:**

In `cpp_engine/src/Simulation.cpp`, `resetToDataCenterRackScenario()` (around line 448):

```cpp
{
    ReactorConfig rc;
    rc.volume_m3  = 30.0;      // Total (for single-zone fallback)
    rc.area_m2    = 60.0;      
    rc.T_amb_K    = kT_amb_K;
    rc.h_W_m2K    = 10.0;
    rc.emissivity = 0.85;
    
    // NEW: Set multi-zone volumes when ThreeZone is active
    rc.V_upper_m3 = 30.0;      // All combustion heat here
    rc.V_lower_m3 = 0.0;       // No lower zone (or epsilon ~0.1)
    rc.A_plume_m2 = 1.0;       // For plume dynamics
    
    reactor_.setConfig(rc);
}
```

## Expected Result After Fix

With corrected config (V_upper = 30, V_lower = 0):

```
Volume-weighted average after heating:
  T_avg = (307.6 × 30 + 295.15 × 0 + 295.15 × ε) / 30
        = (9228) / 30
        = 307.6 K ✓ (matches upper zone, as intended!)
```

Validation output should show:
- **ISO 9705:** T_K > 700 K expected (stratified, upper zone peak) ✓
- **NIST Data:** HRR ≈ 85.6 kW (unchanged combustion) ✓
- **Suppression:** Higher effectiveness (agent reaches hot zone) ✓
- **Stratification:** ΔT = T_upper - T_lower visible ✓

## Implementation Checklist

- [ ] Apply config fix in Simulation.cpp
- [ ] Rebuild with `build-vfep` task
- [ ] Run ValidationSuite.exe
- [ ] Compare with baseline (should see improvement, not collapse)
- [ ] Document findings

