"""
pytest fixtures shared across all SPECTER tests.
See TASKS.md C-00-C for what these fixtures must contain.
"""
import numpy as np
import pytest
from specter.config_loader import load_config, SpectreConfig


@pytest.fixture(scope="session")
def mock_config() -> SpectreConfig:
    """Load config from config/specter_config.yaml."""
    return load_config("config/specter_config.yaml")


@pytest.fixture(scope="session")
def synthetic_sensor_signal() -> tuple[np.ndarray, int]:
    """
    Generate a synthetic sensor signal for testing.

    Model: y = x + 0.3*x² + noise
    where x ~ N(0,1) is broadband Gaussian at 3200 Hz for 10 seconds.
    Noise is low-amplitude additive Gaussian (scale=0.05).

    Returns:
        (signal_array, sample_rate_hz): tuple of float64 array and sample rate
    """
    rng = np.random.default_rng(seed=42)
    fs = 3200          # sample rate, Hz — driven by INA219 max rate
    duration_s = 10    # 10 seconds as specified in TASKS.md C-00-C
    N = fs * duration_s
    x = rng.standard_normal(N)
    noise = 0.05 * rng.standard_normal(N)
    y = x + 0.3 * x**2 + noise
    return (y.astype(np.float64), fs)


@pytest.fixture(scope="session")
def linear_sensor_signal() -> tuple[np.ndarray, int]:
    """
    Linear sensor signal — bispectrum should be near zero.
    Model: y = x + 0.05*noise  (purely linear, Gaussian input)
    """
    rng = np.random.default_rng(seed=99)
    fs = 3200
    N = fs * 20
    x = rng.standard_normal(N)
    noise = 0.05 * rng.standard_normal(N)
    y = x + noise
    return (y.astype(np.float64), fs)
