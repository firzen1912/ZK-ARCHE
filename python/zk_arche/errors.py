from __future__ import annotations

from enum import IntEnum


class ErrorCode(IntEnum):
    UnsupportedVersion = 0x0101
    UnsupportedSuite = 0x0102
    CapabilityMismatch = 0x0103

    MalformedPacket = 0x0201
    UnknownPacketType = 0x0202
    PayloadTooLarge = 0x0203
    PayloadTooShort = 0x0204
    InvalidEncoding = 0x0205

    InvalidPoint = 0x0301
    NonCanonicalScalar = 0x0302
    IdentityPoint = 0x0303
    ProofVerifyFailed = 0x0304
    KeyConfirmFailed = 0x0305
    PeerKeyMismatch = 0x0306

    UnknownSession = 0x0401
    SessionExpired = 0x0402
    ReplayDetected = 0x0403
    SequenceOutOfOrder = 0x0404

    UnknownDevice = 0x0501
    DeviceNotEnrolled = 0x0502
    RoleNotPermitted = 0x0503
    PairingTokenInvalid = 0x0504

    RateLimited = 0x0601
    ServerBusy = 0x0602
    TooManyActive = 0x0603

    StorageFailure = 0x0701
    CredentialMissing = 0x0702
    RegistryCorrupt = 0x0703

    Unspecified = 0x7FFF

    @classmethod
    def from_u16(cls, code: int) -> "ErrorCode":
        try:
            return cls(code)
        except ValueError:
            return cls.Unspecified


class ProtoError(Exception):
    """Top-level protocol error with an optional wire-transmittable code."""

    def __init__(self, kind: str, msg: str, code: ErrorCode | None = None):
        self.kind = kind
        self.msg = msg
        self.code = code
        super().__init__(str(self))

    @classmethod
    def wire(cls, code: ErrorCode, msg: str) -> "ProtoError":
        return cls("wire", msg, code)

    @classmethod
    def transport(cls, msg: str) -> "ProtoError":
        return cls("transport", msg)

    @classmethod
    def storage(cls, msg: str) -> "ProtoError":
        return cls("storage", msg, ErrorCode.StorageFailure)

    @classmethod
    def internal(cls, msg: str) -> "ProtoError":
        return cls("internal", msg, ErrorCode.Unspecified)

    def wire_code(self) -> ErrorCode | None:
        return self.code if self.kind == "wire" else None

    def to_wire_payload(self) -> bytes:
        code = self.code or (ErrorCode.StorageFailure if self.kind == "storage" else ErrorCode.Unspecified)
        return int(code).to_bytes(2, "little") + self.msg.encode("utf-8", "replace")

    @classmethod
    def from_wire_payload(cls, data: bytes) -> "ProtoError":
        if len(data) < 2:
            return cls.wire(ErrorCode.MalformedPacket, "error payload too short")
        code = ErrorCode.from_u16(int.from_bytes(data[:2], "little"))
        msg = data[2:].decode("utf-8", "replace")
        return cls.wire(code, msg)

    def __str__(self) -> str:
        if self.kind == "wire" and self.code is not None:
            return f"wire error 0x{int(self.code):04x}: {self.msg}"
        return f"{self.kind} error: {self.msg}"
