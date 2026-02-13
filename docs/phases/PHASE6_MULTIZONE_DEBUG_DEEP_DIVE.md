# Phase 6: Multi-Zone Diagnostic Deep Dive

## Session Status

We have successfully:
1. ✅ Fixed multi-zone fuel distribution (syncSingleZoneToMultiZone)
2. ✅ Set correct multi-zone volumes in constructor (V_upper=30, V_lower=0.1)
3. ✅ Verified baseline still works (NIST at 85.6 kW, 14.1% error)

Still broken:
4. ❌ Enabling multi-zone causes HRR=1.9 kW (should be 85.6 kW)
5. ❌ Temperature drops to 295K (ambient)

## Current Multi-Zone Activation Sequence

When enabling multi-zone in resetToDataCenterRackScenario():

```cpp
// Step 1: Constructor sets config
rc.V_upper_m3 = 30.0;
rc.V_lower_m3 = 0.1;
rc.model_type = SingleZone;  // DEFAULT
reactor_.setConfig(rc);

// Step 2: resetToDataCenterRackScenario() disables zones
zones_initialized_ = false;  // Flag is still false

// Step 3: Try to enable ThreeZone
ReactorConfig rc = reactor_.config();  // Get current config (with V_upper, V_lower set)
rc.model_type = Model::ThreeZone;      // Change model
reactor_.setConfig(rc);                 // Re-apply config
// zones_initialized_ still false!

// Step 4: First step() call
// if (!zones_initialized_) {
//     initializeMultiZone();      // Creates zones_[0], [1], [2]
//     syncSingleZoneToMultiZone(); // Distributes fuel 100% to zones_[0]
// }
```

## The Real Problem: Zone Initialization Timing

Current initialization in Reactor::step():

```cpp
if (!zones_initialized_) {
    initializeMultiZone();        // Allocate zone array
    syncSingleZoneToMultiZone();  // Copy single-zone state to zones
}
```

The issue: **When syncSingleZoneToMultiZone() is called, the zones might not have proper V_m3 values yet!**

Let me trace initializeMultiZone():

```cpp
void Reactor::initializeMultiZone() noexcept {
    zones_.resize(3);  // Create 3 zones
    
    zones_[0].V_m3 = cfg_.V_upper_m3;  // 30.0 m³
    zones_[1].V_m3 = cfg_.V_lower_m3;  // 0.1 m³
    zones_[2].V_m3 = cfg_.V_plume_m3;  // Uninitialized or 0?
    
    // Initialize n_mol for each zone
    for (auto& z : zones_) {
        z.n_mol.assign(sp_.size(), 0.0);
        z.T_K = cfg_.T_amb_K;
    }
    
    zones_initialized_ = true;
}
```

The question: **Does Zone have a V_plume_m3 parameter?** Or is it dynamically calculated?

## Hypothesis: V_plume_m3 Not Set

Looking at line 157 in Reactor.cpp:
```cpp
zones_[1].V_m3 = cfg_.V_lower_m3;  // 0.1
zones_[2].V_m3 = ???;              // WHAT VALUE?
```

If zones_[2].V_m3 is not set (or set to 0), then:
- Plume zone has NO volume
- Plume can't hold fuel/moles (they'd cause inf concentration)
- Or syncing clamps moles to zero

## Secondary Hypothesis: Problem in heatLoss_W_zone()

The multi-zone combustion happens in upper zone:

```cpp
// STEP 2: COMBUSTION (only in upper zone)
const ReactionResult rr = chemistry_.react(dt, zones_[0].T_K, ..., zones_[0].n_mol, ...);
if (rr.heat_W > 0.0) {
    outCombustionHRR_W = rr.heat_W * f_quench;
}

// STEP 3: THERMAL UPDATES
double Qloss_upper = heatLoss_W_zone(zones_[0]);  // <-- THIS MIGHT BE HUGE!
double Qnet = (outCombustionHRR_W * mult) - Qloss_upper - Qext;
double dT = (Qnet * dt) / Cp_upper;
```

If `heatLoss_W_zone()` is calculated incorrectly, it could massively dampen heating.

heatLoss_W_zone() probably calculates:
```cpp
// Convection (proportional to volume?)
double Qconv = h_W * A * (T - T_amb)

// Radiation (proportional to volume?)
double Qrad = epsilon * sigma * A * (T^4 - T_amb^4)
```

**CRITICAL QUESTION:** Is `A` (area for heat loss) correctly scaled for the zone?

In single-zone: `rc.area_m2 = 60` m²
In multi-zone upper zone: Should it also be 60 m²? Or proportional?

If heat loss area is NOT scaled, then the upper zone loses heat as if it's the entire 60 m² surface. This is CORRECT because the upper zone is in contact with all the walls!

## Theory: Why Multi-Zone HRR Collapsed

Scenario: Enables ThreeZone, enables multi-zone with V_upper=30, V_lower=0.1

Step() execution with multi-zone:

```
1. initializeMultiZone() called
   zones_[0].V_m3 = 30.0
   zones_[1].V_m3 = 0.1
   zones_[2].V_m3 = ??? (undefined or 0)
   
2. syncSingleZoneToMultiZone() called
   zones_[0].n_mol = 100% of fuel/O2
   zones_[1].n_mol = 0%
   zones_[2].n_mol = 0%
   
3. Combustion reaction in zones_[0]
   HRR_actual ≈ 85.6 kW (if conditions normal)
   
4. Heat loss calculation
   Qloss_upper = h_W * area * (T_upper - T_amb)
                = 10 * 60 * (305 - 295)
                = 6000 W
   
5. Net heat
   Qnet = 85600 - 6000 = 79600 W (should be ~80 kW)
   
6. Temperature rise
   Cp_upper = mixture_cp(zones_[0])
   dT = 79600 * dt / Cp_upper
   
   If zones_[0] starts at 50 mol and Cp_upper = 50 * 29 J/K ≈ 1450 J/K
   Then: dT = 79600 * 0.05 / 1450 = 2.7 K per step
   
   After 10 steps: ΔT ≈ 27 K → T ≈ 322 K (NOT ambient!)
```

So theoretically it SHOULD work. The fact that T=295K (ambient) suggests:
- Either HRR = 0 (no combustion)
- Or Qloss_upper >> HRR (massive heat loss)
- Or temperature is being explicitly set to ambient somewhere

## Debugging Strategy

To identify the real problem, we need to know:

1. **Is combustion happening in multi-zone?**
   - Check: chemistry_.react() is returning non-zero HRR
   - Measure: zones_[0].T_K after step (should rise above 295K)

2. **What is heatLoss_W_zone(zones_[0])?**
   - Might be: h * A * zone_surface_area * ΔT
   - If zone_surface_area >> actual zone surface, loss >> heat
   
3. **Is syncMultiZoneToSingleZone() working?**
   - After multi-zone step, T_K_ = volume_weighted_average(zones)
   - Should be ≈ zones_[0].T_K (since V_upper >> V_lower)
   
4. **Are zones being properly initialized with moles?**
   - zones_[0].n_mol should have fuel after syncSingleZoneToMultiZone
   - Otherwise chemistry_.react has no fuel to burn

## Next Debug Steps

Option A: Add debug logging to step/stepMultiZone
Option B: Create minimal test to check zone values
Option C: Inspect Zone::heatLoss_W_zone implementation

**Recommendation:** Start with Option C - understand heatLoss calculation

