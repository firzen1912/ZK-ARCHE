#![no_main]

use libfuzzer_sys::fuzz_target;
use proto::data_audit_chain::{
    append_data_audit_event, data_audit_entry_digest, verify_data_audit_transition,
    DataAuditEvent, DataAuditEventKind, DataAuditState,
};

fn read_u64(bytes: &[u8]) -> u64 {
    u64::from_be_bytes(bytes.try_into().expect("fixed-width fuzz slice"))
}

fuzz_target!(|data: &[u8]| {
    if data.len() < 97 {
        return;
    }

    let mut previous_head = [0u8; 32];
    previous_head.copy_from_slice(&data[0..32]);

    let mut release_context_hash = [0u8; 32];
    release_context_hash.copy_from_slice(&data[32..64]);

    let sequence = read_u64(&data[64..72]);
    let event = DataAuditEvent {
        release_context_hash,
        authorization_generation: read_u64(&data[72..80]),
        revocation_epoch: read_u64(&data[80..88]),
        policy_epoch: read_u64(&data[88..96]),
        kind: if data[96] & 1 == 0 {
            DataAuditEventKind::ReleaseGranted
        } else {
            DataAuditEventKind::ReleaseDenied
        },
    };

    let digest = data_audit_entry_digest(&previous_head, sequence, &event);
    if let Ok(expected_head) = digest {
        assert!(verify_data_audit_transition(
            &previous_head,
            sequence,
            &event,
            &expected_head,
        ));
    }

    let mut state = DataAuditState {
        head: previous_head,
        next_sequence: sequence,
    };
    let before = state;
    match append_data_audit_event(&mut state, &event) {
        Ok(actual_head) => {
            assert_eq!(Some(actual_head), digest.ok());
            assert_eq!(state.head, actual_head);
            assert_eq!(state.next_sequence, sequence + 1);
        }
        Err(_) => {
            assert_eq!(state, before);
        }
    }
});
