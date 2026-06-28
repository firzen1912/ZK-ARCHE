from __future__ import annotations

import ctypes
import ctypes.util
import os

from .errors import ErrorCode, ProtoError

BYTES = 32
SCALARBYTES = 32
HASHBYTES = 64
IDENTITY = b"\x00" * 32
SCALAR_ORDER = 2**252 + 27742317777372353535851937790883648493


class Sodium:
    def __init__(self) -> None:
        path = ctypes.util.find_library("sodium")
        if not path:
            raise ProtoError.internal("libsodium not found; install libsodium with Ristretto255 support")
        self.lib = ctypes.cdll.LoadLibrary(path)
        if self.lib.sodium_init() < 0:
            raise ProtoError.internal("libsodium initialization failed")
        self._bind()

    def _bind(self) -> None:
        L = self.lib
        # Functions returning int.
        for name, argc in [
            ("crypto_core_ristretto255_is_valid_point", 1),
            ("crypto_core_ristretto255_from_hash", 2),
            ("crypto_core_ristretto255_add", 3),
            ("crypto_core_ristretto255_sub", 3),
            ("crypto_scalarmult_ristretto255_base", 2),
            ("crypto_scalarmult_ristretto255", 3),
        ]:
            try:
                fn = getattr(L, name)
            except AttributeError as exc:
                raise ProtoError.internal(f"libsodium missing {name}") from exc
            fn.restype = ctypes.c_int
            fn.argtypes = [ctypes.c_void_p] * argc
        # Void scalar functions.
        for name, argc in [
            ("crypto_core_ristretto255_scalar_random", 1),
            ("crypto_core_ristretto255_scalar_reduce", 2),
            ("crypto_core_ristretto255_scalar_add", 3),
            ("crypto_core_ristretto255_scalar_sub", 3),
            ("crypto_core_ristretto255_scalar_mul", 3),
        ]:
            try:
                fn = getattr(L, name)
            except AttributeError as exc:
                raise ProtoError.internal(f"libsodium missing {name}") from exc
            fn.restype = None
            fn.argtypes = [ctypes.c_void_p] * argc

    @staticmethod
    def _inbuf(data: bytes, size: int | None = None) -> ctypes.Array:
        if size is not None and len(data) != size:
            raise ProtoError.internal(f"expected {size} bytes, got {len(data)}")
        return ctypes.create_string_buffer(data, len(data))

    @staticmethod
    def _out32() -> ctypes.Array:
        return ctypes.create_string_buffer(BYTES)

    def is_valid_point(self, point: bytes) -> bool:
        if len(point) != BYTES:
            return False
        return self.lib.crypto_core_ristretto255_is_valid_point(self._inbuf(point, BYTES)) == 1

    def from_hash(self, h: bytes) -> bytes:
        if len(h) != HASHBYTES:
            raise ProtoError.internal("Ristretto from_hash requires 64 bytes")
        out = self._out32()
        if self.lib.crypto_core_ristretto255_from_hash(out, self._inbuf(h, HASHBYTES)) != 0:
            raise ProtoError.internal("Ristretto from_hash failed")
        return out.raw

    def point_add(self, a: bytes, b: bytes) -> bytes:
        out = self._out32()
        if self.lib.crypto_core_ristretto255_add(out, self._inbuf(a, BYTES), self._inbuf(b, BYTES)) != 0:
            raise ProtoError.wire(ErrorCode.InvalidPoint, "point add failed")
        return out.raw

    def point_sub(self, a: bytes, b: bytes) -> bytes:
        out = self._out32()
        if self.lib.crypto_core_ristretto255_sub(out, self._inbuf(a, BYTES), self._inbuf(b, BYTES)) != 0:
            raise ProtoError.wire(ErrorCode.InvalidPoint, "point sub failed")
        return out.raw

    def scalarmult_base(self, scalar: bytes) -> bytes:
        out = self._out32()
        if self.lib.crypto_scalarmult_ristretto255_base(out, self._inbuf(scalar, SCALARBYTES)) != 0:
            raise ProtoError.wire(ErrorCode.NonCanonicalScalar, "base scalar multiplication failed")
        return out.raw

    def scalarmult(self, scalar: bytes, point: bytes) -> bytes:
        out = self._out32()
        if self.lib.crypto_scalarmult_ristretto255(out, self._inbuf(scalar, SCALARBYTES), self._inbuf(point, BYTES)) != 0:
            raise ProtoError.wire(ErrorCode.InvalidPoint, "scalar multiplication failed")
        return out.raw

    def scalar_random(self) -> bytes:
        out = self._out32()
        self.lib.crypto_core_ristretto255_scalar_random(out)
        return out.raw

    def scalar_reduce(self, wide: bytes) -> bytes:
        if len(wide) != HASHBYTES:
            raise ProtoError.internal("scalar_reduce requires 64 bytes")
        out = self._out32()
        self.lib.crypto_core_ristretto255_scalar_reduce(out, self._inbuf(wide, HASHBYTES))
        return out.raw

    def scalar_add(self, a: bytes, b: bytes) -> bytes:
        out = self._out32()
        self.lib.crypto_core_ristretto255_scalar_add(out, self._inbuf(a, SCALARBYTES), self._inbuf(b, SCALARBYTES))
        return out.raw

    def scalar_sub(self, a: bytes, b: bytes) -> bytes:
        out = self._out32()
        self.lib.crypto_core_ristretto255_scalar_sub(out, self._inbuf(a, SCALARBYTES), self._inbuf(b, SCALARBYTES))
        return out.raw

    def scalar_mul(self, a: bytes, b: bytes) -> bytes:
        out = self._out32()
        self.lib.crypto_core_ristretto255_scalar_mul(out, self._inbuf(a, SCALARBYTES), self._inbuf(b, SCALARBYTES))
        return out.raw


_SODIUM: Sodium | None = None


def sodium() -> Sodium:
    global _SODIUM
    if _SODIUM is None:
        _SODIUM = Sodium()
    return _SODIUM


def canonical_scalar_bytes(data: bytes) -> bool:
    return len(data) == 32 and int.from_bytes(data, "little") < SCALAR_ORDER


def random_bytes_32() -> bytes:
    return os.urandom(32)
