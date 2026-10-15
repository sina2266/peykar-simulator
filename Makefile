.PHONY: install check check-docker test demo api worker schemas build

install:
	uv sync --frozen --all-packages --group dev

check:
	uv run ruff check .
	uv run ruff format --check .
	uv run mypy
	uv run pytest --cov --cov-report=term-missing
	uv run python scripts/export_contracts.py --check

check-docker:
	docker build --target verification -t peykar-check .
	docker run --rm --network none peykar-check

test:
	uv run pytest

demo:
	uv run peykar demo

api:
	uv run peykar serve

worker:
	uv run peykar worker

schemas:
	uv run python scripts/export_contracts.py

build:
	uv build --all-packages
