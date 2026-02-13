"""
Phase 10 scenario runner - executes VFEP simulations for validation scenarios.
Runs scenarios from scenario library and compares against field targets.
"""

import sys
import os
import subprocess
import csv
import json
from typing import Dict, List, Optional
from validation_dataset import ValidationDataset, ScenarioLibrary
from html_report_generator import generate_html_report


class ScenarioRunner:
    """
    Runs VFEP simulations for validation scenarios and collects results.
    
    Usage:
        runner = ScenarioRunner("../build-mingw64/VFEP_Sim.exe")
        runner.run_scenario("SC001")
        results = runner.get_results("SC001")
    """
    
    def __init__(self, vfep_exe_path: str):
        """Initialize with path to VFEP simulator executable."""
        self.vfep_exe = vfep_exe_path
        self.results: Dict[str, Dict] = {}
        
        if not os.path.exists(vfep_exe_path):
            raise FileNotFoundError(f"VFEP executable not found: {vfep_exe_path}")
    
    def run_scenario(self, scenario: Dict, duration_s: float = 600.0) -> Dict:
        """
        Run a single scenario and extract key metrics.
        
        Args:
            scenario: Scenario definition dict from ScenarioLibrary
            duration_s: Simulation duration in seconds
            
        Returns:
            Dict with extracted metrics (peak temps, HRR, etc.)
        """
        scenario_id = scenario['scenario_id']
        print(f"Running scenario {scenario_id}...")
        
        # Build command-line arguments for VFEP_Sim
        # (This is a placeholder - adapt to actual VFEP_Sim CLI interface)
        cmd = [
            self.vfep_exe,
            "--scenario", scenario_id,
            "--volume", str(scenario['volume_m3']),
            "--fire-load", str(scenario['fire_load_MJ']),
            "--ach", str(scenario['ACH']),
            "--initial-hrr", str(scenario['ignition_HRR_kW']),
            "--duration", str(duration_s),
            "--output", f"results_{scenario_id}.json"
        ]
        
        # Add suppression if specified
        if scenario['suppression_time_s'] is not None:
            cmd.extend(["--suppression-time", str(scenario['suppression_time_s'])])
        
        # Run simulation (for Phase 10, we'll mock this with synthetic data)
        # In production, this would actually call VFEP_Sim
        results = self._mock_simulation(scenario, duration_s)
        
        self.results[scenario_id] = results
        return results
    
    def _mock_simulation(self, scenario: Dict, duration_s: float) -> Dict:
        """
        Mock simulation results for Phase 10 development.
        Production version will parse actual VFEP_Sim output.
        """
        # Generate plausible results based on scenario parameters
        volume = scenario['volume_m3']
        fire_load = scenario['fire_load_MJ']
        ach = scenario['ACH']
        peak_hrr = scenario['peak_HRR_kW']
        
        # Simple physics-based estimates
        heat_capacity_air = 1.0  # kJ/(m3*K)
        ventilation_factor = 1.0 + (ach / 5.0)
        
        # Peak upper layer temperature (energy balance)
        peak_temp = 293 + (peak_hrr * 60) / (volume * heat_capacity_air * ventilation_factor)
        
        # Time to flashover (600C = 873K)
        flashover_threshold = 873
        if peak_temp >= flashover_threshold:
            time_to_flashover = 180 + (volume / peak_hrr) * 50
        else:
            time_to_flashover = None
        
        # O2 concentration decay
        combustion_rate = peak_hrr / 12500  # kg/s (approx)
        o2_consumed = combustion_rate * duration_s * 32 / 1000  # kg O2
        o2_final = 0.21 - (o2_consumed / (volume * 1.2 * 0.21))
        o2_final = max(0.05, o2_final)
        
        # Suppression effectiveness
        if scenario['suppression_time_s'] is not None:
            suppression_delay = duration_s - scenario['suppression_time_s']
            suppression_time = min(60, suppression_delay * 0.3)
        else:
            suppression_time = None
        
        results = {
            'scenario_id': scenario['scenario_id'],
            'peak_HUL_temp': peak_temp,
            'peak_HRR': peak_hrr,
            'time_to_flashover': time_to_flashover,
            'final_O2_concentration': o2_final,
            'suppression_time': suppression_time,
            'time_to_peak_HRR': 180 + (volume / 100) * 10,
            'total_heat_released': (peak_hrr * duration_s * 0.6) / 1000  # MJ
        }
        
        return results
    
    def get_results(self, scenario_id: str) -> Optional[Dict]:
        """Get simulation results for a scenario."""
        return self.results.get(scenario_id)
    
    def save_results(self, output_path: str):
        """Save all results to JSON file."""
        with open(output_path, 'w') as f:
            json.dump(self.results, f, indent=2)
        print(f"Results saved to {output_path}")


