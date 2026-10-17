# vcell-stochastic solver image: VCellStoch_x64 built with messaging ON (so a trailing -tid <n>
# reports status to VCell's broker), on a slim runtime. Multi-arch (linux/amd64, linux/arm64).
#
#   docker build -f docker/Dockerfile -t vcell-stochastic .
#   docker run --rm vcell-stochastic                       # version + executables
#   docker run --rm -v $PWD:/simdata vcell-stochastic VCellStoch_x64 gibson /simdata/m.stochInput /simdata/m.ida
#
# The build stage is the same manylinux_2_28 toolchain and static HDF5 as the linux64 release
# archive (ci/build-hdf5.sh, ci/build.sh); the only difference is OPTION_TARGET_MESSAGING=ON,
# which adds libcurl. The runtime stage matches the build's distro (AlmaLinux 8) so the
# system libcurl it links against is the one it runs with.

ARG MANYLINUX_TAG=2026.09.23-1

FROM quay.io/pypa/manylinux_2_28_x86_64:${MANYLINUX_TAG} AS build-amd64
FROM quay.io/pypa/manylinux_2_28_aarch64:${MANYLINUX_TAG} AS build-arm64

FROM build-${TARGETARCH} AS build
ARG VCELLSTOCH_VERSION=dev
RUN dnf install -y libcurl-devel && dnf clean all \
 && /opt/python/cp312-cp312/bin/pip install --no-cache-dir cmake ninja
ENV PATH=/opt/python/cp312-cp312/bin:${PATH}
RUN ln -sf /opt/python/cp312-cp312/bin/ninja /usr/local/bin/ninja
WORKDIR /src
COPY ci/build-hdf5.sh ci/
RUN HDF5_WORK_DIR=/tmp/hdf5 ci/build-hdf5.sh /opt/hdf5 && rm -rf /tmp/hdf5
COPY . .
RUN VCELLSTOCH_VERSION=${VCELLSTOCH_VERSION} BUILD_DIR=/tmp/build \
    ci/build.sh /opt/hdf5 /stage -DOPTION_TARGET_MESSAGING=ON \
 && /stage/VCellStoch_x64 2>&1 | grep -q -- '-tid' \
 && echo "messaging build confirmed (usage offers -tid)"

FROM almalinux:8-minimal AS runtime
RUN microdnf install -y libcurl libstdc++ && microdnf clean all
COPY --from=build /stage/VCellStoch_x64 /usr/local/bin/VCellStoch_x64
COPY --from=build /stage/LICENSE /stage/COPYING-HDF5 /stage/VERSION /usr/local/share/vcell-stochastic/
COPY docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint
# No WORKDIR/ENV: the solver writes only the output paths it is given (/simdata/... under
# VCell's Slurm jobs), so the image runs as any uid, read-only, under --containall.
ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
