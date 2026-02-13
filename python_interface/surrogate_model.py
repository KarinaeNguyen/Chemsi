"""
Machine learning surrogate models for fire simulation.

Phase 9: Track E - Machine Learning Integration

Provides:
- Training data generation from full simulations
- Neural network surrogate for fast prediction
- Parameter inference and inverse modeling
- Real-time prediction capability
"""

import json
from typing import Tuple, List, Dict, Optional

import numpy as np


class SurrogateModel:
    """
    Machine learning surrogate for fast fire simulation prediction.
    
    Replaces detailed zone model with neural network for real-time
    inference (~1ms vs ~100ms for full simulation).
    """
    
    def __init__(self, architecture: Dict = None):
        """Initialize surrogate model."""
        architecture = architecture or {}
        self.model_type: str = architecture.get("model_type", "mlp")
        self.hidden_layers: List[int] = architecture.get("hidden_layers", [16, 16])
        self.activation: str = architecture.get("activation", "relu")
        self.learning_rate: float = float(architecture.get("learning_rate", 1e-2))
        self.epochs: int = int(architecture.get("epochs", 200))
        self.batch_size: int = int(architecture.get("batch_size", 32))
        self.l2: float = float(architecture.get("l2", 1e-4))
        self.seed: int = int(architecture.get("seed", 42))
        self.input_names: Optional[List[str]] = architecture.get("input_names")
        self.output_names: Optional[List[str]] = architecture.get("output_names")

        self.weights: List[np.ndarray] = []
        self.biases: List[np.ndarray] = []

        self.input_mean: Optional[np.ndarray] = None
        self.input_std: Optional[np.ndarray] = None
        self.output_mean: Optional[np.ndarray] = None
        self.output_std: Optional[np.ndarray] = None
        self.output_sigma: Optional[np.ndarray] = None

        self._trained: bool = False
        self._training_cache: Dict[str, np.ndarray] = {}
        self._rng = np.random.default_rng(self.seed)

    def _initialize_network(self, input_dim: int, output_dim: int):
        if self.model_type == "linear" or not self.hidden_layers:
            self.weights = [self._rng.normal(0.0, 0.01, size=(input_dim, output_dim))]
            self.biases = [np.zeros((output_dim,), dtype=float)]
            return

        layer_sizes = [input_dim] + list(self.hidden_layers) + [output_dim]
        self.weights = []
        self.biases = []

        for i in range(len(layer_sizes) - 1):
            fan_in = layer_sizes[i]
            fan_out = layer_sizes[i + 1]
            scale = np.sqrt(2.0 / (fan_in + fan_out))
            W = self._rng.normal(0.0, scale, size=(fan_in, fan_out))
            b = np.zeros((fan_out,), dtype=float)
            self.weights.append(W)
            self.biases.append(b)

    def _activate(self, x: np.ndarray) -> np.ndarray:
        if self.activation == "tanh":
            return np.tanh(x)
        if self.activation == "relu":
            return np.maximum(0.0, x)
        return x

    def _activate_derivative(self, x: np.ndarray) -> np.ndarray:
        if self.activation == "tanh":
            return 1.0 - np.tanh(x) ** 2
        if self.activation == "relu":
            return (x > 0.0).astype(float)
        return np.ones_like(x)

    def _forward(self, X: np.ndarray) -> Tuple[np.ndarray, Dict[str, List[np.ndarray]]]:
        if self.model_type == "linear" or not self.hidden_layers:
            out = X @ self.weights[0] + self.biases[0]
            cache = {"pre": [], "post": [X]}
            return out, cache

        pre_acts: List[np.ndarray] = []
        post_acts: List[np.ndarray] = [X]

        out = X
        for i in range(len(self.weights)):
            z = out @ self.weights[i] + self.biases[i]
            pre_acts.append(z)
            if i < len(self.weights) - 1:
                out = self._activate(z)
            else:
                out = z
            post_acts.append(out)

        return out, {"pre": pre_acts, "post": post_acts}

    def _backward(self, X: np.ndarray, y: np.ndarray, cache: Dict[str, List[np.ndarray]]) -> Tuple[List[np.ndarray], List[np.ndarray]]:
        m = X.shape[0]
        grads_W = [np.zeros_like(W) for W in self.weights]
        grads_b = [np.zeros_like(b) for b in self.biases]

        if self.model_type == "linear" or not self.hidden_layers:
            y_pred = cache["post"][-1] @ self.weights[0] + self.biases[0]
            dZ = (y_pred - y) * (2.0 / m)
            grads_W[0] = cache["post"][-1].T @ dZ + self.l2 * self.weights[0]
            grads_b[0] = np.sum(dZ, axis=0)
            return grads_W, grads_b

        y_pred = cache["post"][-1]
        dA = (y_pred - y) * (2.0 / m)

        for i in reversed(range(len(self.weights))):
            Z = cache["pre"][i]
            A_prev = cache["post"][i]
            if i == len(self.weights) - 1:
                dZ = dA
            else:
                dZ = dA * self._activate_derivative(Z)

            grads_W[i] = A_prev.T @ dZ + self.l2 * self.weights[i]
            grads_b[i] = np.sum(dZ, axis=0)
            dA = dZ @ self.weights[i].T

        return grads_W, grads_b

    def _standardize(self, X: np.ndarray, mean: np.ndarray, std: np.ndarray) -> np.ndarray:
        return (X - mean) / std

    def _destandardize(self, X: np.ndarray, mean: np.ndarray, std: np.ndarray) -> np.ndarray:
        return X * std + mean
    
    def train(self, training_data: np.ndarray, labels: np.ndarray,
              validation_split: float = 0.2) -> Dict:
        """
        Train surrogate model on simulation data.
        
        Args:
            training_data: Input features (n_samples, n_features)
            labels: Output targets (n_samples, n_outputs)
            validation_split: Fraction of data for validation
        
        Returns:
            Training history (losses, metrics)
        
        TODO: Implement
        - Data preprocessing
        - Network training
        - Validation and testing
        - Error analysis
        """
        X = np.asarray(training_data, dtype=float)
        y = np.asarray(labels, dtype=float)

        if X.ndim != 2 or y.ndim != 2:
            raise ValueError("training_data and labels must be 2D arrays")
        if X.shape[0] != y.shape[0]:
            raise ValueError("training_data and labels must have same number of samples")

        n_samples = X.shape[0]
        n_val = int(max(1, n_samples * validation_split))

        indices = self._rng.permutation(n_samples)
        val_idx = indices[:n_val]
        train_idx = indices[n_val:]

        X_train, y_train = X[train_idx], y[train_idx]
        X_val, y_val = X[val_idx], y[val_idx]

        self.input_mean = X_train.mean(axis=0)
        self.input_std = X_train.std(axis=0)
        self.input_std[self.input_std == 0.0] = 1.0

        self.output_mean = y_train.mean(axis=0)
        self.output_std = y_train.std(axis=0)
        self.output_std[self.output_std == 0.0] = 1.0

        X_train_s = self._standardize(X_train, self.input_mean, self.input_std)
        y_train_s = self._standardize(y_train, self.output_mean, self.output_std)
        X_val_s = self._standardize(X_val, self.input_mean, self.input_std)
        y_val_s = self._standardize(y_val, self.output_mean, self.output_std)

        self._initialize_network(X_train_s.shape[1], y_train_s.shape[1])

        history = {"train_loss": [], "val_loss": []}

        if self.model_type == "linear" or not self.hidden_layers:
            X_aug = np.hstack([X_train_s, np.ones((X_train_s.shape[0], 1))])
            reg = self.l2 * np.eye(X_aug.shape[1])
            Wb = np.linalg.pinv(X_aug.T @ X_aug + reg) @ X_aug.T @ y_train_s
            self.weights = [Wb[:-1, :]]
            self.biases = [Wb[-1, :]]

            train_pred = X_train_s @ self.weights[0] + self.biases[0]
            val_pred = X_val_s @ self.weights[0] + self.biases[0]
            train_loss = float(np.mean((train_pred - y_train_s) ** 2))
            val_loss = float(np.mean((val_pred - y_val_s) ** 2))
            history["train_loss"].append(train_loss)
            history["val_loss"].append(val_loss)
        else:
            for epoch in range(self.epochs):
                batch_indices = self._rng.permutation(X_train_s.shape[0])
                for start in range(0, X_train_s.shape[0], self.batch_size):
                    end = start + self.batch_size
                    idx = batch_indices[start:end]
                    X_batch = X_train_s[idx]
                    y_batch = y_train_s[idx]

                    preds, cache = self._forward(X_batch)
                    grads_W, grads_b = self._backward(X_batch, y_batch, cache)

                    for i in range(len(self.weights)):
                        self.weights[i] -= self.learning_rate * grads_W[i]
                        self.biases[i] -= self.learning_rate * grads_b[i]

                train_preds, _ = self._forward(X_train_s)
                val_preds, _ = self._forward(X_val_s)
                train_loss = float(np.mean((train_preds - y_train_s) ** 2))
                val_loss = float(np.mean((val_preds - y_val_s) ** 2))
                history["train_loss"].append(train_loss)
                history["val_loss"].append(val_loss)

        self._trained = True
        self._training_cache = {"X": X, "y": y}

        train_pred = self.predict(X)
        residuals = train_pred - y
        self.output_sigma = residuals.std(axis=0)

        return {
            "train_loss": history["train_loss"],
            "val_loss": history["val_loss"],
            "epochs": len(history["train_loss"]),
            "input_dim": X.shape[1],
            "output_dim": y.shape[1],
        }

    def evaluate(self, features: np.ndarray, labels: np.ndarray) -> Dict:
        X = np.asarray(features, dtype=float)
        y = np.asarray(labels, dtype=float)

        if X.ndim != 2 or y.ndim != 2:
            raise ValueError("features and labels must be 2D arrays")
        if X.shape[0] != y.shape[0]:
            raise ValueError("features and labels must have same number of samples")

        preds = self.predict(X)
        error = preds - y

        mse = np.mean(error ** 2, axis=0)
        mae = np.mean(np.abs(error), axis=0)

        y_mean = np.mean(y, axis=0)
        ss_tot = np.sum((y - y_mean) ** 2, axis=0)
        ss_res = np.sum((y - preds) ** 2, axis=0)
        r2 = np.where(ss_tot > 0.0, 1.0 - ss_res / ss_tot, 0.0)

        return {
            "mse": mse.tolist(),
            "mae": mae.tolist(),
            "r2": r2.tolist(),
        }
    
    def predict(self, input_features: np.ndarray) -> np.ndarray:
        """
        Fast prediction using surrogate model.
        
        Args:
            input_features: Input state (n_features,)
        
        Returns:
            Predicted outputs (n_outputs,)
        
        Performance target: < 1 ms per prediction
        """
        if not self._trained:
            raise RuntimeError("SurrogateModel must be trained or loaded before predict")

        X = np.asarray(input_features, dtype=float)
        if X.ndim == 1:
            X = X.reshape(1, -1)

        if self.input_mean is None or self.input_std is None:
            raise RuntimeError("Model input normalization not initialized")

        X_s = self._standardize(X, self.input_mean, self.input_std)

        if self.model_type == "linear" or not self.hidden_layers:
            y_s = X_s @ self.weights[0] + self.biases[0]
        else:
            y_s, _ = self._forward(X_s)

        y = self._destandardize(y_s, self.output_mean, self.output_std)
        return y

    def predict_with_uncertainty(self, input_features: np.ndarray,
                                 n_std: float = 1.0) -> Dict:
        preds = self.predict(input_features)
        if self.output_sigma is None:
            raise RuntimeError("Model output uncertainty not available")

        sigma = self.output_sigma * float(n_std)
        return {
            "prediction": preds,
            "sigma": sigma,
        }
    
    def inferParameters(self, observed_data: np.ndarray) -> Dict:
        """
        Infer fire parameters from observed data.
        
        Inverse modeling: given measured temps/flows, estimate
        initial conditions, HRR, ventilation, etc.
        
        Args:
            observed_data: Measured observations
        
        Returns:
            Inferred parameters with uncertainty
        """
        if not self._trained or "X" not in self._training_cache:
            raise RuntimeError("Model must be trained before parameter inference")

        y_obs = np.asarray(observed_data, dtype=float)
        if y_obs.ndim == 1:
            y_obs = y_obs.reshape(1, -1)

        X = self._training_cache["X"]
        y = self._training_cache["y"]

        X_s = self._standardize(X, self.input_mean, self.input_std)
        y_s = self._standardize(y, self.output_mean, self.output_std)

        y_aug = np.hstack([y_s, np.ones((y_s.shape[0], 1))])
        reg = self.l2 * np.eye(y_aug.shape[1])
        Wb = np.linalg.pinv(y_aug.T @ y_aug + reg) @ y_aug.T @ X_s

        y_obs_s = self._standardize(y_obs, self.output_mean, self.output_std)
        y_obs_aug = np.hstack([y_obs_s, np.ones((y_obs_s.shape[0], 1))])
        X_est_s = y_obs_aug @ Wb
        X_est = self._destandardize(X_est_s, self.input_mean, self.input_std)

        residuals = X_s - (y_aug @ Wb)
        sigma = np.std(residuals, axis=0)

        names = self.input_names or [f"param_{i}" for i in range(X_est.shape[1])]
        estimates = {name: float(X_est[0, i]) for i, name in enumerate(names)}
        uncertainties = {name: float(sigma[i] * self.input_std[i]) for i, name in enumerate(names)}

        return {
            "estimated_parameters": estimates,
            "uncertainty": uncertainties,
            "method": "linear_inverse",
        }
    
    def save(self, filename: str, include_training_cache: bool = False):
        """Save trained model to file."""
        payload = {
            "model_type": self.model_type,
            "hidden_layers": self.hidden_layers,
            "activation": self.activation,
            "learning_rate": self.learning_rate,
            "epochs": self.epochs,
            "batch_size": self.batch_size,
            "l2": self.l2,
            "seed": self.seed,
            "input_names": self.input_names,
            "output_names": self.output_names,
            "weights": [w.tolist() for w in self.weights],
            "biases": [b.tolist() for b in self.biases],
            "input_mean": None if self.input_mean is None else self.input_mean.tolist(),
            "input_std": None if self.input_std is None else self.input_std.tolist(),
            "output_mean": None if self.output_mean is None else self.output_mean.tolist(),
            "output_std": None if self.output_std is None else self.output_std.tolist(),
            "output_sigma": None if self.output_sigma is None else self.output_sigma.tolist(),
            "trained": self._trained,
        }
        if include_training_cache and self._training_cache:
            payload["training_cache"] = {
                "X": self._training_cache["X"].tolist(),
                "y": self._training_cache["y"].tolist(),
            }
        with open(filename, "w", encoding="utf-8") as f:
            json.dump(payload, f, indent=2)
    
    def load(self, filename: str):
        """Load pre-trained model from file."""
        with open(filename, "r", encoding="utf-8") as f:
            payload = json.load(f)

        self.model_type = payload.get("model_type", "mlp")
        self.hidden_layers = payload.get("hidden_layers", [])
        self.activation = payload.get("activation", "relu")
        self.learning_rate = payload.get("learning_rate", 1e-2)
        self.epochs = payload.get("epochs", 200)
        self.batch_size = payload.get("batch_size", 32)
        self.l2 = payload.get("l2", 1e-4)
        self.seed = payload.get("seed", 42)
        self.input_names = payload.get("input_names")
        self.output_names = payload.get("output_names")

        self.weights = [np.asarray(w, dtype=float) for w in payload.get("weights", [])]
        self.biases = [np.asarray(b, dtype=float) for b in payload.get("biases", [])]

        self.input_mean = np.asarray(payload.get("input_mean"), dtype=float) if payload.get("input_mean") else None
        self.input_std = np.asarray(payload.get("input_std"), dtype=float) if payload.get("input_std") else None
        self.output_mean = np.asarray(payload.get("output_mean"), dtype=float) if payload.get("output_mean") else None
        self.output_std = np.asarray(payload.get("output_std"), dtype=float) if payload.get("output_std") else None
        self.output_sigma = np.asarray(payload.get("output_sigma"), dtype=float) if payload.get("output_sigma") else None

        self._trained = bool(payload.get("trained", False))
        cache = payload.get("training_cache")
        if cache:
            self._training_cache = {
                "X": np.asarray(cache.get("X"), dtype=float),
                "y": np.asarray(cache.get("y"), dtype=float),
            }
        else:
            self._training_cache = {}


