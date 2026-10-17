# syntax=docker/dockerfile:1.7
#
# vcell-nfsim solver image — ghcr.io/virtualcell/vcell-nfsim:<X.Y.Z> (and the Apptainer SIF
# ghcr.io/virtualcell/vcell-nfsim_singularity:<X.Y.Z> built from it). See SOLVER-RELEASE.md.
#
#     docker build -f docker/Dockerfile --build-arg VERSION=0.2.1 -t vcell-nfsim .
#
# Stage "build" produces exactly the contents of the linux64.tgz / linux64arm.tgz release archive
# (packaging/build-linux.sh on manylinux_2_28); stage "archive" exports them as that tarball
# (`docker buildx build --target archive --output type=local,dest=out .`); the final stage is a
# slim distro base with those same files on PATH.

# ---------------------------------------------------------------------------------------------
FROM quay.io/pypa/manylinux_2_28_x86_64 AS build-amd64
FROM quay.io/pypa/manylinux_2_28_aarch64 AS build-arm64
FROM build-${TARGETARCH} AS build
ARG VERSION=0.0.0-dev
COPY . /src
RUN /src/packaging/build-linux.sh /src /opt/vcell-nfsim "${VERSION}"

# ---------------------------------------------------------------------------------------------
FROM build AS package
ARG TARGETARCH
RUN name=$([ "${TARGETARCH}" = arm64 ] && echo linux64arm || echo linux64) && \
    mkdir /dist && tar -C /opt/vcell-nfsim -czf "/dist/${name}.tgz" . && ls -l /dist

FROM scratch AS archive
COPY --from=package /dist/ /

# ---------------------------------------------------------------------------------------------
FROM debian:bookworm-slim AS runtime
ARG VERSION=0.0.0-dev
LABEL org.opencontainers.image.source="https://github.com/virtualcell/vcell-nfsim" \
      org.opencontainers.image.description="NFsim (NFsim_x64) for the Virtual Cell, with VCell messaging" \
      org.opencontainers.image.version="${VERSION}"

COPY --from=build /opt/vcell-nfsim/ /opt/vcell-nfsim/
COPY docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint
RUN chmod 0755 /usr/local/bin/vcell-solver-entrypoint /opt/vcell-nfsim/NFsim_x64 && \
    /opt/vcell-nfsim/NFsim_x64 -help | grep -q 'starting NFsim'

ENV PATH=/opt/vcell-nfsim:${PATH} \
    VCELL_SOLVER_HOME=/opt/vcell-nfsim
WORKDIR /tmp
ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
