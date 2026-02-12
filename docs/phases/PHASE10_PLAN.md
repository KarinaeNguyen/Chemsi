# Phase 10 Plan: Field Validation & Production Readiness

**Prepared**: February 12, 2026
**Status**: Draft Plan
**Goal**: Prepare VFEP for real-world deployment by expanding validation coverage, integrating field data, and formalizing operational artifacts.

---

## Objectives

1. **Field data integration**
   - Identify public compartment-fire datasets and ingest them into VFEP validation workflows.
   - Quantify bias and calibration drift vs. real incidents.

2. **Scenario library expansion**
   - Grow from 7 literature scenarios to 10-15 scenario variants.
   - Cover multiple fire loads, ventilation rates, and suppression timings.

3. **Validation coverage targets**
   - Improve validation coverage from ~25% to 50-75% across scenario classes.
   - Ensure all Phase 9 models remain stable under expanded coverage.

4. **Operational readiness artifacts**
   - Deployment checklist, operator guide, and stakeholder-facing summary of accuracy and limits.

---

## Scope (In)

- Field validation preparation and data ingestion.
- Scenario library definition, generation, and results tracking.
- Validation envelope reporting (point estimate + uncertainty band).
- Production-readiness checklist and training materials.

## Scope (Out)

- Structural response modeling (see Phase 10+ roadmap in Phase 9 docs).
- New physics model development beyond Phase 9 tracks.
- UI redesign or non-validation feature work.

---

## Deliverables

1. **Field Data Pack**
   - Documented sources (metadata, citations, measurement uncertainties).
   - Normalized dataset format for VFEP ingestion.

2. **Scenario Library v1**
   - 10-15 scenarios with defined inputs and acceptance bounds.
   - CSV outputs per scenario plus summary dashboard table.

3. **Validation Envelope Report**
   - Per-scenario: predicted values, target, error, uncertainty band.
   - Pass/fail rules based on literature or field uncertainty.

4. **Operational Readiness Bundle**
   - Deployment checklist.
   - Operator quick guide (scenario selection guidance).
   - Stakeholder summary (accuracy claims, limitations).

---

## Data Sources (Candidate List)

- NIST compartment fire experiments (already referenced).
- ISO 9705 room corner test variants.
- SFPE Handbook datasets.
- Public incident reports with measured temperature/HRR curves.

---

## Metrics & Gates

- **Validation coverage**: >= 10 scenarios, >= 50% of representative classes.
- **Accuracy**: Target mean error <= 20% for field datasets (where uncertainty is known).
- **Stability**: All NumericIntegrity tests pass; no new warnings.
- **Performance**: < 10s per 60s simulation for multi-room scenarios.

---

## Implementation Plan

### Step 1: Define dataset schema
- Standard CSV schema for field/literature targets:
  - scenario_id, source, target_type, target_value, lower_bound, upper_bound, units

### Step 2: Scenario library expansion
- Build 10-15 scenarios across combinations:
  - Fire load: small/medium/large
  - ACH: 0.5-5
  - Volume: 10-200 m^3
  - Suppression: early/late/none

### Step 3: Validation envelope and reporting
- Generate per-scenario results and envelope calculations.
- Produce report table and charts for quick review.

### Step 4: Readiness artifacts
- Prepare operator checklist and stakeholder summary.
- Document calibration changes driven by field data.

---

## Risks & Mitigations

- **Field data variability**: Use uncertainty bands; reject ambiguous datasets.
- **Overfitting to field data**: Separate calibration and validation sets.
- **Performance regressions**: Track performance per scenario and gate on thresholds.

---

## Timeline (2-4 Weeks)

- **Week 1**: Dataset selection + schema + first 5 scenarios.
- **Week 2**: Remaining scenarios + envelope report draft.
- **Week 3**: Operator guide + stakeholder summary + refinement.
- **Week 4**: Final validation pass and readiness sign-off.

---

## Immediate Next Actions

1. Agree on field data sources and licensing.
2. Define scenario schema and output table format.
3. Generate Scenario Library v1 (10-15).