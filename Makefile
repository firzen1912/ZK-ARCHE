.PHONY: ci release-qualification release-qual rust-ci c-ci python-ci c-vectors clean

ci:
	bash ./scripts/ci-all.sh

release-qualification:
	bash ./scripts/ci-release-qualification.sh

release-qual: release-qualification

rust-ci:
	bash ./scripts/ci-rust.sh

c-ci:
	bash ./scripts/ci-c.sh

python-ci:
	bash ./scripts/ci-python.sh

c-vectors:
	cd c && ./build/tests/test_vectors ../rust/test-vectors/0x0001

clean:
	cd rust && cargo clean || true
	cd c && make clean || true
	rm -rf python/.pytest_cache python/build python/dist python/*.egg-info
	rm -rf evidence/*.log
