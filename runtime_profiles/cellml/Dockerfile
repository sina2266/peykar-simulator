FROM python:3.12-slim@sha256:2c941e860699f878900b0edc2403613c234d4b32eda3cc9fa7036991a2a63c4a
# Official libCellML wheels dynamically link libxml2; keep this explicit.
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxml2=2.12.7+dfsg+really2.9.14-2.1+deb13u3 \
    && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir uv==0.12.23
COPY runtime_profiles/cellml/pyproject.toml runtime_profiles/cellml/uv.lock /opt/runtime/
WORKDIR /opt/runtime
RUN uv sync --frozen --no-dev --no-editable
COPY runtime_profiles/cellml/probe.py /opt/runtime/probe.py
ENV PATH="/opt/runtime/.venv/bin:$PATH" OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1
WORKDIR /work
USER 10001:10001
ENTRYPOINT ["/opt/runtime/.venv/bin/python", "/opt/runtime/probe.py"]
