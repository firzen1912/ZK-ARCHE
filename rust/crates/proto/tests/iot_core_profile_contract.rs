use sha2::{Digest, Sha256};
use std::collections::BTreeMap;

const CONTRACT: &str = include_str!("../../../test-vectors/profiles/iot-core-v1.profile");
const EXPECTED_KEYS: &[&str] = &[
    "format",
    "profile_id",
    "name",
    "definition_revision",
    "status",
    "selectable",
    "protocol_version",
    "suite_id",
    "required_selected_capabilities",
    "allowed_selected_capabilities",
    "forbidden_selected_capabilities",
    "authorization_schema",
    "authorization_context_bytes",
    "critical_extensions_policy",
    "channel_binding_policy",
    "max_datagram_bytes",
    "replay_policy",
    "replay_min_entries",
    "replay_epoch_rule",
    "restart_replay_rule",
    "resource_evidence",
    "prescriptive_change_rule",
    "deprecated_selectable",
    "stable_semantics_mutable",
    "contract_sha256",
];

fn parse_contract() -> BTreeMap<&'static str, &'static str> {
    let mut fields = BTreeMap::new();
    for line in CONTRACT.lines() {
        assert!(!line.is_empty(), "blank lines are not canonical");
        let (key, value) = line.split_once('=').expect("key=value line");
        assert!(EXPECTED_KEYS.contains(&key), "unknown profile key: {key}");
        assert!(fields.insert(key, value).is_none(), "duplicate key: {key}");
    }
    assert_eq!(fields.len(), EXPECTED_KEYS.len(), "profile key count drift");
    for key in EXPECTED_KEYS {
        assert!(fields.contains_key(key), "missing profile key: {key}");
    }
    fields
}

fn hex_u64(value: &str) -> u64 {
    let digits = value.strip_prefix("0x").expect("canonical hex prefix");
    u64::from_str_radix(digits, 16).expect("canonical u64 hex")
}

fn profile_body() -> &'static [u8] {
    let marker = "contract_sha256=";
    let offset = CONTRACT.find(marker).expect("fingerprint marker");
    CONTRACT[..offset].as_bytes()
}

fn digest_hex(bytes: &[u8]) -> String {
    Sha256::digest(bytes)
        .iter()
        .map(|byte| format!("{byte:02x}"))
        .collect()
}

#[test]
fn iot_core_profile_fingerprint_and_identity_are_stable() {
    let fields = parse_contract();
    assert_eq!(fields["format"], "ZKPROFILE/1");
    assert_eq!(fields["profile_id"], "0x0001");
    assert_eq!(fields["name"], "iot-core");
    assert_eq!(fields["definition_revision"], "1");
    assert_eq!(fields["status"], "draft");
    assert_eq!(fields["selectable"], "0");

    let actual = digest_hex(profile_body());
    assert_eq!(actual, fields["contract_sha256"]);
    assert_eq!(
        actual,
        "31b53234616189ce470c8c7f2d3d446432bb20953a2f4e5a191fd356a1f54ad4"
    );
}

#[test]
fn iot_core_profile_prescriptive_semantics_are_consistent() {
    let fields = parse_contract();
    assert_eq!(fields["protocol_version"], "0x03");
    assert_eq!(fields["suite_id"], "0x0001");

    let required = hex_u64(fields["required_selected_capabilities"]);
    let allowed = hex_u64(fields["allowed_selected_capabilities"]);
    let forbidden = hex_u64(fields["forbidden_selected_capabilities"]);
    assert_eq!(required, 0x6);
    assert_eq!(allowed, 0x6);
    assert_eq!(required & !allowed, 0, "required must be allowed");
    assert_eq!(allowed & forbidden, 0, "allowed and forbidden overlap");
    assert_eq!(
        allowed | forbidden,
        u64::MAX,
        "capability masks must be total"
    );

    assert_eq!(fields["authorization_schema"], "iot-core-authz-v1");
    assert_eq!(fields["authorization_context_bytes"], "148");
    assert_eq!(fields["critical_extensions_policy"], "none");
    assert_eq!(fields["channel_binding_policy"], "none");
    assert_eq!(fields["max_datagram_bytes"], "2048");
}

#[test]
fn iot_core_profile_remains_fail_closed_while_replay_is_unresolved() {
    let fields = parse_contract();
    assert_eq!(fields["replay_policy"], "unresolved");
    assert_eq!(fields["replay_min_entries"], "unresolved");
    assert_eq!(fields["replay_epoch_rule"], "unresolved");
    assert_eq!(fields["restart_replay_rule"], "unresolved");
    assert_eq!(fields["resource_evidence"], "required-before-stable");
    assert_eq!(fields["selectable"], "0");

    assert_eq!(fields["prescriptive_change_rule"], "new-profile-id");
    assert_eq!(fields["deprecated_selectable"], "0");
    assert_eq!(fields["stable_semantics_mutable"], "0");
}
