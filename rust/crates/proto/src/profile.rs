//! Session timing parameters and profile selection.
//!
//! These are part of the wire-format specification: peers MUST agree on
//! baseline values or the retry behavior diverges in the field. Constants
//! here mirror the reference implementation; customization happens via the
//! `Profile` struct, which callers pass into the state machines.

use std::time::Duration;

/// Default initial retry timeout. UDP only. TCP transports ignore.
pub const DEFAULT_RETRANSMIT_TIMEOUT: Duration = Duration::from_millis(800);

/// Default retry count before giving up. UDP only.
pub const DEFAULT_MAX_RETRIES: usize = 4;

/// Backoff cap: `timeout << min(attempt, MAX_BACKOFF_SHIFT)`.
pub const DEFAULT_MAX_BACKOFF_SHIFT: u32 = 3;

/// End-to-end I/O timeout once the peer is known to be responsive.
pub const DEFAULT_IO_TIMEOUT: Duration = Duration::from_secs(5);

/// Session TTL on the server side.
pub const DEFAULT_SESSION_TTL: Duration = Duration::from_secs(15);

/// Implementation profile. Controls feature set + resource limits + defaults.
#[derive(Clone, Copy, Debug)]
pub enum ProfileKind {
    /// Tiny MCU. Auth only; setup must be out-of-band. Smaller retry/state limits.
    Minimal,
    /// Full client/server on capable hardware. Default.
    Standard,
    /// Gateway / proxy. Enables richer observability + larger state limits.
    Gateway,
}

#[derive(Clone, Copy, Debug)]
pub struct Profile {
    pub kind:                ProfileKind,
    pub retransmit_timeout:  Duration,
    pub max_retries:         usize,
    pub max_backoff_shift:   u32,
    pub io_timeout:          Duration,
    pub session_ttl:         Duration,
    pub max_active_sessions: usize,
    pub max_cached_responses: usize,
}

impl Profile {
    pub const fn standard() -> Self {
        Self {
            kind:                 ProfileKind::Standard,
            retransmit_timeout:   DEFAULT_RETRANSMIT_TIMEOUT,
            max_retries:          DEFAULT_MAX_RETRIES,
            max_backoff_shift:    DEFAULT_MAX_BACKOFF_SHIFT,
            io_timeout:           DEFAULT_IO_TIMEOUT,
            session_ttl:          DEFAULT_SESSION_TTL,
            max_active_sessions:  1024,
            max_cached_responses: 2048,
        }
    }

    pub const fn minimal() -> Self {
        Self {
            kind:                 ProfileKind::Minimal,
            max_retries:          2,
            max_active_sessions:  8,
            max_cached_responses: 16,
            ..Self::standard()
        }
    }

    pub const fn gateway() -> Self {
        Self {
            kind:                 ProfileKind::Gateway,
            max_active_sessions:  8192,
            max_cached_responses: 16384,
            ..Self::standard()
        }
    }
}

impl Default for Profile {
    fn default() -> Self { Self::standard() }
}
