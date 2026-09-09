#![no_main]
use libfuzzer_sys::fuzz_target;
use proto::proto::payloads::{
    decode_ack, encode_ack, Auth1, Auth2, Auth3, Setup1, Setup2, Setup3,
};

fn assert_canonical_roundtrip<T, Decode, Encode>(data: &[u8], decode: Decode, encode: Encode)
where
    Decode: Fn(&[u8]) -> proto::Result<T>,
    Encode: Fn(&T) -> Vec<u8>,
{
    if let Ok(value) = decode(data) {
        let canonical = encode(&value);
        let reparsed = decode(&canonical).expect("accepted payload must decode after canonical encode");
        assert_eq!(
            encode(&reparsed),
            canonical,
            "accepted payload must have a stable canonical encoding"
        );
    }
}

fuzz_target!(|data: &[u8]| {
    assert_canonical_roundtrip(data, Setup1::decode, Setup1::encode);
    assert_canonical_roundtrip(data, Setup2::decode, Setup2::encode);
    assert_canonical_roundtrip(data, Setup3::decode, Setup3::encode);
    assert_canonical_roundtrip(data, Auth1::decode, Auth1::encode);
    assert_canonical_roundtrip(data, Auth2::decode, Auth2::encode);
    assert_canonical_roundtrip(data, Auth3::decode, Auth3::encode);

    if decode_ack(data).is_ok() {
        let canonical = encode_ack();
        assert!(decode_ack(&canonical).is_ok());
        assert_eq!(canonical, vec![0x01]);
    }
});