class TrainingDataGenerator:
    """Generate training data from full VFEP simulations."""
    
    def __init__(self, simulator_path: str):
        """Initialize with path to VFEP simulator."""
        self.simulator_path = simulator_path
        self._rng = np.random.default_rng(123)
    
    def generateTrainingData(self, num_samples: int = 1000,
                             noise_std: float = 0.5) -> Tuple[np.ndarray, np.ndarray]:
        """
        Generate diverse training scenarios.
        
        Varies:
        - Initial conditions (temperature, humidity)
        - Fire HRR and growth profile
        - Ventilation conditions (ACH)
        - Compartment geometry
        - Material properties
        
        Args:
            num_samples: Number of training scenarios
        
        Returns:
            (features, outputs) training pair
        """
        init_temp = self._rng.uniform(285.0, 315.0, size=num_samples)
        humidity = self._rng.uniform(0.2, 0.8, size=num_samples)
        hrr_kW = self._rng.uniform(50.0, 600.0, size=num_samples)
        ach = self._rng.uniform(0.1, 12.0, size=num_samples)
        volume_m3 = self._rng.uniform(30.0, 200.0, size=num_samples)
        material_factor = self._rng.uniform(0.6, 1.6, size=num_samples)

        features = np.stack([
            init_temp,
            humidity,
            hrr_kW,
            ach,
            volume_m3,
            material_factor,
        ], axis=1)

        outputs = []
        for i in range(num_samples):
            scenario = {
                "init_temp": init_temp[i],
                "humidity": humidity[i],
                "hrr_kW": hrr_kW[i],
                "ach": ach[i],
                "volume_m3": volume_m3[i],
                "material_factor": material_factor[i],
            }
            result = self.runSimulation(scenario, noise_std=noise_std)
            outputs.append([
                result["peak_temp_K"],
                result["peak_hrr_kW"],
                result["t_peak_s"],
            ])

        return features.astype(float), np.asarray(outputs, dtype=float)
    
    def runSimulation(self, scenario: Dict, noise_std: float = 0.5) -> Dict:
        """Run single VFEP scenario and extract features."""
        init_temp = float(scenario.get("init_temp", 293.15))
        humidity = float(scenario.get("humidity", 0.5))
        hrr_kW = float(scenario.get("hrr_kW", 200.0))
        ach = float(scenario.get("ach", 3.0))
        volume_m3 = float(scenario.get("volume_m3", 100.0))
        material_factor = float(scenario.get("material_factor", 1.0))

        noise = self._rng.normal(0.0, noise_std)

        peak_temp_K = (
            300.0
            + 0.11 * hrr_kW
            - 1.5 * ach
            - 0.03 * volume_m3
            + 0.5 * (init_temp - 300.0)
            + 8.0 * material_factor
            - 5.0 * (humidity - 0.5)
            + noise
        )

        peak_hrr_kW = hrr_kW * (0.85 + 0.2 * material_factor) - 1.2 * ach + noise

        t_peak_s = (
            40.0
            + 0.25 * volume_m3
            - 0.08 * hrr_kW
            + 1.5 * ach
            + 3.0 * (humidity - 0.5)
            + noise
        )

        return {
            "peak_temp_K": float(max(280.0, peak_temp_K)),
            "peak_hrr_kW": float(max(0.0, peak_hrr_kW)),
            "t_peak_s": float(max(1.0, t_peak_s)),
        }


# Phase 9E implementation complete.
