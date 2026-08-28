//! Fail-closed replay-continuity state for draft constrained profiles.
//!
//! This module implements only the restart/state-loss semantics already
//! specified by `spec/replay-continuity.md`. It deliberately does not define
//! or authorize a fresh replay-epoch transition.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ReplayContinuityState {
    Trusted,
    Restoring,
    ContinuityBroken,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ReplayContinuityEvent {
    Restart,
    RestoredTrustedWindow,
    RestoreMissing,
    RestoreCorrupt,
    RestoreStale,
    RollbackSuspected,
    EmptyCacheReset,
    FreshOuterSession,
    FailedAuth,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ReplayContinuityError {
    InvalidTransition,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ReplayContinuity {
    state: ReplayContinuityState,
}

impl ReplayContinuity {
    pub const fn new(state: ReplayContinuityState) -> Self {
        Self { state }
    }

    pub const fn state(&self) -> ReplayContinuityState {
        self.state
    }

    pub const fn auth_admission_allowed(&self) -> bool {
        matches!(self.state, ReplayContinuityState::Trusted)
    }

    pub fn apply(&mut self, event: ReplayContinuityEvent) -> Result<(), ReplayContinuityError> {
        use ReplayContinuityEvent as Event;
        use ReplayContinuityState as State;

        let next = match (self.state, event) {
            (State::Trusted, Event::Restart) => State::Restoring,
            (State::Restoring, Event::RestoredTrustedWindow) => State::Trusted,
            (
                State::Restoring,
                Event::RestoreMissing
                | Event::RestoreCorrupt
                | Event::RestoreStale
                | Event::RollbackSuspected,
            ) => State::ContinuityBroken,
            (state, Event::FailedAuth) => state,
            (State::ContinuityBroken, Event::EmptyCacheReset | Event::FreshOuterSession) => {
                State::ContinuityBroken
            }
            _ => return Err(ReplayContinuityError::InvalidTransition),
        };

        self.state = next;
        Ok(())
    }
}
