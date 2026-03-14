"""
tests/synthetic/test_scaffold.py — C-00 smoke tests.

These tests verify the scaffold is correctly built before any DSP logic
is implemented. They run on the stub modules and must all pass at C-00.
Real algorithm tests are added in C-07 through C-11.
"""
import numpy as np
import pytest
from specter.config_loader import SpectreConfig


# ---------------------------------------------------------------------------
# Fixture smoke tests
# ---------------------------------------------------------------------------

def test_mock_config_loads(mock_config: SpectreConfig) -> None:
    """
    Verify mock_config fixture loads and returns a valid SpectreConfig.

    Args:
        mock_config: pytest fixture from conftest.py
    Returns:
        None
    Raises:
        AssertionError: if config fields are not populated correctly
    """
    assert isinstance(mock_config, SpectreConfig)
    # Spot-check mandatory fields that must match the YAML template
    assert mock_config.voltage_sample_rate_hz == 10000
    assert mock_config.current_sample_rate_hz == 3200
    assert mock_config.bispectrum_fft_size == 512
    assert mock_config.bispectrum_frames_M == 100
    assert mock_config.ekf_state_dim == 80
    assert mock_config.volterra_memory_taps_linear == 64
    assert mock_config.volterra_memory_taps_quadratic == 16


def test_synthetic_signal_shape(synthetic_sensor_signal: tuple[np.ndarray, int]) -> None:
    """
    Verify synthetic_sensor_signal fixture produces correct shape and content.

    Signal must be: y = x + 0.3*x² + noise, 3200 Hz, 10 seconds.

    Args:
        synthetic_sensor_signal: pytest fixture from conftest.py
    Returns:
        None
    Raises:
        AssertionError: if signal shape, dtype, or nonlinearity check fails
    """
    y, fs = synthetic_sensor_signal
    # 10 seconds at 3200 Hz = 32000 samples
    assert fs == 3200, f"Expected fs=3200, got {fs}"
    assert y.shape == (32_000,), f"Expected 32000 samples, got {y.shape}"
    assert y.dtype == np.float64, f"Expected float64, got {y.dtype}"
    # Nonlinearity check: skewness nonzero (x² term biases distribution)
    mean = float(np.mean(y))
    assert abs(mean) > 0.01, (
        "Mean of y = x + 0.3*x² + noise should be nonzero due to x² term "
        f"(got mean={mean:.4f})"
    )


def test_all_specter_modules_importable() -> None:
    """
    Verify all specter/ stub modules are importable without errors.

    This confirms C-00-B: each file has a module docstring and no syntax
    errors, even though logic is not yet implemented.

    Returns:
        None
    Raises:
        ImportError: if any module fails to import
    """
    import specter.acquire          # noqa: F401
    import specter.bootstrap        # noqa: F401
    import specter.compensate       # noqa: F401
    import specter.config_loader    # noqa: F401
    import specter.crossmodal       # noqa: F401
    import specter.forensics        # noqa: F401
    import specter.kalman           # noqa: F401
    import specter.shared_memory_layout  # noqa: F401
    import specter.state            # noqa: F401
    import specter.telemetry        # noqa: F401
    import specter.main             # noqa: F401
