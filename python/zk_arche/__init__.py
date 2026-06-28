"""ZK-ARCHE v2 Python reference implementation."""

from .caps import PROTOCOL_VERSION, MIN_SUPPORTED_VERSION, SUITE_RISTRETTO255_SHA256
from .errors import ErrorCode, ProtoError
from .profile import Profile, ProfileKind

DEFAULT_ALLOWED_ROLES = (1, 2)

__all__ = [
    "PROTOCOL_VERSION",
    "MIN_SUPPORTED_VERSION",
    "SUITE_RISTRETTO255_SHA256",
    "ErrorCode",
    "ProtoError",
    "Profile",
    "ProfileKind",
    "DEFAULT_ALLOWED_ROLES",
]
