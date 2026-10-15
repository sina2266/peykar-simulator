# HTTP application

`uv run uvicorn apps.api.main:create_app --factory` or `uv run peykar serve`.
Set PEYKAR_API_TOKEN first. See docs/operations.md and schemas/openapi.json.
App lifespan creates local infrastructure. Simulation jobs execute in a worker.
