# The vcell-mbsolver runtime image: a slim distro base plus the release archive's contents
# (SOLVER-RELEASE in README.md). The executable is built by .github/workflows/release.yml
# (manylinux_2_28, HDF5 and libcurl static, messaging ON), unpacked into the build context at
# dist/linux-<arch>/ — this Dockerfile compiles nothing, so both architectures build without
# emulation.
#
#   docker build -f docker/Dockerfile --platform linux/amd64 -t vcell-mbsolver .
FROM debian:bookworm-slim

ARG TARGETARCH
LABEL org.opencontainers.image.source="https://github.com/virtualcell/vcell-mbsolver" \
      org.opencontainers.image.description="VCell moving-boundary solver (MovingBoundary_x64)" \
      org.opencontainers.image.licenses="MIT"

COPY dist/linux-${TARGETARCH}/ /opt/vcell/bin/
COPY --chmod=755 docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint

ENV PATH=/opt/vcell/bin:${PATH}
WORKDIR /tmp

ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
