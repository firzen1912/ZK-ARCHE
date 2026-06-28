//! Protocol state machines (Layer A).
//!
//! These are pure protocol drivers: they call a `ClientTransport` / `Transport`
//! to exchange framed packets, a `CredentialStore` / `RegistryStore` to load
//! and save long-term state, and the `crypto` module for primitives. They are
//! the same regardless of whether the underlying transport is UDP, TCP, CoAP,
//! BLE, or an in-process channel.
//!
//! Each flow is expressed as a "run-to-completion" function. A true
//! state-machine-per-packet style (returning `Action`s that the caller
//! executes) is an easy refactor from here but is not required for the
//! transport-abstraction goal.

pub mod auth;
pub(crate) mod common;
pub mod payloads;
pub mod setup;
