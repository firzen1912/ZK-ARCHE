from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


DEFAULT_RETRANSMIT_TIMEOUT = 0.800
DEFAULT_MAX_RETRIES = 4
DEFAULT_MAX_BACKOFF_SHIFT = 3
DEFAULT_IO_TIMEOUT = 5.0
DEFAULT_SESSION_TTL = 15.0


class ProfileKind(Enum):
    Minimal = "minimal"
    Standard = "standard"
    Gateway = "gateway"


@dataclass(frozen=True)
class Profile:
    kind: ProfileKind = ProfileKind.Standard
    retransmit_timeout: float = DEFAULT_RETRANSMIT_TIMEOUT
    max_retries: int = DEFAULT_MAX_RETRIES
    max_backoff_shift: int = DEFAULT_MAX_BACKOFF_SHIFT
    io_timeout: float = DEFAULT_IO_TIMEOUT
    session_ttl: float = DEFAULT_SESSION_TTL
    max_active_sessions: int = 1024
    max_cached_responses: int = 2048

    @classmethod
    def standard(cls) -> "Profile":
        return cls()

    @classmethod
    def minimal(cls) -> "Profile":
        return cls(kind=ProfileKind.Minimal, max_retries=2, max_active_sessions=8, max_cached_responses=16)

    @classmethod
    def gateway(cls) -> "Profile":
        return cls(kind=ProfileKind.Gateway, max_active_sessions=8192, max_cached_responses=16384)
