//! Bounded local audit hash chain for protected DATA release events.
//!
//! This module is wire-neutral. Audit state is evidence about local release
//! decisions; it is never an authorization source and never substitutes for
//! current AUTH, revocation, lineage, policy, or device release authority.

use sha2::{Digest, Sha256};

const DOMAIN: &[u8] = b"ZKARCHE-DATA-AUDIT-v1";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum DataAuditEventKind {
    ReleaseGranted = 1,
    ReleaseDenied = 2,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DataAuditEvent {
    /// Commitment to the canonical release context; never protected plaintext.
    pub release_context_hash: [u8; 32],
    pub authorization_generation: u64,
    pub revocation_epoch: u64,
    pub policy_epoch: u64,
    pub kind: DataAuditEventKind,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DataAuditState {
    pub head: [u8; 32],
    pub next_sequence: u64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DataAuditError {
    InvalidSequence,
    SequenceExhausted,
}

impl DataAuditState {
    pub const fn genesis() -> Self {
        Self {
            head: [0u8; 32],
            next_sequence: 1,
        }
    }
}

pub fn data_audit_entry_digest(
    previous_head: &[u8; 32],
    sequence: u64,
    event: &DataAuditEvent,
) -> Result<[u8; 32], DataAuditError> {
    if sequence == 0 {
        return Err(DataAuditError::InvalidSequence);
    }

    let mut h = Sha256::new();
    h.update(DOMAIN);
    h.update(previous_head);
    h.update(sequence.to_be_bytes());
    h.update([event.kind as u8]);
    h.update(event.release_context_hash);
    h.update(event.authorization_generation.to_be_bytes());
    h.update(event.revocation_epoch.to_be_bytes());
    h.update(event.policy_epoch.to_be_bytes());
    Ok(h.finalize().into())
}

pub fn append_data_audit_event(
    state: &mut DataAuditState,
    event: &DataAuditEvent,
) -> Result<[u8; 32], DataAuditError> {
    if state.next_sequence == u64::MAX {
        return Err(DataAuditError::SequenceExhausted);
    }
    let digest = data_audit_entry_digest(&state.head, state.next_sequence, event)?;
    state.head = digest;
    state.next_sequence += 1;
    Ok(digest)
}

pub fn verify_data_audit_transition(
    previous_head: &[u8; 32],
    sequence: u64,
    event: &DataAuditEvent,
    expected_head: &[u8; 32],
) -> bool {
    data_audit_entry_digest(previous_head, sequence, event)
        .map(|actual| actual == *expected_head)
        .unwrap_or(false)
}

#[cfg(test)]
mod tests {
    use super::*;

    const EXPECTED_FIRST: [u8; 32] = [
        0x9d, 0xf3, 0x6e, 0x00, 0xb5, 0x56, 0x84, 0xfc,
        0xd1, 0xaf, 0x82, 0x97, 0xcb, 0x83, 0x2f, 0x61,
        0x6b, 0x62, 0xcd, 0xe5, 0x18, 0x1c, 0x0c, 0xe9,
        0xf2, 0xcd, 0x8e, 0x99, 0x7b, 0xe5, 0x80, 0x98,
    ];

    fn event() -> DataAuditEvent {
        let mut release_context_hash = [0u8; 32];
        for (i, byte) in release_context_hash.iter_mut().enumerate() {
            *byte = i as u8;
        }
        DataAuditEvent {
            release_context_hash,
            authorization_generation: 7,
            revocation_epoch: 11,
            policy_epoch: 13,
            kind: DataAuditEventKind::ReleaseGranted,
        }
    }

    #[test]
    fn canonical_first_entry_matches_cross_language_fixture() {
        let mut state = DataAuditState::genesis();
        assert_eq!(append_data_audit_event(&mut state, &event()).unwrap(), EXPECTED_FIRST);
        assert_eq!(state.head, EXPECTED_FIRST);
        assert_eq!(state.next_sequence, 2);
    }

    #[test]
    fn continuity_and_event_mutation_are_detectable() {
        let e = event();
        assert!(verify_data_audit_transition(&[0u8; 32], 1, &e, &EXPECTED_FIRST));
        assert!(!verify_data_audit_transition(&[1u8; 32], 1, &e, &EXPECTED_FIRST));
        assert!(!verify_data_audit_transition(&[0u8; 32], 2, &e, &EXPECTED_FIRST));

        let mut changed = e;
        changed.release_context_hash[0] ^= 1;
        assert!(!verify_data_audit_transition(&[0u8; 32], 1, &changed, &EXPECTED_FIRST));

        changed = e;
        changed.authorization_generation += 1;
        assert!(!verify_data_audit_transition(&[0u8; 32], 1, &changed, &EXPECTED_FIRST));

        changed = e;
        changed.revocation_epoch += 1;
        assert!(!verify_data_audit_transition(&[0u8; 32], 1, &changed, &EXPECTED_FIRST));

        changed = e;
        changed.policy_epoch += 1;
        assert!(!verify_data_audit_transition(&[0u8; 32], 1, &changed, &EXPECTED_FIRST));

        changed = e;
        changed.kind = DataAuditEventKind::ReleaseDenied;
        assert!(!verify_data_audit_transition(&[0u8; 32], 1, &changed, &EXPECTED_FIRST));
    }

    #[test]
    fn sequence_zero_and_exhaustion_fail_closed() {
        assert_eq!(
            data_audit_entry_digest(&[0u8; 32], 0, &event()),
            Err(DataAuditError::InvalidSequence)
        );
        let mut state = DataAuditState {
            head: [0u8; 32],
            next_sequence: u64::MAX,
        };
        assert_eq!(
            append_data_audit_event(&mut state, &event()),
            Err(DataAuditError::SequenceExhausted)
        );
    }
}
