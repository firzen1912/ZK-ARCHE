//! Canonical TLS exporter context encoding for draft ZK-ARCHE-BIND.
//!
//! This module constructs only the 32-byte SHA-256 context required by
//! `spec/tls-exporter-channel-binding.md`. It does not perform TLS, validate
//! certificates, create authorization, or make TLS a Common Contract dependency.

use sha2::{Digest, Sha256};

const DOMAIN: &[u8] = b"ZKARCHE-EXPORTER-CONTEXT-v1";

#[derive(Clone, Copy, Debug)]
pub struct TlsExporterContext<'a> {
    pub application_id: &'a [u8],
    pub alpn: &'a [u8],
    pub deployment_id: &'a [u8],
    pub initiator_id: &'a [u8],
    pub responder_id: &'a [u8],
    pub protocol_version: u16,
    pub suite_id: u16,
    pub profile_id: u16,
    pub auth_instance_id: &'a [u8],
    pub auth_transcript_hash: &'a [u8],
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ExporterContextError {
    EmptyRequiredField,
    EmptyAlpnNotPermitted,
    FieldTooLong,
}

fn update_len_field(
    hash: &mut Sha256,
    value: &[u8],
) -> core::result::Result<(), ExporterContextError> {
    let len = u16::try_from(value.len()).map_err(|_| ExporterContextError::FieldTooLong)?;
    hash.update(len.to_be_bytes());
    hash.update(value);
    Ok(())
}

/// Build `ZKARCHE-EXPORTER-CONTEXT-v1` exactly as specified.
///
/// `allow_empty_alpn` is a transport-profile decision; all other byte-string
/// fields are mandatory and non-empty. All variable fields are bounded by the
/// canonical u16 length prefix before hashing.
pub fn tls_exporter_context_digest(
    ctx: &TlsExporterContext<'_>,
    allow_empty_alpn: bool,
) -> core::result::Result<[u8; 32], ExporterContextError> {
    if ctx.application_id.is_empty()
        || ctx.deployment_id.is_empty()
        || ctx.initiator_id.is_empty()
        || ctx.responder_id.is_empty()
        || ctx.auth_instance_id.is_empty()
        || ctx.auth_transcript_hash.is_empty()
    {
        return Err(ExporterContextError::EmptyRequiredField);
    }
    if ctx.alpn.is_empty() && !allow_empty_alpn {
        return Err(ExporterContextError::EmptyAlpnNotPermitted);
    }

    let mut hash = Sha256::new();
    hash.update(DOMAIN);
    update_len_field(&mut hash, ctx.application_id)?;
    update_len_field(&mut hash, ctx.alpn)?;
    update_len_field(&mut hash, ctx.deployment_id)?;
    update_len_field(&mut hash, ctx.initiator_id)?;
    update_len_field(&mut hash, ctx.responder_id)?;
    hash.update(ctx.protocol_version.to_be_bytes());
    hash.update(ctx.suite_id.to_be_bytes());
    hash.update(ctx.profile_id.to_be_bytes());
    update_len_field(&mut hash, ctx.auth_instance_id)?;
    update_len_field(&mut hash, ctx.auth_transcript_hash)?;

    Ok(hash.finalize().into())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn fixture<'a>(transcript: &'a [u8]) -> TlsExporterContext<'a> {
        TlsExporterContext {
            application_id: b"zk-arche",
            alpn: b"zkarche/1",
            deployment_id: b"lab",
            initiator_id: &[0x01, 0x02],
            responder_id: &[0x03, 0x04],
            protocol_version: 3,
            suite_id: 1,
            profile_id: 2,
            auth_instance_id: b"auth-0001",
            auth_transcript_hash: transcript,
        }
    }

    #[test]
    fn matches_shared_fixture() {
        let transcript: Vec<u8> = (0u8..32).collect();
        let got = tls_exporter_context_digest(&fixture(&transcript), false).unwrap();
        let expected = hex::decode("a3ba0867ead3c60416f1a68596566f01739b3520b41422a3cda7c039b280c107").unwrap();
        assert_eq!(got.as_slice(), expected.as_slice());
    }

    #[test]
    fn security_context_changes_are_domain_separated() {
        let transcript: Vec<u8> = (0u8..32).collect();
        let base = fixture(&transcript);
        let first = tls_exporter_context_digest(&base, false).unwrap();

        let other_application = TlsExporterContext {
            application_id: b"zk-arche-alt",
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_application, false).unwrap());

        let other_alpn = TlsExporterContext {
            alpn: b"zkarche/2",
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_alpn, false).unwrap());

        let other_initiator = TlsExporterContext {
            initiator_id: &[0x05, 0x06],
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_initiator, false).unwrap());

        let other_responder = TlsExporterContext {
            responder_id: &[0x07, 0x08],
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_responder, false).unwrap());

        let swapped = TlsExporterContext {
            initiator_id: base.responder_id,
            responder_id: base.initiator_id,
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&swapped, false).unwrap());

        let other_version = TlsExporterContext {
            protocol_version: 4,
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_version, false).unwrap());

        let second_instance = TlsExporterContext {
            auth_instance_id: b"auth-0002",
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&second_instance, false).unwrap());

        let other_deployment = TlsExporterContext {
            deployment_id: b"field",
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_deployment, false).unwrap());

        let other_suite = TlsExporterContext {
            suite_id: 2,
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_suite, false).unwrap());

        let other_profile = TlsExporterContext {
            profile_id: 3,
            ..base
        };
        assert_ne!(first, tls_exporter_context_digest(&other_profile, false).unwrap());

        let mut changed_transcript = transcript.clone();
        changed_transcript[31] ^= 0x01;
        let other_transcript = fixture(&changed_transcript);
        assert_ne!(first, tls_exporter_context_digest(&other_transcript, false).unwrap());
    }

    #[test]
    fn rejects_invalid_required_fields_and_alpn_policy() {
        let transcript: Vec<u8> = (0u8..32).collect();
        let base = fixture(&transcript);
        let empty_application = TlsExporterContext { application_id: b"", ..base };
        assert_eq!(
            tls_exporter_context_digest(&empty_application, false),
            Err(ExporterContextError::EmptyRequiredField)
        );
        let empty_alpn = TlsExporterContext { alpn: b"", ..base };
        assert_eq!(
            tls_exporter_context_digest(&empty_alpn, false),
            Err(ExporterContextError::EmptyAlpnNotPermitted)
        );
        assert!(tls_exporter_context_digest(&empty_alpn, true).is_ok());
    }
}