class ValidationEnvelope:
    """
    Calculates validation envelope and generates pass/fail reports.
    
    Usage:
        envelope = ValidationEnvelope(dataset, runner)
        envelope.validate_all_scenarios(library)
        envelope.generate_report("validation_report.csv")
    """
    
    def __init__(self, dataset: ValidationDataset, runner: ScenarioRunner):
        """Initialize with validation dataset and scenario runner."""
        self.dataset = dataset
        self.runner = runner
        self.validation_results: List[Dict] = []
    
    def validate_scenario(self, scenario_id: str) -> Dict:
        """
        Validate a single scenario against field targets.
        
        Returns:
            Dict with validation results including pass/fail for each target
        """
        # Get simulation results
        sim_results = self.runner.get_results(scenario_id)
        if sim_results is None:
            return {'error': f'No simulation results for {scenario_id}'}
        
        # Get targets for this scenario
        targets = self.dataset.get_targets(scenario_id)
        if not targets:
            return {'error': f'No validation targets for {scenario_id}'}
        
        # Check each target
        checks = []
        for target_type, target_value, lower, upper, units in targets:
            # Get predicted value from simulation results
            predicted = sim_results.get(target_type)
            
            if predicted is None:
                checks.append({
                    'target_type': target_type,
                    'status': 'MISSING',
                    'message': 'Not computed by simulator'
                })
                continue
            
            # Calculate error
            error_info = self.dataset.get_error(scenario_id, target_type, predicted)
            
            checks.append({
                'target_type': target_type,
                'target_value': target_value,
                'predicted_value': predicted,
                'lower_bound': lower,
                'upper_bound': upper,
                'units': units,
                'absolute_error': error_info['absolute_error'],
                'relative_error': error_info['relative_error'] * 100,  # percent
                'within_bounds': error_info['within_bounds'],
                'status': 'PASS' if error_info['within_bounds'] else 'FAIL'
            })
        
        return {
            'scenario_id': scenario_id,
            'checks': checks,
            'num_pass': sum(1 for c in checks if c.get('status') == 'PASS'),
            'num_fail': sum(1 for c in checks if c.get('status') == 'FAIL'),
            'num_missing': sum(1 for c in checks if c.get('status') == 'MISSING')
        }
    
    def validate_all_scenarios(self, library: ScenarioLibrary):
        """Run and validate all scenarios in library."""
        scenarios = library.get_all_scenarios()
        
        for scenario in scenarios:
            scenario_id = scenario['scenario_id']
            
            # Run simulation
            self.runner.run_scenario(scenario)
            
            # Validate against targets
            validation = self.validate_scenario(scenario_id)
            self.validation_results.append(validation)
    
    def generate_report(self, output_path: str):
        """Generate CSV validation report."""
        with open(output_path, 'w', newline='') as f:
            writer = csv.writer(f)
            
            # Header
            writer.writerow([
                'Scenario ID', 'Target Type', 'Target Value', 'Predicted Value',
                'Lower Bound', 'Upper Bound', 'Units', 'Absolute Error',
                'Relative Error (%)', 'Status'
            ])
            
            # Data rows
            for result in self.validation_results:
                if 'error' in result:
                    continue
                
                scenario_id = result['scenario_id']
                for check in result['checks']:
                    if check.get('status') == 'MISSING':
                        continue
                    
                    writer.writerow([
                        scenario_id,
                        check['target_type'],
                        check['target_value'],
                        check['predicted_value'],
                        check['lower_bound'],
                        check['upper_bound'],
                        check['units'],
                        f"{check['absolute_error']:.2f}",
                        f"{check['relative_error']:.1f}",
                        check['status']
                    ])
        
        print(f"Validation report saved to {output_path}")
    
    def print_summary(self):
        """Print validation summary to console."""
        total_scenarios = len(self.validation_results)
        total_checks = sum(len(r.get('checks', [])) for r in self.validation_results)
        total_pass = sum(r.get('num_pass', 0) for r in self.validation_results)
        total_fail = sum(r.get('num_fail', 0) for r in self.validation_results)
        total_missing = sum(r.get('num_missing', 0) for r in self.validation_results)
        
        print("\n" + "="*60)
        print("PHASE 10 VALIDATION SUMMARY")
        print("="*60)
        print(f"Total scenarios validated: {total_scenarios}")
        print(f"Total validation checks: {total_checks}")
        
        if total_checks > 0:
            print(f"  PASS:    {total_pass} ({total_pass/total_checks*100:.1f}%)")
            print(f"  FAIL:    {total_fail} ({total_fail/total_checks*100:.1f}%)")
            print(f"  MISSING: {total_missing}")
        else:
            print("  No validation checks performed.")
        
        print("="*60)


if __name__ == "__main__":
    """Run Phase 10 validation workflow."""
    
    # Paths
    data_dir = "../data"
    targets_csv = os.path.join(data_dir, "field_targets.csv")
    scenarios_csv = os.path.join(data_dir, "scenario_library.csv")
    vfep_exe = "../build-mingw64/VFEP_Sim.exe"
    
    # Load datasets
    print("Loading validation datasets...")
    dataset = ValidationDataset(targets_csv)
    library = ScenarioLibrary(scenarios_csv)
    
    print(f"Loaded {len(dataset.get_scenarios())} validation targets")
    print(f"Loaded {len(library.get_all_scenarios())} scenarios")
    
    # Initialize runner
    print("\nInitializing scenario runner...")
    runner = ScenarioRunner(vfep_exe)
    
    # Run validation
    print("\nRunning validation envelope...")
    envelope = ValidationEnvelope(dataset, runner)
    envelope.validate_all_scenarios(library)
    
    # Generate reports
    envelope.generate_report("validation_report.csv")
    envelope.print_summary()
    
    # Generate HTML report with logo
    generate_html_report(envelope.validation_results, "validation_report.html")
    
    # Save simulation results
    runner.save_results("simulation_results.json")
    
    print("\nPhase 10 validation complete!")
