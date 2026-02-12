# Phase 10: Field Validation Data Schema

## Target Dataset CSV Schema

Field validation targets and literature benchmarks use a standardized CSV format for VFEP ingestion.

### Format: `field_targets.csv`

```csv
scenario_id,source,target_type,target_value,lower_bound,upper_bound,units,notes
NIST_RSE_01,NIST-RSE-2008,peak_HUL_temp,873,850,900,K,Room upper layer peak temperature
NIST_RSE_01,NIST-RSE-2008,time_to_flashover,420,390,450,s,Time to 600C upper layer
ISO9705_1,ISO-9705-1993,peak_HRR,2500,2300,2700,kW,Peak heat release rate
ISO9705_1,ISO-9705-1993,total_smoke_production,180,160,200,m2,Total smoke produced
SFPE_Office_Small,SFPE-Handbook-Ed5,final_O2_concentration,0.15,0.14,0.16,fraction,O2 at 600s
```

### Column Definitions

- **scenario_id**: Unique identifier for the validation scenario
- **source**: Citation/reference for the data (experiment, handbook, incident report)
- **target_type**: Physical quantity being validated (e.g., peak_HUL_temp, time_to_flashover, peak_HRR)
- **target_value**: Reference/measured value from field or literature
- **lower_bound**: Lower bound of acceptable range (measurement uncertainty or tolerance)
- **upper_bound**: Upper bound of acceptable range
- **units**: Physical units (K, s, kW, m2, fraction, etc.)
- **notes**: Optional description or context

### Supported Target Types

#### Thermal
- `peak_HUL_temp` - Peak upper layer temperature (K)
- `peak_LLR_temp` - Peak lower layer temperature (K)
- `time_to_flashover` - Time to 600°C upper layer (s)
- `avg_wall_temp` - Average wall temperature at time T (K)

#### Heat Release Rate
- `peak_HRR` - Maximum heat release rate (kW)
- `time_to_peak_HRR` - Time to maximum HRR (s)
- `total_heat_released` - Cumulative heat (MJ)

#### Species & Visibility
- `final_O2_concentration` - O2 volume fraction at end time
- `min_visibility` - Minimum visibility distance (m)
- `peak_CO_ppm` - Peak CO concentration (ppm)

#### Suppression
- `suppression_time` - Time to HRR < 50 kW after activation (s)
- `water_application_rate` - Effective water flow rate (L/min/m2)

## Scenario Definition Schema

### Format: `scenario_library.csv`

```csv
scenario_id,description,volume_m3,fire_load_MJ,ACH,ignition_HRR_kW,peak_HRR_kW,suppression_time_s,wall_material,validation_class
SC001,Small office fire - no suppression,50,800,2.0,50,500,none,concrete,thermal_buildup
SC002,Medium retail - early suppression,120,1500,3.5,100,1200,180,gypsum,suppression_effectiveness
SC003,Large warehouse - delayed detection,500,5000,1.0,200,3000,420,steel,flashover_timing
```

### Column Definitions

- **scenario_id**: Unique scenario identifier
- **description**: Human-readable scenario name
- **volume_m3**: Compartment volume (m³)
- **fire_load_MJ**: Total combustible energy (MJ)
- **ACH**: Air changes per hour
- **ignition_HRR_kW**: Initial fire HRR at t=0 (kW)
- **peak_HRR_kW**: Expected peak HRR (kW)
- **suppression_time_s**: Suppression activation time (s) or "none"
- **wall_material**: Wall type (concrete, gypsum, steel)
- **validation_class**: Category (thermal_buildup, flashover_timing, suppression_effectiveness, ventilation_limited, etc.)

## Python Integration

Use `ValidationDataset` class to load and query targets:

```python
from python_interface.validation_dataset import ValidationDataset

# Load targets
dataset = ValidationDataset("data/field_targets.csv")

# Query specific scenario
targets = dataset.get_targets("NIST_RSE_01")
# Returns: [(target_type, value, lower, upper, units), ...]

# Check if prediction passes
passes = dataset.check_prediction("NIST_RSE_01", "peak_HUL_temp", 885.0)
# Returns: True (within 850-900 K bounds)
```
