"""
Validation dataset loader and checker for Phase 10 field validation.
Loads field targets from CSV and provides validation pass/fail checking.
"""

import csv
from typing import List, Tuple, Dict, Optional


class ValidationDataset:
    """
    Loads and queries field validation targets from CSV.
    
    Usage:
        dataset = ValidationDataset("data/field_targets.csv")
        targets = dataset.get_targets("NIST_RSE_01")
        passes = dataset.check_prediction("NIST_RSE_01", "peak_HUL_temp", 885.0)
    """
    
    def __init__(self, csv_path: str):
        """Load validation targets from CSV file."""
        self.targets: Dict[str, List[Dict]] = {}
        self._load_csv(csv_path)
    
    def _load_csv(self, csv_path: str):
        """Parse CSV and organize by scenario_id."""
        with open(csv_path, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                scenario_id = row['scenario_id']
                if scenario_id not in self.targets:
                    self.targets[scenario_id] = []
                
                target_entry = {
                    'source': row['source'],
                    'target_type': row['target_type'],
                    'target_value': float(row['target_value']),
                    'lower_bound': float(row['lower_bound']),
                    'upper_bound': float(row['upper_bound']),
                    'units': row['units'],
                    'notes': row.get('notes', '')
                }
                self.targets[scenario_id].append(target_entry)
    
    def get_scenarios(self) -> List[str]:
        """Get list of all scenario IDs."""
        return list(self.targets.keys())
    
    def get_targets(self, scenario_id: str) -> List[Tuple[str, float, float, float, str]]:
        """
        Get all targets for a scenario.
        
        Returns:
            List of (target_type, target_value, lower_bound, upper_bound, units)
        """
        if scenario_id not in self.targets:
            return []
        
        return [
            (t['target_type'], t['target_value'], t['lower_bound'], 
             t['upper_bound'], t['units'])
            for t in self.targets[scenario_id]
        ]
    
    def check_prediction(self, scenario_id: str, target_type: str, 
                        predicted_value: float) -> bool:
        """
        Check if prediction falls within acceptable bounds.
        
        Args:
            scenario_id: Scenario identifier
            target_type: Type of target (e.g., 'peak_HUL_temp')
            predicted_value: Simulated/predicted value
            
        Returns:
            True if prediction is within bounds, False otherwise
        """
        if scenario_id not in self.targets:
            return False
        
        for target in self.targets[scenario_id]:
            if target['target_type'] == target_type:
                return (target['lower_bound'] <= predicted_value <= 
                       target['upper_bound'])
        
        return False
    
    def get_error(self, scenario_id: str, target_type: str, 
                  predicted_value: float) -> Optional[Dict]:
        """
        Calculate error metrics for a prediction.
        
        Returns:
            Dict with 'absolute_error', 'relative_error', 'within_bounds', 'target_value'
            or None if target not found
        """
        if scenario_id not in self.targets:
            return None
        
        for target in self.targets[scenario_id]:
            if target['target_type'] == target_type:
                target_val = target['target_value']
                abs_error = predicted_value - target_val
                rel_error = abs_error / target_val if target_val != 0 else float('inf')
                within_bounds = (target['lower_bound'] <= predicted_value <= 
                               target['upper_bound'])
                
                return {
                    'absolute_error': abs_error,
                    'relative_error': rel_error,
                    'within_bounds': within_bounds,
                    'target_value': target_val,
                    'lower_bound': target['lower_bound'],
                    'upper_bound': target['upper_bound'],
                    'units': target['units']
                }
        
        return None
    
    def get_scenario_summary(self, scenario_id: str) -> Dict:
        """Get summary information for a scenario."""
        if scenario_id not in self.targets:
            return {}
        
        targets_list = self.targets[scenario_id]
        if not targets_list:
            return {}
        
        return {
            'scenario_id': scenario_id,
            'source': targets_list[0]['source'],
            'num_targets': len(targets_list),
            'target_types': [t['target_type'] for t in targets_list]
        }


class ScenarioLibrary:
    """
    Loads and manages scenario definitions from CSV.
    
    Usage:
        library = ScenarioLibrary("data/scenario_library.csv")
        scenario = library.get_scenario("SC001")
        all_scenarios = library.get_all_scenarios()
    """
    
    def __init__(self, csv_path: str):
        """Load scenario library from CSV file."""
        self.scenarios: Dict[str, Dict] = {}
        self._load_csv(csv_path)
    
    def _load_csv(self, csv_path: str):
        """Parse CSV and organize by scenario_id."""
        with open(csv_path, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                scenario_id = row['scenario_id']
                
                # Parse numeric fields
                scenario = {
                    'scenario_id': scenario_id,
                    'description': row['description'],
                    'volume_m3': float(row['volume_m3']),
                    'fire_load_MJ': float(row['fire_load_MJ']),
                    'ACH': float(row['ACH']),
                    'ignition_HRR_kW': float(row['ignition_HRR_kW']),
                    'peak_HRR_kW': float(row['peak_HRR_kW']),
                    'suppression_time_s': None if row['suppression_time_s'] == 'none' 
                                         else float(row['suppression_time_s']),
                    'wall_material': row['wall_material'],
                    'validation_class': row['validation_class']
                }
                
                self.scenarios[scenario_id] = scenario
    
    def get_scenario(self, scenario_id: str) -> Optional[Dict]:
        """Get scenario definition by ID."""
        return self.scenarios.get(scenario_id)
    
    def get_all_scenarios(self) -> List[Dict]:
        """Get all scenario definitions."""
        return list(self.scenarios.values())
    
    def get_scenarios_by_class(self, validation_class: str) -> List[Dict]:
        """Get scenarios filtered by validation class."""
        return [s for s in self.scenarios.values() 
                if s['validation_class'] == validation_class]
    
    def get_validation_classes(self) -> List[str]:
        """Get unique validation classes."""
        return list(set(s['validation_class'] for s in self.scenarios.values()))
