use proto::auth_v3::{derive_kc_keys_v3, finished_tag_v3};

fn bytes32(hex_value: &str) -> [u8; 32] {
    hex::decode(hex_value).unwrap().try_into().unwrap()
}

#[test]
fn auth_v3_finished_rejects_cross_direction_reflection() {
    let session_key =
        bytes32("5cea979c840f9cb1302db41f7dcfe91c4f8b22f7019b0586db183219e27ef348");
    let th = bytes32("e2b85befd4f3f58b5e880673ce1b27e81de875bf4443d7af6971d811e10439d2");
    let expected_server =
        bytes32("453edbad5976c5c08e720e6bffb0e111eb117501f26afb0f55b54cc719e38096");
    let expected_client =
        bytes32("6c748a2e93e297095be36ba757c66bf886ec11fb9833ffdab7cbdcf5aa75d819");

    let (k_s2c, k_c2s, _) = derive_kc_keys_v3(&session_key, &th);
    let server_tag = finished_tag_v3(&k_s2c, b"server finished v3", &th);
    let client_tag = finished_tag_v3(&k_c2s, b"client finished v3", &th);

    assert_eq!(server_tag, expected_server);
    assert_eq!(client_tag, expected_client);

    // A verbatim Finished value from either direction must not satisfy the
    // opposite-direction expectation.
    assert_ne!(server_tag, expected_client);
    assert_ne!(client_tag, expected_server);

    // Keep the two independent direction separators covered: even if an
    // attacker presents the target role label, using the source-direction key
    // still cannot produce the target-direction Finished value.
    let server_key_with_client_label =
        finished_tag_v3(&k_s2c, b"client finished v3", &th);
    let client_key_with_server_label =
        finished_tag_v3(&k_c2s, b"server finished v3", &th);
    assert_ne!(server_key_with_client_label, expected_client);
    assert_ne!(client_key_with_server_label, expected_server);
}
