FROM python:3.12-slim@sha256:2c941e860699f878900b0edc2403613c234d4b32eda3cc9fa7036991a2a63c4a
RUN pip install --no-cache-dir uv==0.12.23
COPY vendor/upstream/gears /opt/peykar/vendor/upstream/gears
COPY runtime_profiles/gears/pyproject.toml runtime_profiles/gears/uv.lock /opt/peykar/runtime_profiles/gears/
WORKDIR /opt/peykar/runtime_profiles/gears
RUN uv sync --frozen --no-dev --no-editable
COPY runtime_profiles/gears/*.py /opt/runtime/
ENV PATH="/opt/peykar/runtime_profiles/gears/.venv/bin:$PATH" \
    OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 NUMBA_NUM_THREADS=1 \
    NUMBA_CACHE_DIR=/tmp/numba MPLCONFIGDIR=/tmp/mpl
WORKDIR /work
USER 10001:10001
ENTRYPOINT ["/opt/peykar/runtime_profiles/gears/.venv/bin/python", "/opt/runtime/probe.py"]
