"""
State machine definitions for SPECTER.
See docs/ARCHITECTURE.md section 5 for full transition table.
"""
from enum import Enum


class SystemState(str, Enum):
    COLD_BOOT    = "COLD_BOOT"
    BOOTSTRAP    = "BOOTSTRAP"
    COMPENSATING = "COMPENSATING"
    RE_BOOTSTRAP = "RE_BOOTSTRAP"
    SENSOR_SWAP  = "SENSOR_SWAP"
    ERROR        = "ERROR"
