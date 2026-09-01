//! Transport/channel binding normalization for `ZK-ARCHE-BIND`.
//!
//! This module classifies adapter-derived context only. It does not authenticate
//! a peer, authorize an operation, mutate trust, or interpret a transport
//! address as protocol identity. A caller may use `Bound` as one input to AUTH,
//! LINK, or resumption decisions; the binding result is never protocol authority
//! by itself.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportBindingFacts {
    pub profile_requires_binding: bool,
    pub binding_present: bool,
    pub binding_integrity_valid: bool,
    pub binding_fresh: bool,
    pub auth_instance_match: bool,
    pub peer_context_match: bool,
    pub profile_context_match: bool,
    pub transport_address_as_identity: bool,
    pub transport_metadata_as_authority: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportBindingDisposition {
    Bound,
    UnboundAllowed,
    Reject,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportBindingReason {
    Current,
    InvalidFacts,
    TransportAddressAsIdentity,
    TransportMetadataAsAuthority,
    BindingMissing,
    BindingNotRequired,
    BindingInvalid,
    BindingStale,
    AuthInstanceMismatch,
    PeerContextMismatch,
    ProfileContextMismatch,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportBindingDecision {
    pub disposition: TransportBindingDisposition,
    pub reason: TransportBindingReason,
}

fn decision(
    disposition: TransportBindingDisposition,
    reason: TransportBindingReason,
) -> TransportBindingDecision {
    TransportBindingDecision {
        disposition,
        reason,
    }
}

pub fn classify_transport_binding(facts: &TransportBindingFacts) -> TransportBindingDecision {
    if facts.transport_address_as_identity {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::TransportAddressAsIdentity,
        );
    }
    if facts.transport_metadata_as_authority {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::TransportMetadataAsAuthority,
        );
    }
    if !facts.binding_present {
        if facts.profile_requires_binding {
            return decision(
                TransportBindingDisposition::Reject,
                TransportBindingReason::BindingMissing,
            );
        }
        return decision(
            TransportBindingDisposition::UnboundAllowed,
            TransportBindingReason::BindingNotRequired,
        );
    }
    if !facts.binding_integrity_valid {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::BindingInvalid,
        );
    }
    if !facts.binding_fresh {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::BindingStale,
        );
    }
    if !facts.auth_instance_match {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::AuthInstanceMismatch,
        );
    }
    if !facts.peer_context_match {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::PeerContextMismatch,
        );
    }
    if !facts.profile_context_match {
        return decision(
            TransportBindingDisposition::Reject,
            TransportBindingReason::ProfileContextMismatch,
        );
    }
    decision(
        TransportBindingDisposition::Bound,
        TransportBindingReason::Current,
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    fn bit(value: &str) -> bool {
        match value {
            "0" => false,
            "1" => true,
            _ => panic!("invalid bit: {value}"),
        }
    }

    fn disposition(value: &str) -> TransportBindingDisposition {
        match value {
            "BOUND" => TransportBindingDisposition::Bound,
            "UNBOUND_ALLOWED" => TransportBindingDisposition::UnboundAllowed,
            "REJECT" => TransportBindingDisposition::Reject,
            _ => panic!("invalid disposition: {value}"),
        }
    }

    fn reason(value: &str) -> TransportBindingReason {
        match value {
            "CURRENT" => TransportBindingReason::Current,
            "INVALID_FACTS" => TransportBindingReason::InvalidFacts,
            "TRANSPORT_ADDRESS_AS_IDENTITY" => TransportBindingReason::TransportAddressAsIdentity,
            "TRANSPORT_METADATA_AS_AUTHORITY" => TransportBindingReason::TransportMetadataAsAuthority,
            "BINDING_MISSING" => TransportBindingReason::BindingMissing,
            "BINDING_NOT_REQUIRED" => TransportBindingReason::BindingNotRequired,
            "BINDING_INVALID" => TransportBindingReason::BindingInvalid,
            "BINDING_STALE" => TransportBindingReason::BindingStale,
            "AUTH_INSTANCE_MISMATCH" => TransportBindingReason::AuthInstanceMismatch,
            "PEER_CONTEXT_MISMATCH" => TransportBindingReason::PeerContextMismatch,
            "PROFILE_CONTEXT_MISMATCH" => TransportBindingReason::ProfileContextMismatch,
            _ => panic!("invalid reason: {value}"),
        }
    }

    #[test]
    fn canonical_corpus_matches_classifier() {
        let corpus = include_str!("../../../../test-vectors/state/transport-binding-v1.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else {
                continue;
            };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 12);
            let facts = TransportBindingFacts {
                profile_requires_binding: bit(fields[1]),
                binding_present: bit(fields[2]),
                binding_integrity_valid: bit(fields[3]),
                binding_fresh: bit(fields[4]),
                auth_instance_match: bit(fields[5]),
                peer_context_match: bit(fields[6]),
                profile_context_match: bit(fields[7]),
                transport_address_as_identity: bit(fields[8]),
                transport_metadata_as_authority: bit(fields[9]),
            };
            assert_eq!(
                classify_transport_binding(&facts),
                TransportBindingDecision {
                    disposition: disposition(fields[10]),
                    reason: reason(fields[11]),
                },
                "case {}",
                fields[0]
            );
            count += 1;
        }
        assert_eq!(count, 12);
    }
}
