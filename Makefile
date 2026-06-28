.PHONY: ci rust-ci c-ci python-ci c-vectors clean

ci:
	./scripts/ci-all.sh

rust-ci:
	./scripts/ci-rust.sh

c-ci:
	./scripts/ci-c.sh

python-ci:
	./scripts/ci-python.sh

c-vectors:
	cd c && ./build/tests/test_vectors ../rust/test-vectors/0x0001

clean:
	cd rust && cargo clean || true
	cd c && make clean || true
	rm -rf python/.pytest_cache python/build python/dist python/*.egg-info
	rm -rf evidence/*.log
