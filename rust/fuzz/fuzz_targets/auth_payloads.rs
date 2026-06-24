#![no_main]
use libfuzzer_sys::fuzz_target;
use proto::proto::payloads::{
    decode_ack, Auth1, Auth2, Auth3, Setup1, Setup2, Setup3,
};

fuzz_target!(|data: &[u8]| {
    let _ = Setup1::decode(data);
    let _ = Setup2::decode(data);
    let _ = Setup3::decode(data);
    let _ = Auth1::decode(data);
    let _ = Auth2::decode(data);
    let _ = Auth3::decode(data);
    let _ = decode_ack(data);
});
