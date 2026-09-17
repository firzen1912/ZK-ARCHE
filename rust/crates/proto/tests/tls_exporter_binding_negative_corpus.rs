use zk_arche_proto::transport::tls_exporter_context::{
    tls_exporter_context_digest, TlsExporterContext,
};

const CORPUS: &str = include_str!(
    "../../../test-vectors/transport/tls-exporter-binding-negatives-v1.txt"
);

fn base<'a>(transcript: &'a [u8]) -> TlsExporterContext<'a> {
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
fn consumes_tls_exporter_binding_negative_corpus() {
    let transcript: Vec<u8> = (0u8..32).collect();
    let base_ctx = base(&transcript);
    let base_digest = tls_exporter_context_digest(&base_ctx, false).unwrap();

    let mut seen = Vec::new();
    for line in CORPUS.lines().filter(|line| !line.is_empty() && !line.starts_with('#')) {
        let fields: Vec<_> = line.split('|').collect();
        assert_eq!(fields.len(), 4, "malformed corpus row: {line}");
        let id = fields[0];
        let mutation = fields[1];
        let expected_relation = fields[2];
        seen.push(id);

        let mutated = match mutation {
            "none" | "transport_address=changed" | "routing_metadata=changed" => base_ctx,
            "auth_instance_id=auth-0002" => TlsExporterContext {
                auth_instance_id: b"auth-0002",
                ..base_ctx
            },
            "auth_instance_id=auth-resumed-0001" => TlsExporterContext {
                auth_instance_id: b"auth-resumed-0001",
                ..base_ctx
            },
            "application_id=zk-arche-alt" => TlsExporterContext {
                application_id: b"zk-arche-alt",
                ..base_ctx
            },
            "initiator_id=0506" => TlsExporterContext {
                initiator_id: &[0x05, 0x06],
                ..base_ctx
            },
            "responder_id=0708" => TlsExporterContext {
                responder_id: &[0x07, 0x08],
                ..base_ctx
            },
            "profile_id=0003" => TlsExporterContext {
                profile_id: 3,
                ..base_ctx
            },
            other => panic!("unhandled corpus mutation {other} in {id}"),
        };

        let digest = tls_exporter_context_digest(&mutated, false).unwrap();
        match expected_relation {
            "equal_base" => assert_eq!(digest, base_digest, "{id} must preserve context digest"),
            "different_base" => assert_ne!(digest, base_digest, "{id} must separate context digest"),
            other => panic!("unknown expected relation {other} in {id}"),
        }
    }

    assert_eq!(
        seen,
        vec![
            "TB-N01", "TB-N02", "TB-N03", "TB-N04", "TB-N05", "TB-N06", "TB-N07",
            "TB-N08", "TB-N09", "TB-N10",
        ],
        "the consumer must cover the complete governed TB-N01..TB-N10 corpus"
    );
}
