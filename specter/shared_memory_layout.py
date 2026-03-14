"""
Canonical definitions for all shared memory regions in SPECTER.
All sizes, names, and dtypes are defined here. Never hardcode them elsewhere.
See docs/DATA_MODELS.md sections 2-6 for field-level detail.
"""
from multiprocessing.shared_memory import SharedMemory
import numpy as np

# Shared memory names
SHM_RING       = "specter_ring_buffer"
SHM_RING_CTRL  = "specter_ring_ctrl"
SHM_COEFF      = "specter_coefficients"
SHM_OUTPUT     = "specter_output"
SHM_TELEMETRY  = "specter_telemetry"
SHM_HEARTBEAT  = "specter_heartbeat"

# Ring buffer
RING_CAPACITY = 4096
RING_DTYPE = np.dtype([
    ('timestamp_us', np.uint32),
    ('voltage_raw',  np.int16),
    ('current_raw',  np.int16),
])
SHM_RING_SIZE = RING_CAPACITY * RING_DTYPE.itemsize  # 32768 bytes

CTRL_DTYPE = np.dtype([('write_idx', np.uint32), ('read_idx', np.uint32)])
SHM_RING_CTRL_SIZE = CTRL_DTYPE.itemsize  # 8 bytes

# Coefficients
COEFF_DIM = 80
SHM_COEFF_SIZE = COEFF_DIM * 8 + 1  # 641 bytes (80×float64 + lock flag)

# Output buffer
OUTPUT_CAPACITY = 256
OUTPUT_DTYPE = np.dtype([
    ('voltage_raw',       np.float32),
    ('voltage_corrected', np.float32),
    ('current_raw',       np.float32),
    ('current_corrected', np.float32),
])
SHM_OUTPUT_SIZE = OUTPUT_CAPACITY * OUTPUT_DTYPE.itemsize  # 4096 bytes

# Telemetry (JSON string)
SHM_TELEMETRY_SIZE = 2048  # bytes

# Heartbeat: [acq_ts, comp_ts, track_ts, svc_ts] as uint64 (Unix epoch ms)
SHM_HEARTBEAT_SIZE = 4 * 8  # 32 bytes


def create_all_shm() -> dict[str, SharedMemory]:
    """Create all shared memory regions. Called once by main.py at startup."""
    return {
        SHM_RING:      SharedMemory(name=SHM_RING,      create=True, size=SHM_RING_SIZE),
        SHM_RING_CTRL: SharedMemory(name=SHM_RING_CTRL, create=True, size=SHM_RING_CTRL_SIZE),
        SHM_COEFF:     SharedMemory(name=SHM_COEFF,     create=True, size=SHM_COEFF_SIZE),
        SHM_OUTPUT:    SharedMemory(name=SHM_OUTPUT,     create=True, size=SHM_OUTPUT_SIZE),
        SHM_TELEMETRY: SharedMemory(name=SHM_TELEMETRY,  create=True, size=SHM_TELEMETRY_SIZE),
        SHM_HEARTBEAT: SharedMemory(name=SHM_HEARTBEAT,  create=True, size=SHM_HEARTBEAT_SIZE),
    }


def attach_all_shm() -> dict[str, SharedMemory]:
    """Attach to existing shared memory regions. Called by worker processes."""
    return {
        SHM_RING:      SharedMemory(name=SHM_RING,      create=False),
        SHM_RING_CTRL: SharedMemory(name=SHM_RING_CTRL, create=False),
        SHM_COEFF:     SharedMemory(name=SHM_COEFF,     create=False),
        SHM_OUTPUT:    SharedMemory(name=SHM_OUTPUT,     create=False),
        SHM_TELEMETRY: SharedMemory(name=SHM_TELEMETRY,  create=False),
        SHM_HEARTBEAT: SharedMemory(name=SHM_HEARTBEAT,  create=False),
    }
