import os
import tempfile
import unittest

import numpy as np

from surrogate_model import SurrogateModel, TrainingDataGenerator


class TestSurrogateModel(unittest.TestCase):
    def test_9E1_train_predict(self):
        generator = TrainingDataGenerator("dummy_simulator")
        X, y = generator.generateTrainingData(num_samples=400, noise_std=0.05)

        model = SurrogateModel({
            "model_type": "linear",
            "seed": 123,
            "input_names": [
                "init_temp",
                "humidity",
                "hrr_kW",
                "ach",
                "volume_m3",
                "material_factor",
            ],
            "output_names": ["peak_temp_K", "peak_hrr_kW", "t_peak_s"],
            "l2": 1e-6,
        })

        history = model.train(X, y, validation_split=0.2)
        self.assertIn("train_loss", history)
        self.assertGreaterEqual(len(history["train_loss"]), 1)

        preds = model.predict(X[:10])
        self.assertEqual(preds.shape, y[:10].shape)
        mse = float(np.mean((preds - y[:10]) ** 2))
        self.assertLess(mse, 25.0, "9E1: prediction MSE too high")

        metrics = model.evaluate(X[:20], y[:20])
        self.assertIn("mse", metrics)
        self.assertIn("mae", metrics)
        self.assertIn("r2", metrics)

        uncertainty = model.predict_with_uncertainty(X[:5], n_std=2.0)
        self.assertIn("prediction", uncertainty)
        self.assertIn("sigma", uncertainty)
        self.assertEqual(uncertainty["prediction"].shape, y[:5].shape)

    def test_9E2_save_load_infer(self):
        generator = TrainingDataGenerator("dummy_simulator")
        X, y = generator.generateTrainingData(num_samples=150)

        model = SurrogateModel({
            "model_type": "linear",
            "seed": 7,
            "input_names": [
                "init_temp",
                "humidity",
                "hrr_kW",
                "ach",
                "volume_m3",
                "material_factor",
            ],
            "output_names": ["peak_temp_K", "peak_hrr_kW", "t_peak_s"],
        })
        model.train(X, y, validation_split=0.2)

        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "surrogate_model.json")
            model.save(path, include_training_cache=True)

            loaded = SurrogateModel()
            loaded.load(path)

            preds_original = model.predict(X[:5])
            preds_loaded = loaded.predict(X[:5])

            self.assertTrue(
                np.allclose(preds_original, preds_loaded, atol=1e-6),
                "9E2: loaded model predictions mismatch",
            )

            inference = loaded.inferParameters(y[0])
            self.assertIn("estimated_parameters", inference)
            self.assertIn("uncertainty", inference)
            self.assertEqual(len(inference["estimated_parameters"]), X.shape[1])
            for value in inference["uncertainty"].values():
                self.assertGreaterEqual(value, 0.0)


if __name__ == "__main__":
    unittest.main()
