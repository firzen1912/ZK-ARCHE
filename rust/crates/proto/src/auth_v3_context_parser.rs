//! Strict raw parser for the draft AUTH-v3 canonical `ZKCTX` v1 envelope.
//!
//! The canonical encoder lives in `auth_v3_context`. This module provides the
//! receive-side half of the same contract: raw bytes are parsed without
//! normalization and are rejected unless they are already the unique canonical
//! representation required by `spec/auth-v3-context-encoding.md`.

use crate::auth_v3_context::{ContextEntry, ContextKind, CONTEXT_ENCODING_VERSION, CONTEXT_MAGIC};
use sha2::{Digest, Sha256};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ContextParseError {
    Truncated,
    InvalidMagic,
    UnsupportedVersion,
    UnknownKind,
    InvalidId,
    NonCanonicalOrder,
    InvalidCriticalId,
    NonZeroFlags,
    TrailingBytes,
    EntryLimitExceeded,
}

pub struct ParsedContext<'a> {
    pub kind: ContextKind,
    pub entries: Vec<ContextEntry<'a>>,
}

fn parse_kind(value: u8) -> Result<ContextKind, ContextParseError> {
    match value {
        1 => Ok(ContextKind::Authorization),
        2 => Ok(ContextKind::CriticalExtensions),
        3 => Ok(ContextKind::ChannelBinding),
        _ => Err(ContextParseError::UnknownKind),
    }
}

fn parse_canonical_context_impl(
    input: &[u8],
    max_entries: Option<usize>,
) -> Result<ParsedContext<'_>, ContextParseError> {
    if input.len() < 9 {
        return Err(ContextParseError::Truncated);
    }
    if &input[..CONTEXT_MAGIC.len()] != CONTEXT_MAGIC {
        return Err(ContextParseError::InvalidMagic);
    }
    if input[5] != CONTEXT_ENCODING_VERSION {
        return Err(ContextParseError::UnsupportedVersion);
    }

    let kind = parse_kind(input[6])?;
    let entry_count = u16::from_le_bytes([input[7], input[8]]) as usize;
    if max_entries.is_some_and(|limit| entry_count > limit) {
        return Err(ContextParseError::EntryLimitExceeded);
    }
    let structural_entry_limit = (input.len() - 9) / 5;
    if entry_count > structural_entry_limit {
        return Err(ContextParseError::Truncated);
    }
    let mut entries = Vec::with_capacity(entry_count);
    let mut offset = 9usize;
    let mut previous_id = 0u16;

    for index in 0..entry_count {
        let header_end = offset.checked_add(5).ok_or(ContextParseError::Truncated)?;
        let header = input
            .get(offset..header_end)
            .ok_or(ContextParseError::Truncated)?;
        let id = u16::from_le_bytes([header[0], header[1]]);
        let flags = header[2];
        let value_len = u16::from_le_bytes([header[3], header[4]]) as usize;

        if id == 0 {
            return Err(ContextParseError::InvalidId);
        }
        if index > 0 && id <= previous_id {
            return Err(ContextParseError::NonCanonicalOrder);
        }
        if kind == ContextKind::CriticalExtensions && ((id & 0x8000) == 0 || (id & 0x7fff) == 0) {
            return Err(ContextParseError::InvalidCriticalId);
        }
        if flags != 0 {
            return Err(ContextParseError::NonZeroFlags);
        }

        let value_end = header_end
            .checked_add(value_len)
            .ok_or(ContextParseError::Truncated)?;
        let value = input
            .get(header_end..value_end)
            .ok_or(ContextParseError::Truncated)?;
        entries.push(ContextEntry { id, value });
        previous_id = id;
        offset = value_end;
    }

    if offset != input.len() {
        return Err(ContextParseError::TrailingBytes);
    }

    Ok(ParsedContext { kind, entries })
}

/// Parse one byte-exact `ZKCTX` v1 value.
///
/// Returned entry values borrow directly from `input`; the parser never sorts,
/// rewrites, deduplicates, or otherwise normalizes attacker-controlled bytes.
pub fn parse_canonical_context(input: &[u8]) -> Result<ParsedContext<'_>, ContextParseError> {
    parse_canonical_context_impl(input, None)
}

