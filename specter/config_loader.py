"""
Loads config/specter_config.yaml into a typed SpectreConfig dataclass.
All tunables live in that YAML file — no hardcoded constants elsewhere.
"""
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import yaml


@dataclass(frozen=True)
class SpectreConfig:
    voltage_sample_rate_hz: int
    current_sample_rate_hz: int
    crossmodal_rate_hz: int
    bispectrum_fft_size: int
    bispectrum_frames_M: int
    bootstrap_timeout_s: int
    volterra_memory_taps_linear: int
    volterra_memory_taps_quadratic: int
    tikhonov_lambda_volterra: float
    wiener_lambda_floor: float
    ekf_state_dim: int
    ekf_Q_diagonal: float
    ekf_R_initial: float
    ekf_update_rate_hz: int
    cusum_slack_factor: float
    cusum_threshold_sigma: float
    r_load_ohms: float
    crossmodal_alpha: float
    crossmodal_beta: float
    crossmodal_gamma: float
    telemetry_rate_hz: int
    uart_baud_esp32: int
    teensy_usb_path: str
    pi_uart_esp32: str
    confidence_green_threshold: int
    confidence_amber_threshold: int


def load_config(path: str = "config/specter_config.yaml") -> SpectreConfig:
    """
    Load SPECTER configuration from YAML file.

    Args:
        path: path to specter_config.yaml (relative to project root)
    Returns:
        SpectreConfig frozen dataclass
    Raises:
        FileNotFoundError: if config file does not exist
        KeyError: if any required field is missing (shows the missing key name)
    """
    config_path = Path(path)
    if not config_path.exists():
        raise FileNotFoundError(f"Config file not found: {config_path.absolute()}")
    with open(config_path) as f:
        raw = yaml.safe_load(f)
    try:
        return SpectreConfig(**raw)
    except TypeError as e:
        raise KeyError(f"Config field error: {e}") from e
