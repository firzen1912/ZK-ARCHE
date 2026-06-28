.PHONY: ci install test fuzz-wire fuzz-payloads clean
ci:
	./scripts/ci-python.sh
install:
	python -m pip install -e .[dev]
test:
	python -m pytest -q
fuzz-wire:
	python fuzz/fuzz_wire.py
fuzz-payloads:
	python fuzz/fuzz_payloads.py
clean:
	rm -rf .pytest_cache build dist *.egg-info .mypy_cache .ruff_cache
	find . -type d -name __pycache__ -exec rm -rf {} +
