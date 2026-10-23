FROM node:22.14.0-bookworm-slim AS dashboard
WORKDIR /app/apps/dashboard
COPY apps/dashboard/package*.json ./
RUN npm ci --ignore-scripts --no-fund --no-audit
COPY apps/dashboard ./
COPY schemas /app/schemas
RUN npm run check && npm run build

FROM python:3.12-slim AS build
RUN pip install --no-cache-dir uv==0.12.23
WORKDIR /app
COPY pyproject.toml uv.lock Makefile LICENSE NOTICE LICENSING.md README.md README.fa.md ./
COPY packages packages
COPY scripts scripts
COPY tests tests
COPY experiments experiments
COPY schemas schemas
COPY model_packages model_packages
COPY licenses licenses
COPY docs docs
COPY engine_adapters engine_adapters
COPY biological_libraries biological_libraries
COPY dataset_connectors dataset_connectors
COPY runtime_profiles runtime_profiles
COPY --from=dashboard /app/packages/platform/src/peykar/web_assets packages/platform/src/peykar/web_assets
RUN uv sync --frozen --all-packages --no-dev --no-editable

FROM build AS verification
COPY vendor/models/vcornea-history-b6123adf vendor/models/vcornea-history-b6123adf
COPY --from=dashboard /usr/local/bin/node /usr/local/bin/node
COPY --from=dashboard /usr/local/lib/node_modules/npm /usr/local/lib/node_modules/npm
COPY --from=dashboard /app/apps/dashboard apps/dashboard
RUN apt-get update && apt-get install -y --no-install-recommends make git libstdc++6 \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s /usr/local/lib/node_modules/npm/bin/npm-cli.js /usr/local/bin/npm
RUN uv sync --frozen --all-packages --group dev --no-editable
ENV UV_NO_SYNC=1 UV_OFFLINE=1
CMD ["make", "check", "build"]

FROM python:3.12-slim AS runtime
WORKDIR /app
COPY --from=build /app/.venv /app/.venv
COPY --from=build /app/uv.lock /app/uv.lock
ENV PATH="/app/.venv/bin:$PATH" PYTHONUNBUFFERED=1 PYTHONDONTWRITEBYTECODE=1 \
    PEYKAR_DATA_DIR=/var/lib/peykar
RUN groupadd --gid 10001 peykar && useradd --uid 10001 --gid 10001 --no-create-home peykar \
    && mkdir -p /var/lib/peykar && chown peykar:peykar /var/lib/peykar
USER 10001:10001
EXPOSE 8000
HEALTHCHECK --interval=30s --timeout=3s CMD python -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:8000/healthz',timeout=2)"
CMD ["peykar", "serve", "--host", "0.0.0.0"]
