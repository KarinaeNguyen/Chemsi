VFEP / Chemsi — Simulation Integrity Process

This document defines the integrity validation process for the VFEP / Chemsi simulation engine.
The objective is to ensure the simulation is trustworthy, reproducible, and suitable for sustained industrial development.

Integrity validation is divided into five ordered steps.
Each step is a hard gate: downstream work must not proceed until the current step passes.

────────────────────────────────────────────────────────────
Overview — The Five Integrity Steps
────────────────────────────────────────────────────────────

Step 1A — Deterministic Execution (PASSED)
Step 1B — Numerical Safety & Boundedness (PASSED)
Step 1C — State Transition Integrity (PASSED)
Step 2  — Physical Consistency (PASSED)
Step 3  — Interface & State Ownership Integrity (PASSED)
Step 4  — Visualization & Interpretation Validation (ACTIVE)
Step 5  — Long-Horizon Regression & Change Control (LOCKED)

This repository has successfully passed Steps 1A, 1B, 1C, Step 2, and Step 3.
The current integrity gate is Step 4.

────────────────────────────────────────────────────────────
Step 3 — Interface & State Ownership Integrity (PASSED)
────────────────────────────────────────────────────────────

Objective

Verify that the public interface of vfep::Simulation cannot violate correctness
through improper state exposure, ownership errors, or API misuse.

This step ensures correctness cannot be compromised by external code,
even if the physics and numerics are sound.

This step answers the question:

“Can a caller accidentally or maliciously break the simulation
through aliasing, shared state, reset leakage, unsafe observation, or command misuse?”

────────────────────────────────────────────────────────────
Step 3 Structure — Sub-Test Groups
────────────────────────────────────────────────────────────

Step 3 is divided into three sub-groups.
Each sub-group is independently enforced and must pass in full.

Failure in any sub-group blocks progression to Step 4.

────────────────────────────────────────
Step 3.Axx — Snapshot & Observation Safety
────────────────────────────────────────

Focus:
Prevent aliasing and unsafe exposure of internal mutable state.

Risks addressed:
• Mutable aliasing via snap(), observe(), or StateSnap
• Shallow copies of internal buffers
• Observation side effects
• Unsafe read-only assumptions

Guarantees enforced:
• Returned snapshots never alias internal mutable state
• Mutating a snapshot cannot affect the simulation
• Repeated observe() calls are side-effect free
• Telemetry and sample buffers are returned by value (not reference/view)
• Observations remain finite, bounded, and internally consistent

Typical tests:
• Snapshot mutation attempts
• Repeated observation identity checks
• Buffer tampering checks
• Observation finiteness and boundedness assertions

────────────────────────────────────────
Step 3.Bxx — Instance & Reset Ownership Integrity
────────────────────────────────────────

Focus:
Ensure strict ownership boundaries between simulation instances and across resets.

Risks addressed:
• Hidden shared state between instances
• Static or global accumulators
• Reset failing to fully clear internal state
• Cached data surviving reset

Guarantees enforced:
• Multiple vfep::Simulation instances never share state
• Actions in one instance cannot affect another
• reset() returns the simulation to a clean baseline
• Post-reset state matches a fresh instance
• No latches, accumulators, or cached state survive reset

Typical tests:
• Dual-instance interference checks
• Cross-instance command isolation
• Dirty-state → reset → baseline equivalence checks
• Multi-cycle reset stress tests

────────────────────────────────────────
Step 3.Cxx — Command & API Misuse Safety
────────────────────────────────────────

Focus:
Ensure the command interface is safe under misuse and cannot corrupt state.

Risks addressed:
• Commands advancing time
• Unsafe command ordering
• Repeated commands causing drift
• Commands creating invalid intermediate states

Guarantees enforced:
• Time advances only via step(dt)
• Commands never advance time
• Repeated or out-of-order commands are safe no-ops (no corruption)
• Commands cannot produce NaN/Inf or invalid states
• Public API calls are side-effect free unless explicitly documented

Typical tests:
• 3C1: command spam without stepping
• 3C2: terminal-state misuse safety (freeze/no mutation)
• 3C3: high-frequency polling stability (idempotent + non-perturbing paired run)

────────────────────────────────────────────────────────────
Test Method
────────────────────────────────────────────────────────────

Step 3 is validated using additional NumericIntegrity tests
organized as the 3.Axx / 3.Bxx / 3.Cxx series.

All Step 3 tests are:
• Always-on (never compiled out)
• Enforced in Debug and Release
• Public-API-only
• Semantic (contract enforcement, not numeric tuning)
• Fail-hard on contract violation

Tests rely on:
• Paired runs
• Snapshot mutation attempts
• Cross-instance comparisons
• Reset equivalence against fresh instances
• Misuse stress patterns (command spam, polling spam, terminal misuse)

────────────────────────────────────────────────────────────
Gate Status
────────────────────────────────────────────────────────────

Step 1A — Deterministic Execution: PASSED
Step 1B — Numerical Safety & Boundedness: PASSED
Step 1C — State Transition Integrity: PASSED
Step 2  — Physical Consistency: PASSED
Step 3  — Interface & State Ownership Integrity: PASSED
Step 4  — Visualization & Interpretation Validation: ACTIVE

Next allowed work:
Proceed with Step 4 only. Step 5 remains locked until Step 4 passes.

────────────────────────────────────────────────────────────
How to Run the Integrity Gate
────────────────────────────────────────────────────────────

From cpp_engine:

cmake --build .\build --config Release
.\build\Release\NumericIntegrity.exe

(Recommended) also run Debug to ensure “always-on” enforcement:

cmake --build .\build --config Debug
.\build\Debug\NumericIntegrity.exe
