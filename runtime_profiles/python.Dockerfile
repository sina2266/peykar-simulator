FROM python:3.12-slim@sha256:2c941e860699f878900b0edc2403613c234d4b32eda3cc9fa7036991a2a63c4a
RUN pip install --no-cache-dir uv==0.12.23
ARG PROFILE
COPY runtime_profiles/${PROFILE}/pyproject.toml runtime_profiles/${PROFILE}/uv.lock /opt/runtime/
WORKDIR /opt/runtime
RUN uv sync --frozen --no-dev --no-editable
COPY runtime_profiles/${PROFILE}/*.py /opt/runtime/
ENV PATH="/opt/runtime/.venv/bin:$PATH" MPLCONFIGDIR=/tmp/mpl \
    OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1
WORKDIR /work
USER 10001:10001
ENTRYPOINT ["/opt/runtime/.venv/bin/python", "/opt/runtime/probe.py"]
