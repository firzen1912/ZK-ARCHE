//! Draft AUTH-v3 canonical subcontext encoding.
//!
//! This module implements the non-advertised `ZKCTX` v1 envelope defined in
//! `spec/auth-v3-context-encoding.md`. It exists to make AUTH-v3 context hashes
//! byte-exact across implementations before production negotiation is enabled.

use sha2::{Digest, Sha256};

pub const CONTEXT_MAGIC: &[u8; 5] = b"ZKCTX";
pub const CONTEXT_ENCODING_VERSION: u8 = 1;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(u8)]
pub enum ContextKind {
    Authorization = 1,
    CriticalExtensions = 2,
    ChannelBinding = 3,
}

#[derive(Clone, Copy)]
pub struct ContextEntry<'a> {
    pub id: u16,
    pub value: &'a [u8],
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ContextEncodingError {
    InvalidId,
    NonCanonicalOrder,
    InvalidCriticalId,
    TooManyEntries,
    ValueTooLong,
}

fn validate_entries(kind: ContextKind, entries: &[ContextEntry<'_>]) -> Result<(), ContextEncodingError> {
    if entries.len() > u16::MAX as usize {
        return Err(ContextEncodingError::TooManyEntries);
    }

    let mut previous = 0u16;
    for (index, entry) in entries.iter().enumerate() {
        if entry.id == 0 {
            return Err(ContextEncodingError::InvalidId);
        }
        if index > 0 && entry.id <= previous {
            return Err(ContextEncodingError::NonCanonicalOrder);
        }
        if kind == ContextKind::CriticalExtensions
            && ((entry.id & 0x8000) == 0 || (entry.id & 0x7fff) == 0)
        {
            return Err(ContextEncodingError::InvalidCriticalId);
        }
        if entry.value.len() > u16::MAX as usize {
            return Err(ContextEncodingError::ValueTooLong);
        }
        previous = entry.id;
    }
    Ok(())
}

pub fn encode_canonical_context(
    kind: ContextKind,
    entries: &[ContextEntry<'_>],
) -> Result<Vec<u8>, ContextEncodingError> {
    validate_entries(kind, entries)?;

    let value_bytes: usize = entries.iter().map(|entry| entry.value.len()).sum();
    let mut out = Vec::with_capacity(9 + entries.len() * 5 + value_bytes);
    out.extend_from_slice(CONTEXT_MAGIC);
    out.push(CONTEXT_ENCODING_VERSION);
    out.push(kind as u8);
    out.extend_from_slice(&(entries.len() as u16).to_le_bytes());

    for entry in entries {
        out.extend_from_slice(&entry.id.to_le_bytes());
        out.push(0); // reserved flags byte
        out.extend_from_slice(&(entry.value.len() as u16).to_le_bytes());
        out.extend_from_slice(entry.value);
    }

    Ok(out)
}

pub fn hash_canonical_context(
    kind: ContextKind,
    entries: &[ContextEntry<'_>],
) -> Result<[u8; 32], ContextEncodingError> {
    let encoded = encode_canonical_context(kind, entries)?;
    let mut out = [0u8; 32];
    out.copy_from_slice(&Sha256::digest(encoded));
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn assert_case(kind: ContextKind, entries: &[ContextEntry<'_>], expected_hex: &str, expected_hash: &str) {
        let encoded = encode_canonical_context(kind, entries).unwrap();
        assert_eq!(hex::encode(&encoded), expected_hex);
        assert_eq!(hex::encode(hash_canonical_context(kind, entries).unwrap()), expected_hash);
    }

    #[test]
    fn canonical_empty_contexts_are_distinct() {
        assert_case(
            ContextKind::Authorization,
            &[],
            "5a4b43545801010000",
            "505121c6096720d111eab443818cc974bb66f3339e06de742f69e4692dd2717a",
        );
        assert_case(
            ContextKind::CriticalExtensions,
            &[],
            "5a4b43545801020000",
            "ef8116870a7dc594749827eae3c9a5346057612b0d93ed3d1f0cea3d6ff0f3ed",
        );
        assert_case(
            ContextKind::ChannelBinding,
            &[],
            "5a4b43545801030000",
            "7f724afa7e3e7a6c13e0fe167fc48a034888d10c523abd7864671c68aaea5fa8",
        );
    }

    #[test]
    fn canonical_positive_vectors_are_stable() {
        let authz = [
            ContextEntry { id: 1, value: &[0x01] },
            ContextEntry { id: 2, value: b"edge-a" },
            ContextEntry { id: 3, value: &[0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff] },
        ];
        assert_case(
            ContextKind::Authorization,
            &authz,
            "5a4b435458010103000100000100010200000600656467652d61030000100000112233445566778899aabbccddeeff",
            "3f85c714dfca2070fcfa909bfa31442c5828f4f91834f62e140db0320fcfcb69",
        );

        let critical = [
            ContextEntry { id: 0x8001, value: &[0xaa, 0xbb] },
            ContextEntry { id: 0x8004, value: &[0x01, 0x02, 0x03] },
        ];
        assert_case(
            ContextKind::CriticalExtensions,
            &critical,
            "5a4b435458010202000180000200aabb0480000300010203",
            "3f4424631680740d286af85cd2eb397e89e32a035eca62b8d9498aa970d4e36c",
        );

        let channel = [ContextEntry { id: 1, value: b"tls-exporter-example" }];
        assert_case(
            ContextKind::ChannelBinding,
            &channel,
            "5a4b435458010301000100001400746c732d6578706f727465722d6578616d706c65",
            "a8916d7d0e1cac4884319dd7149e76937e654e51e233edfb641232ccd0a5118a",
        );
    }

    #[test]
    fn malformed_semantics_fail_closed() {
        assert_eq!(
            encode_canonical_context(
                ContextKind::Authorization,
                &[ContextEntry { id: 0, value: &[] }],
            ),
            Err(ContextEncodingError::InvalidId)
        );
        assert_eq!(
            encode_canonical_context(
                ContextKind::Authorization,
                &[
                    ContextEntry { id: 1, value: &[1] },
                    ContextEntry { id: 1, value: &[2] },
                ],
            ),
            Err(ContextEncodingError::NonCanonicalOrder)
        );
        assert_eq!(
            encode_canonical_context(
                ContextKind::Authorization,
                &[
                    ContextEntry { id: 2, value: &[1] },
                    ContextEntry { id: 1, value: &[2] },
                ],
            ),
            Err(ContextEncodingError::NonCanonicalOrder)
        );
        assert_eq!(
            encode_canonical_context(
                ContextKind::CriticalExtensions,
                &[ContextEntry { id: 1, value: &[1] }],
            ),
            Err(ContextEncodingError::InvalidCriticalId)
        );
    }
}
