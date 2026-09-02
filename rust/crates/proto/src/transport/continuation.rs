//! Transport-independent association continuation decision.
//!
//! Routing addresses and adapter metadata are never protocol identity or
//! authorization authority. Continuation also inherits the bounded-reuse and
//! authorization-generation freshness requirements of resumption.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportContinuationFacts {
    pub continuation_requested: bool,
    pub authenticated_peer_context_match: bool,
    pub authenticated_profile_context_match: bool,
    pub binding_required: bool,
    pub binding_valid: bool,
    pub resumption_authorized: bool,
    pub replay_continuity_current: bool,
    pub usage_counter_continuity_current: bool,
    pub authorization_generation_current: bool,
    pub association_invalidated: bool,
    pub transport_route_changed: bool,
    pub transport_connection_changed: bool,
    pub transport_address_as_identity: bool,
    pub transport_metadata_as_authority: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportContinuationAction { Continue, FullAuthRequired, Reject }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransportContinuationReason {
    Current, TransportAddressAsIdentity, TransportMetadataAsAuthority,
    AssociationInvalidated, ReplayContinuityStale, UsageCounterContinuityStale,
    AuthorizationGenerationStale, PeerContextMismatch, ProfileContextMismatch,
    BindingInvalid, ResumptionNotAuthorized, ContinuationNotRequested,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransportContinuationDecision {
    pub action: TransportContinuationAction,
    pub reason: TransportContinuationReason,
}

fn decision(action: TransportContinuationAction, reason: TransportContinuationReason) -> TransportContinuationDecision {
    TransportContinuationDecision { action, reason }
}

pub fn classify_transport_continuation(f: &TransportContinuationFacts) -> TransportContinuationDecision {
    use TransportContinuationAction::*;
    use TransportContinuationReason::*;
    if f.transport_address_as_identity { return decision(Reject, TransportAddressAsIdentity); }
    if f.transport_metadata_as_authority { return decision(Reject, TransportMetadataAsAuthority); }
    if f.association_invalidated { return decision(Reject, AssociationInvalidated); }
    if !f.replay_continuity_current { return decision(Reject, ReplayContinuityStale); }
    if !f.usage_counter_continuity_current { return decision(Reject, UsageCounterContinuityStale); }
    if !f.authorization_generation_current { return decision(FullAuthRequired, AuthorizationGenerationStale); }
    if !f.authenticated_peer_context_match { return decision(Reject, PeerContextMismatch); }
    if !f.authenticated_profile_context_match { return decision(FullAuthRequired, ProfileContextMismatch); }
    if f.binding_required && !f.binding_valid { return decision(Reject, BindingInvalid); }
    if !f.resumption_authorized { return decision(FullAuthRequired, ResumptionNotAuthorized); }
    if !f.continuation_requested { return decision(FullAuthRequired, ContinuationNotRequested); }
    let _ = f.transport_route_changed;
    let _ = f.transport_connection_changed;
    decision(Continue, Current)
}

#[cfg(test)]
mod tests {
    use super::*;
    fn bit(v: &str) -> bool { match v { "0" => false, "1" => true, _ => panic!("invalid bit: {v}") } }
    fn action(v: &str) -> TransportContinuationAction { match v { "CONTINUE" => TransportContinuationAction::Continue, "FULL_AUTH_REQUIRED" => TransportContinuationAction::FullAuthRequired, "REJECT" => TransportContinuationAction::Reject, _ => panic!("invalid action: {v}") } }
    fn reason(v: &str) -> TransportContinuationReason { match v {
        "CURRENT" => TransportContinuationReason::Current,
        "TRANSPORT_ADDRESS_AS_IDENTITY" => TransportContinuationReason::TransportAddressAsIdentity,
        "TRANSPORT_METADATA_AS_AUTHORITY" => TransportContinuationReason::TransportMetadataAsAuthority,
        "ASSOCIATION_INVALIDATED" => TransportContinuationReason::AssociationInvalidated,
        "REPLAY_CONTINUITY_STALE" => TransportContinuationReason::ReplayContinuityStale,
        "USAGE_COUNTER_CONTINUITY_STALE" => TransportContinuationReason::UsageCounterContinuityStale,
        "AUTHORIZATION_GENERATION_STALE" => TransportContinuationReason::AuthorizationGenerationStale,
        "PEER_CONTEXT_MISMATCH" => TransportContinuationReason::PeerContextMismatch,
        "PROFILE_CONTEXT_MISMATCH" => TransportContinuationReason::ProfileContextMismatch,
        "BINDING_INVALID" => TransportContinuationReason::BindingInvalid,
        "RESUMPTION_NOT_AUTHORIZED" => TransportContinuationReason::ResumptionNotAuthorized,
        "CONTINUATION_NOT_REQUESTED" => TransportContinuationReason::ContinuationNotRequested,
        _ => panic!("invalid reason: {v}"), } }

    #[test]
    fn canonical_corpus_matches_classifier() {
        let corpus = include_str!("../../../../test-vectors/state/transport-continuation-v2.txt");
        let mut count = 0usize;
        for line in corpus.lines() {
            let Some(case) = line.strip_prefix("case=") else { continue; };
            let fields: Vec<&str> = case.split('|').collect();
            assert_eq!(fields.len(), 17);
            let facts = TransportContinuationFacts {
                continuation_requested: bit(fields[1]), authenticated_peer_context_match: bit(fields[2]),
                authenticated_profile_context_match: bit(fields[3]), binding_required: bit(fields[4]),
                binding_valid: bit(fields[5]), resumption_authorized: bit(fields[6]),
                replay_continuity_current: bit(fields[7]), usage_counter_continuity_current: bit(fields[8]),
                authorization_generation_current: bit(fields[9]), association_invalidated: bit(fields[10]),
                transport_route_changed: bit(fields[11]), transport_connection_changed: bit(fields[12]),
                transport_address_as_identity: bit(fields[13]), transport_metadata_as_authority: bit(fields[14]),
            };
            assert_eq!(classify_transport_continuation(&facts), TransportContinuationDecision { action: action(fields[15]), reason: reason(fields[16]) }, "case {}", fields[0]);
            count += 1;
        }
        assert_eq!(count, 15);
    }
}
