.PHONY: ci test fmt clippy fuzz-wire fuzz-auth clean
ci:
	bash ./scripts/ci-rust.sh
fmt:
	cargo fmt --all -- --check
test:
	cargo test --workspace --locked --all-features
clippy:
	cargo clippy --workspace --all-targets --all-features -- -D warnings
fuzz-wire:
	cargo fuzz run wire_parse -- -max_total_time=300
fuzz-auth:
	cargo fuzz run auth_payloads -- -max_total_time=300
clean:
	cargo clean
