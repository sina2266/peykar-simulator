FROM --platform=linux/amd64 mambaorg/micromamba:2.3.3@sha256:e583f8e151fd1ebbc78b05cf4feb37ddbb518892dbabf6c0c03ea8ad613bc519

LABEL org.opencontainers.image.title="Peykar isolated CompuCell3D 4.7.0 runtime" \
      org.opencontainers.image.version="4.7.0-py312hb93dfd8_5" \
      org.opencontainers.image.description="Pinned upstream runtime; no biological model bundled"

COPY --chown=$MAMBA_USER:$MAMBA_USER conda-lock.yml /tmp/cc3d-lock.yml
COPY --chown=$MAMBA_USER:$MAMBA_USER preserve_notices.py /tmp/preserve_notices.py
RUN micromamba create --yes --name cc3d --no-pyc --file /tmp/cc3d-lock.yml \
    && /opt/conda/envs/cc3d/bin/python /tmp/preserve_notices.py \
    && micromamba clean --all --yes

COPY --chown=$MAMBA_USER:$MAMBA_USER package-inventory.json LICENSE.cc3d.txt probe.py /opt/conda/cc3d-runtime/
ENV ENV_NAME=cc3d \
    PATH=/opt/conda/envs/cc3d/bin:$PATH \
    QT_QPA_PLATFORM=offscreen \
    OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 MKL_NUM_THREADS=1 \
    PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1 \
    HOME=/tmp
WORKDIR /tmp
CMD ["python", "-m", "cc3d.run_script", "--help"]
