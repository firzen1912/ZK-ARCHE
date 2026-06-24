//! Protocol version, cipher-suite identifiers, and capability bits.
//!
//! These are part of the wire-format specification. Do not reassign values.
//! Add new suites by appending, not by renumbering.

use crate::error::{ErrorCode, ProtoError, Result};

/// Current protocol version. The `version` byte in every framed packet header.
pub const PROTOCOL_VERSION: u8 = 0x02;

/// The lowest version this implementation can negotiate down to.
pub const MIN_SUPPORTED_VERSION: u8 = 0x02;

/// Suite identifier: (curve, hash, kdf, mac).
/// Registered values (stable, do not renumber):
///
/// | ID     | Curve        | Hash    | KDF         | MAC         |
/// |--------|--------------|---------|-------------|-------------|
/// | 0x0001 | ristretto255 | SHA-256 | HKDF-SHA256 | HMAC-SHA256 |
/// | 0x0002 | ristretto255 | SHA-512 | HKDF-SHA512 | HMAC-SHA256 | (reserved)
///
/// The transcript challenge hash (SHA-512) and the PID / KC hash (SHA-256) are
/// determined by the suite. Only 0x0001 is implemented today.
pub type SuiteId = u16;

pub const SUITE_RISTRETTO255_SHA256: SuiteId = 0x0001;

/// Capability bits exchanged in `HELLO` and `HELLO_REPLY`. A peer MUST NOT use
/// a feature whose bit is clear in the intersection of the two peers' bitmaps.
///
/// The lower 32 bits are reserved for protocol features; the upper 32 bits are
/// reserved for vendor extensions.
pub mod cap {
    pub type Bits = u64;

    /// Supports the full ZK-ARCHE v2 online auth flow (AUTH_1/2/3).
    pub const AUTH_V2:                Bits = 1 << 0;
    /// Supports role-commitment re-randomization proofs.
    pub const ROLE_RERAND:            Bits = 1 << 1;
    /// Supports role set-membership proofs.
    pub const ROLE_SET_MEMBERSHIP:    Bits = 1 << 2;
    /// Supports pairing-token-gated setup.
    pub const PAIRING_TOKEN:          Bits = 1 << 3;
    /// Supports opportunistic TOFU pinning on first setup (lab-mode only).
    pub const TOFU_SETUP:             Bits = 1 << 4;
    /// Supports the minimal profile (auth only; setup is out-of-band).
    pub const PROFILE_MINIMAL:        Bits = 1 << 8;
    /// Supports the standard profile (full setup + auth with all proofs).
    pub const PROFILE_STANDARD:       Bits = 1 << 9;
    /// Supports the gateway profile (relaying / observability).
    pub const PROFILE_GATEWAY:        Bits = 1 << 10;
    /// Supports CBOR framing in addition to the native TLV format.
    pub const CBOR_FRAMING:           Bits = 1 << 16;

    /// Baseline every conforming implementation MUST advertise.
    pub const BASELINE: Bits =
        AUTH_V2 | ROLE_RERAND | ROLE_SET_MEMBERSHIP | PROFILE_STANDARD;
}

/// The locally-supported capability set this build advertises.
pub fn local_capabilities() -> cap::Bits {
    // In the reference implementation the client uses PROFILE_STANDARD and the
    // server speaks all three; both enable the baseline plus TOFU/pairing.
    cap::BASELINE | cap::PAIRING_TOKEN
}

/// Result of negotiating version + suite + capabilities between two peers.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Negotiated {
    pub version: u8,
    pub suite:   SuiteId,
    pub caps:    cap::Bits,
}

/// Intersect two capability sets, picking the highest mutually-supported
/// version and a mutually-supported suite. Returns an error with a specific
/// wire code on mismatch so the initiator can tell *why* negotiation failed.
pub fn negotiate(
    local_version: u8,
    local_min_version: u8,
    local_suites: &[SuiteId],
    local_caps: cap::Bits,
    peer_version: u8,
    peer_min_version: u8,
    peer_suites: &[SuiteId],
    peer_caps: cap::Bits,
) -> Result<Negotiated> {
    let version = std::cmp::min(local_version, peer_version);
    if version < local_min_version || version < peer_min_version {
        return Err(ProtoError::wire(
            ErrorCode::UnsupportedVersion,
            "no mutually-supported protocol version",
        ));
    }

    // Pick the first locally-preferred suite that the peer also advertises.
    let suite = local_suites
        .iter()
        .copied()
        .find(|s| peer_suites.contains(s))
        .ok_or_else(|| ProtoError::wire(
            ErrorCode::UnsupportedSuite,
            "no mutually-supported cipher suite",
        ))?;

    let caps = local_caps & peer_caps;
    if caps & cap::BASELINE != cap::BASELINE {
        return Err(ProtoError::wire(
            ErrorCode::CapabilityMismatch,
            "baseline capabilities not mutually supported",
        ));
    }

    Ok(Negotiated { version, suite, caps })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn negotiates_common_version_and_suite() {
        let n = negotiate(
            2, 2, &[SUITE_RISTRETTO255_SHA256], cap::BASELINE,
            2, 2, &[SUITE_RISTRETTO255_SHA256], cap::BASELINE,
        ).unwrap();
        assert_eq!(n.version, 2);
        assert_eq!(n.suite,   SUITE_RISTRETTO255_SHA256);
    }

    #[test]
    fn fails_on_version_floor() {
        // Peer requires >=3, we can only offer 2.
        let e = negotiate(
            2, 2, &[SUITE_RISTRETTO255_SHA256], cap::BASELINE,
            3, 3, &[SUITE_RISTRETTO255_SHA256], cap::BASELINE,
        ).unwrap_err();
        assert_eq!(e.wire_code(), Some(ErrorCode::UnsupportedVersion));
    }
}
