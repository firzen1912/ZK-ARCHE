#![no_main]
use libfuzzer_sys::fuzz_target;
use proto::wire::{parse_packet, TlvIter};

fuzz_target!(|data: &[u8]| {
    let _ = parse_packet(data);
    let mut iter = TlvIter::new(data);
    while let Some(item) = iter.next() {
        let _ = item;
    }
});
