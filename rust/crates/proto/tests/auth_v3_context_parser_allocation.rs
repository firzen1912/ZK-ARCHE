use proto::auth_v3_context_parser::{parse_canonical_context, ContextParseError};
use std::alloc::{GlobalAlloc, Layout, System};
use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering};

struct TrackingAllocator;

static TRACK_ALLOCATIONS: AtomicBool = AtomicBool::new(false);
static ALLOCATION_CALLS: AtomicUsize = AtomicUsize::new(0);

fn record_allocation() {
    if TRACK_ALLOCATIONS.load(Ordering::Relaxed) {
        ALLOCATION_CALLS.fetch_add(1, Ordering::Relaxed);
    }
}

unsafe impl GlobalAlloc for TrackingAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        record_allocation();
        unsafe { System.alloc(layout) }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        unsafe { System.dealloc(ptr, layout) }
    }

    unsafe fn alloc_zeroed(&self, layout: Layout) -> *mut u8 {
        record_allocation();
        unsafe { System.alloc_zeroed(layout) }
    }

    unsafe fn realloc(&self, ptr: *mut u8, layout: Layout, new_size: usize) -> *mut u8 {
        record_allocation();
        unsafe { System.realloc(ptr, layout, new_size) }
    }
}

#[global_allocator]
static GLOBAL_ALLOCATOR: TrackingAllocator = TrackingAllocator;

#[test]
fn hostile_entry_count_is_rejected_before_heap_materialization() {
    // ZKCTX v1 Authorization header declaring 65,535 entries but carrying no
    // entry bytes. The structural lower bound is checked before Vec capacity is
    // derived from the attacker-controlled count, so this rejection must not
    // allocate or reallocate through the test process's global allocator.
    let input = [b'Z', b'K', b'C', b'T', b'X', 1, 1, 0xff, 0xff];

    ALLOCATION_CALLS.store(0, Ordering::SeqCst);
    TRACK_ALLOCATIONS.store(true, Ordering::SeqCst);
    let result = parse_canonical_context(&input);
    TRACK_ALLOCATIONS.store(false, Ordering::SeqCst);
    let allocation_calls = ALLOCATION_CALLS.load(Ordering::SeqCst);

    assert_eq!(result.err(), Some(ContextParseError::Truncated));
    assert_eq!(
        allocation_calls, 0,
        "hostile entry_count reached heap materialization before structural rejection"
    );
}
