.PHONY: ci install test clean
ci:
	./scripts/ci-python.sh
install:
	python -m pip install -e .[dev]
test:
	python -m pytest -q
clean:
	rm -rf .pytest_cache build dist *.egg-info .mypy_cache .ruff_cache
	find . -type d -name __pycache__ -exec rm -rf {} +
