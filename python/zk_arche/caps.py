from __future__ import annotations

from dataclasses import dataclass

from .errors import ErrorCode, ProtoError

PROTOCOL_VERSION = 0x02
MIN_SUPPORTED_VERSION = 0x02

SuiteId = int
SUITE_RISTRETTO255_SHA256: SuiteId = 0x0001


class cap:
    AUTH_V2 = 1 << 0
    ROLE_RERAND = 1 << 1
    ROLE_SET_MEMBERSHIP = 1 << 2
    PAIRING_TOKEN = 1 << 3
    TOFU_SETUP = 1 << 4
    PROFILE_MINIMAL = 1 << 8
    PROFILE_STANDARD = 1 << 9
    PROFILE_GATEWAY = 1 << 10
    CBOR_FRAMING = 1 << 16

    BASELINE = AUTH_V2 | ROLE_RERAND | ROLE_SET_MEMBERSHIP | PROFILE_STANDARD


def local_capabilities() -> int:
    return cap.BASELINE | cap.PAIRING_TOKEN


@dataclass(frozen=True)
class Negotiated:
    version: int
    suite: SuiteId
    caps: int


def negotiate(
    local_version: int,
    local_min_version: int,
    local_suites: list[SuiteId] | tuple[SuiteId, ...],
    local_caps: int,
    peer_version: int,
    peer_min_version: int,
    peer_suites: list[SuiteId] | tuple[SuiteId, ...],
    peer_caps: int,
) -> Negotiated:
    version = min(local_version, peer_version)
    if version < local_min_version or version < peer_min_version:
        raise ProtoError.wire(ErrorCode.UnsupportedVersion, "no mutually-supported protocol version")
    suite = next((s for s in local_suites if s in peer_suites), None)
    if suite is None:
        raise ProtoError.wire(ErrorCode.UnsupportedSuite, "no mutually-supported cipher suite")
    caps = local_caps & peer_caps
    if caps & cap.BASELINE != cap.BASELINE:
        raise ProtoError.wire(ErrorCode.CapabilityMismatch, "baseline capabilities not mutually supported")
    return Negotiated(version, suite, caps)
