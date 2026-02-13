"""
Unit tests for Phase 10 validation infrastructure.
Tests ValidationDataset and ScenarioLibrary classes.
"""

import unittest
import os
import tempfile
import csv
from validation_dataset import ValidationDataset, ScenarioLibrary


class TestValidationDataset(unittest.TestCase):
    """Test ValidationDataset loading and querying."""
    
    def setUp(self):
        """Create temporary CSV file for testing."""
        self.temp_dir = tempfile.mkdtemp()
        self.csv_path = os.path.join(self.temp_dir, "test_targets.csv")
        
        # Create test data
        with open(self.csv_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['scenario_id', 'source', 'target_type', 'target_value',
                           'lower_bound', 'upper_bound', 'units', 'notes'])
            writer.writerow(['SC001', 'NIST-Test', 'peak_HUL_temp', '650', '620', '680', 'K', 'Test scenario'])
            writer.writerow(['SC001', 'NIST-Test', 'peak_HRR', '500', '450', '550', 'kW', 'Test HRR'])
            writer.writerow(['SC002', 'ISO-Test', 'peak_HUL_temp', '800', '750', '850', 'K', 'Another scenario'])
    
    def tearDown(self):
        """Clean up temporary files."""
        if os.path.exists(self.csv_path):
            os.remove(self.csv_path)
        os.rmdir(self.temp_dir)
    
    def test_10A1_load_csv(self):
        """Test 10A1: Load validation targets from CSV."""
        dataset = ValidationDataset(self.csv_path)
        
        scenarios = dataset.get_scenarios()
        self.assertEqual(len(scenarios), 2)
        self.assertIn('SC001', scenarios)
        self.assertIn('SC002', scenarios)
    
    def test_10A2_get_targets(self):
        """Test 10A2: Retrieve targets for specific scenario."""
        dataset = ValidationDataset(self.csv_path)
        
        targets = dataset.get_targets('SC001')
        self.assertEqual(len(targets), 2)
        
        # Check first target
        target_type, target_val, lower, upper, units = targets[0]
        self.assertEqual(target_type, 'peak_HUL_temp')
        self.assertEqual(target_val, 650.0)
        self.assertEqual(lower, 620.0)
        self.assertEqual(upper, 680.0)
        self.assertEqual(units, 'K')
    
    def test_10A3_check_prediction_pass(self):
        """Test 10A3: Check prediction within bounds (PASS)."""
        dataset = ValidationDataset(self.csv_path)
        
        # Prediction within bounds
        result = dataset.check_prediction('SC001', 'peak_HUL_temp', 650.0)
        self.assertTrue(result)
        
        # Edge cases
        result = dataset.check_prediction('SC001', 'peak_HUL_temp', 620.0)
        self.assertTrue(result)
        
        result = dataset.check_prediction('SC001', 'peak_HUL_temp', 680.0)
        self.assertTrue(result)
    
    def test_10A4_check_prediction_fail(self):
        """Test 10A4: Check prediction outside bounds (FAIL)."""
        dataset = ValidationDataset(self.csv_path)
        
        # Too low
        result = dataset.check_prediction('SC001', 'peak_HUL_temp', 600.0)
        self.assertFalse(result)
        
        # Too high
        result = dataset.check_prediction('SC001', 'peak_HUL_temp', 700.0)
        self.assertFalse(result)
    
    def test_10A5_get_error(self):
        """Test 10A5: Calculate error metrics."""
        dataset = ValidationDataset(self.csv_path)
        
        error = dataset.get_error('SC001', 'peak_HUL_temp', 670.0)
        
        self.assertIsNotNone(error)
        self.assertAlmostEqual(error['absolute_error'], 20.0, places=2)
        self.assertAlmostEqual(error['relative_error'], 20.0/650.0, places=4)
        self.assertTrue(error['within_bounds'])
        self.assertEqual(error['units'], 'K')


class TestScenarioLibrary(unittest.TestCase):
    """Test ScenarioLibrary loading and querying."""
    
    def setUp(self):
        """Create temporary CSV file for testing."""
        self.temp_dir = tempfile.mkdtemp()
        self.csv_path = os.path.join(self.temp_dir, "test_scenarios.csv")
        
        # Create test data
        with open(self.csv_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['scenario_id', 'description', 'volume_m3', 'fire_load_MJ',
                           'ACH', 'ignition_HRR_kW', 'peak_HRR_kW', 'suppression_time_s',
                           'wall_material', 'validation_class'])
            writer.writerow(['SC001', 'Test scenario 1', '50', '800', '2.0', '50', '500',
                           'none', 'concrete', 'thermal_buildup'])
            writer.writerow(['SC002', 'Test scenario 2', '120', '1500', '3.5', '100', '1200',
                           '180', 'gypsum', 'suppression_effectiveness'])
    
    def tearDown(self):
        """Clean up temporary files."""
        if os.path.exists(self.csv_path):
            os.remove(self.csv_path)
        os.rmdir(self.temp_dir)
    
    def test_10B1_load_scenarios(self):
        """Test 10B1: Load scenario library from CSV."""
        library = ScenarioLibrary(self.csv_path)
        
        scenarios = library.get_all_scenarios()
        self.assertEqual(len(scenarios), 2)
    
    def test_10B2_get_scenario(self):
        """Test 10B2: Retrieve specific scenario."""
        library = ScenarioLibrary(self.csv_path)
        
        scenario = library.get_scenario('SC001')
        
        self.assertIsNotNone(scenario)
        self.assertEqual(scenario['scenario_id'], 'SC001')
        self.assertEqual(scenario['volume_m3'], 50.0)
        self.assertEqual(scenario['fire_load_MJ'], 800.0)
        self.assertEqual(scenario['ACH'], 2.0)
        self.assertEqual(scenario['ignition_HRR_kW'], 50.0)
        self.assertEqual(scenario['peak_HRR_kW'], 500.0)
        self.assertIsNone(scenario['suppression_time_s'])
        self.assertEqual(scenario['wall_material'], 'concrete')
        self.assertEqual(scenario['validation_class'], 'thermal_buildup')
    
    def test_10B3_get_scenarios_by_class(self):
        """Test 10B3: Filter scenarios by validation class."""
        library = ScenarioLibrary(self.csv_path)
        
        thermal = library.get_scenarios_by_class('thermal_buildup')
        self.assertEqual(len(thermal), 1)
        self.assertEqual(thermal[0]['scenario_id'], 'SC001')
        
        suppression = library.get_scenarios_by_class('suppression_effectiveness')
        self.assertEqual(len(suppression), 1)
        self.assertEqual(suppression[0]['scenario_id'], 'SC002')
    
    def test_10B4_get_validation_classes(self):
        """Test 10B4: Get unique validation classes."""
        library = ScenarioLibrary(self.csv_path)
        
        classes = library.get_validation_classes()
        self.assertEqual(len(classes), 2)
        self.assertIn('thermal_buildup', classes)
        self.assertIn('suppression_effectiveness', classes)


if __name__ == '__main__':
    # Run tests
    unittest.main(verbosity=2)