/// Parse one byte-exact `ZKCTX` v1 value while enforcing a caller-selected
/// entry-count ceiling before allocating the parsed entry vector.
///
/// Constrained/profile-specific receive paths SHOULD use this form with their
/// already-approved profile bound so an attacker-controlled entry count cannot
/// cause materialization beyond that profile's resource envelope.
pub fn parse_canonical_context_bounded(
    input: &[u8],
    max_entries: usize,
) -> Result<ParsedContext<'_>, ContextParseError> {
    parse_canonical_context_impl(input, Some(max_entries))
}

/// Validate the raw canonical representation before hashing the exact bytes.
///
/// This intentionally hashes `input` rather than a re-encoded semantic object,
/// so a receiver cannot accidentally accept one representation and authenticate
/// another.
pub fn hash_canonical_context_bytes(input: &[u8]) -> Result<[u8; 32], ContextParseError> {
    parse_canonical_context(input)?;
    let mut out = [0u8; 32];
    out.copy_from_slice(&Sha256::digest(input));
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn decode(hex_value: &str) -> Vec<u8> {
        hex::decode(hex_value).unwrap()
    }

    fn assert_error(hex_value: &str, expected: ContextParseError) {
        assert_eq!(
            parse_canonical_context(&decode(hex_value)).err(),
            Some(expected)
        );
    }

    #[test]
    fn canonical_values_parse_without_normalization() {
        let encoded = decode(
            "5a4b435458010103000100000100010200000600656467652d61030000100000112233445566778899aabbccddeeff",
        );
        let parsed = parse_canonical_context(&encoded).unwrap();
        assert_eq!(parsed.kind, ContextKind::Authorization);
        assert_eq!(parsed.entries.len(), 3);
        assert_eq!(parsed.entries[0].id, 1);
        assert_eq!(parsed.entries[0].value, &[0x01]);
        assert_eq!(parsed.entries[1].id, 2);
        assert_eq!(parsed.entries[1].value, b"edge-a");
        assert_eq!(parsed.entries[2].id, 3);
        assert_eq!(
            hex::encode(hash_canonical_context_bytes(&encoded).unwrap()),
            "3f85c714dfca2070fcfa909bfa31442c5828f4f91834f62e140db0320fcfcb69"
        );

        let empty = decode("5a4b43545801030000");
        let parsed_empty = parse_canonical_context(&empty).unwrap();
        assert_eq!(parsed_empty.kind, ContextKind::ChannelBinding);
        assert!(parsed_empty.entries.is_empty());
        assert_eq!(
            hex::encode(hash_canonical_context_bytes(&empty).unwrap()),
            "7f724afa7e3e7a6c13e0fe167fc48a034888d10c523abd7864671c68aaea5fa8"
        );
    }

    #[test]
    fn bounded_parse_rejects_profile_excess_before_materialization() {
        let encoded = decode(
            "5a4b4354580101080001000000000200000000030000000004000000000500000000060000000007000000000800000000",
        );
        assert_eq!(parse_canonical_context(&encoded).unwrap().entries.len(), 8);
        assert_eq!(
            parse_canonical_context_bounded(&encoded, 7).err(),
            Some(ContextParseError::EntryLimitExceeded)
        );
    }

    #[test]
    fn malformed_raw_contexts_fail_closed() {
        assert_error("5a4b435458010100", ContextParseError::Truncated);
        assert_error("004b43545801010000", ContextParseError::InvalidMagic);
        assert_error("5a4b43545802010000", ContextParseError::UnsupportedVersion);
        assert_error("5a4b43545801040000", ContextParseError::UnknownKind);
        assert_error("5a4b4354580101ffff", ContextParseError::Truncated);
        assert_error("5a4b435458010101000000000000", ContextParseError::InvalidId);
        assert_error(
            "5a4b4354580101020001000000000100000000",
            ContextParseError::NonCanonicalOrder,
        );
        assert_error(
            "5a4b4354580101020002000000000100000000",
            ContextParseError::NonCanonicalOrder,
        );
        assert_error(
            "5a4b435458010201000100000000",
            ContextParseError::InvalidCriticalId,
        );
        assert_error(
            "5a4b435458010101000100010000",
            ContextParseError::NonZeroFlags,
        );
        assert_error("5a4b435458010101000100", ContextParseError::Truncated);
        assert_error(
            "5a4b435458010101000100000200aa",
            ContextParseError::Truncated,
        );
        assert_error("5a4b4354580101000000", ContextParseError::TrailingBytes);
    }
}
