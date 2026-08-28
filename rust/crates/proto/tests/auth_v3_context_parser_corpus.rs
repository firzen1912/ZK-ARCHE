use proto::auth_v3_context_parser::{
    hash_canonical_context_bytes, parse_canonical_context, ContextParseError,
};

const CORPUS: &str = include_str!("../../../test-vectors/auth-v3/context-parser-negative-v1.txt");

fn expected_error(token: &str) -> ContextParseError {
    match token {
        "TRUNCATED" => ContextParseError::Truncated,
        "INVALID_MAGIC" => ContextParseError::InvalidMagic,
        "UNSUPPORTED_VERSION" => ContextParseError::UnsupportedVersion,
        "UNKNOWN_KIND" => ContextParseError::UnknownKind,
        "INVALID_ID" => ContextParseError::InvalidId,
        "NON_CANONICAL_ORDER" => ContextParseError::NonCanonicalOrder,
        "INVALID_CRITICAL_ID" => ContextParseError::InvalidCriticalId,
        "NONZERO_FLAGS" => ContextParseError::NonZeroFlags,
        "TRAILING_BYTES" => ContextParseError::TrailingBytes,
        other => panic!("unknown parser corpus error token: {other}"),
    }
}

#[test]
fn rust_consumes_shared_raw_negative_parser_corpus() {
    let mut case_count = 0usize;

    for (line_index, raw_line) in CORPUS.lines().enumerate() {
        let line = raw_line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }

        let mut fields = line.split('|');
        let name = fields.next().expect("corpus case name");
        let expected_token = fields.next().expect("corpus expected error");
        let hex_bytes = fields.next().expect("corpus input hex");
        assert!(
            fields.next().is_none(),
            "unexpected extra field on corpus line {}",
            line_index + 1
        );

        let input = hex::decode(hex_bytes).expect("canonical corpus hex");
        let expected = expected_error(expected_token);

        assert_eq!(
            parse_canonical_context(&input).err(),
            Some(expected),
            "parse decision mismatch for shared corpus case {name}"
        );
        assert_eq!(
            hash_canonical_context_bytes(&input).err(),
            Some(expected),
            "hash admission mismatch for shared corpus case {name}"
        );

        case_count += 1;
    }

    assert_eq!(case_count, 12, "shared parser corpus case-count drift");
}
