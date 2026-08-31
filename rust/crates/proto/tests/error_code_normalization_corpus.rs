use proto::{ErrorCode, ProtoError};

const CORPUS: &str =
    include_str!("../../../test-vectors/wire/error-code-normalization-v1.txt");

fn parse_hex_u16(value: &str) -> u16 {
    u16::from_str_radix(value, 16).unwrap_or_else(|_| panic!("bad u16 hex {value}"))
}

#[test]
fn shared_error_code_normalization_corpus_is_enforced() {
    assert!(CORPUS.lines().any(|line| line == "version=1"));
    let mut count = 0usize;

    for line in CORPUS.lines().filter(|line| line.starts_with("case=")) {
        let fields: Vec<&str> = line.trim_start_matches("case=").split('|').collect();
        assert_eq!(fields.len(), 3, "{}", fields[0]);

        let received = parse_hex_u16(fields[1]);
        let expected = parse_hex_u16(fields[2]);
        assert_eq!(ErrorCode::from_u16(received).as_u16(), expected, "{}", fields[0]);

        let bytes = received.to_le_bytes();
        let payload = [bytes[0], bytes[1], b'x'];
        match ProtoError::from_wire_payload(&payload) {
            ProtoError::Wire { code, msg } => {
                assert_eq!(code.as_u16(), expected, "{}", fields[0]);
                assert_eq!(msg, "x", "{}", fields[0]);
            }
            other => panic!("{}: unexpected decoded error {other:?}", fields[0]),
        }
        count += 1;
    }

    assert_eq!(count, 35);
}

#[test]
fn short_error_payload_remains_malformed() {
    match ProtoError::from_wire_payload(&[0x03]) {
        ProtoError::Wire { code, .. } => assert_eq!(code, ErrorCode::MalformedPacket),
        other => panic!("unexpected decoded error {other:?}"),
    }
}
