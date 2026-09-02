//! Transport-independent association continuation decision.
//!
//! A routing address, socket, connection identifier, or other adapter metadata
//! is never protocol identity or authorization authority. This classifier
//! decides whether an already-authenticated association may continue across a
//! reconnect/re-route, must fall back to fresh AUTH, or must fail closed.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportContinuationFacts {
    pub continuation_requested: bool,
    pub authenticated_peer_context_match: bool,
    pub authenticated_profile_context_match: bool,
    pub binding_required: bool,
    pub binding_valid: bool,
    pub resumption_authorized: bool,
    pub replay_continuity_current: bool,
    pub association_invalidated: bool,
    pub transport_route_changed: bool,
    pub transport_connection_changed: bool,
    pub transport_address_as_identity: bool,
    pub transport_metadata_as_authority: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportContinuationAction {
    Continue,
    FullAuthRequired,
    Reject,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportContinuationReason {
    Current,
    TransportAddressAsIdentity,
    TransportMetadataAsAuthority,
    AssociationInvalidated,
    ReplayContinuityStale,
    PeerContextMismatch,
    ProfileContextMismatch,
    BindingInvalid,
    ResumptionNotAuthorized,
    ContinuationNotRequested,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportContinuationDecision {
    pub action: TransportContinuationAction,
    pub reason: TransportContinuationReason,
}

fn decision(
    action: TransportContinuationAction,
    reason: TransportContinuationReason,
) -> TransportContinuationDecision {
    TransportContinuationDecision { action, reason }
}

pub fn classify_transport_continuation(
    facts: &TransportContinuationFacts,
) -> TransportContinuationDecision {
    if facts.transport_address_as_identity {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::TransportAddressAsIdentity,
        );
    }
    if facts.transport_metadata_as_authority {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::TransportMetadataAsAuthority,
        );
    }
    if facts.association_invalidated {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::AssociationInvalidated,
        );
    }
    if !facts.replay_continuity_current {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::ReplayContinuityStale,
        );
    }
    if !facts.authenticated_peer_context_match {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::PeerContextMismatch,
        );
    }
    if !facts.authenticated_profile_context_match {
        return decision(
            TransportContinuationAction::FullAuthRequired,
            TransportContinuationReason::ProfileContextMismatch,
        );
    }
    if facts.binding_required && !facts.binding_valid {
        return decision(
            TransportContinuationAction::Reject,
            TransportContinuationReason::BindingInvalid,
        );
    }
    if !facts.resumption_authorized {
        return decision(
            TransportContinuationAction::FullAuthRequired,
            TransportContinuationReason::ResumptionNotAuthorized,
        );
    }
    if !facts.continuation_requested {
        return decision(
            TransportContinuationAction::FullAuthRequired,
            TransportContinuationReason::ContinuationNotRequested,
        );
    }

    // Route/connection changes are deliberately not authority inputs. Once the
    // authenticated peer/profile context, required binding, resumption policy,
    // replay continuity, and invalidation state are valid, a changed route or
    // transport connection does not by itself change protocol identity.
    let _ = facts.transport_route_changed;
    let _ = facts.transport_connection_changed;

    decision(
        TransportContinuationAction::Continue,
        TransportContinuationReason::Current,
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

    fn action(value: &str) -> TransportContinuationAction {
        match value {
            "CONTINUE" => TransportContinuationAction::Continue,
            "FULL_AUTH_REQUIRED" => TransportContinuationAction::FullAuthRequired,
            "REJECT" => TransportContinuationAction::Reject,
            _ => panic!("invalid action: {value}"),
        }
    }

    fn reason(value: &str) -> TransportContinuationReason {
        match value {
            "CURRENT" => TransportContinuationReason::Current,
            "TRANSPORT_ADDRESS_AS_IDENTITY" => TransportContinuationReason::TransportAddressAsIdentity,
            "TRANSPORT_METADATA_AS_AUTHORITY" => TransportContinuationReason::TransportMetadataAsAuthority,
            "ASSOCIATION_INVALIDATED" => TransportContinuationReason::AssociationInvalidated,
            "REPLAY_CONTINUITY_STALE" => TransportContinuationReason::ReplayContinuityStale,
            "PEER_CONTEXT_MISMATCH" => TransportContinuationReason::PeerContextMismatch,
            "PROFILE_CONTEXT_MISMATCH" => TransportContinuationReason::ProfileContextMismatch,
            "BINDING_INVALID" => TransportContinuationReason::BindingInvalid,
            "RESUMPTION_NOT_AUTHORIZED" => TransportContinuationReason::ResumptionNotAuthorized,
            "CONTINUATION_NOT_REQUESTED" => TransportContinuationReason::ContinuationNotRequested,
            _ => panic!("invalid reason: {value}"),
        }
    }

    #[test]
    fn canonical_corpus_matches_classifier() {
        let corpus = include_str!("../../../../test-vectors/state/transport-continuation-v1.txt");
        let mut count = 0usize;

        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else {
                continue;
            };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 15);
            let facts = TransportContinuationFacts {
                continuation_requested: bit(fields[1]),
                authenticated_peer_context_match: bit(fields[2]),
                authenticated_profile_context_match: bit(fields[3]),
                binding_required: bit(fields[4]),
                binding_valid: bit(fields[5]),
                resumption_authorized: bit(fields[6]),
                replay_continuity_current: bit(fields[7]),
                association_invalidated: bit(fields[8]),
                transport_route_changed: bit(fields[9]),
                transport_connection_changed: bit(fields[10]),
                transport_address_as_identity: bit(fields[11]),
                transport_metadata_as_authority: bit(fields[12]),
            };
            assert_eq!(
                classify_transport_continuation(&facts),
                TransportContinuationDecision {
                    action: action(fields[13]),
                    reason: reason(fields[14]),
                },
                "case {}",
                fields[0]
            );
            count += 1;
        }

        assert_eq!(count, 13);
    }
}
