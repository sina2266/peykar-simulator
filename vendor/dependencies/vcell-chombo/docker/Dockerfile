# syntax=docker/dockerfile:1.7
#
# vcell-chombo runtime image: the Linux release archive on a slim distro base,
# behind the standard VCell solver entry point (SOLVER-RELEASE.md). There is no
# build stage on purpose -- the image carries exactly the binaries the release
# archive does, which were built and checked on a manylinux_2_28 base by
# .github/workflows/release.yml. Build context: a directory holding the
# unpacked archive in dist/ and this docker/ directory:
#
#     tar -xzf linux64.tgz -C dist/      (linux64arm.tgz on arm64)
#     docker build -f docker/Dockerfile -t vcell-chombo .
#
# The archive bundles its GCC runtime (libgfortran, libquadmath) next to the
# executables with RUNPATH=$ORIGIN; the base supplies glibc, libstdc++ and
# libgcc_s, which the archive deliberately does not ship.

ARG BASE=debian:bookworm-slim
FROM ${BASE}

COPY dist/ /opt/vcell-chombo/
COPY docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint

# HDF5_USE_FILE_LOCKING: the solver writes HDF5 onto bind mounts (NFS on the
# cluster, host shares on a desktop), where HDF5's default locking fails.
ENV PATH=/opt/vcell-chombo:${PATH} \
    HDF5_USE_FILE_LOCKING=FALSE

# Fail the build, not the first job, if a library does not resolve here.
RUN chmod 0755 /usr/local/bin/vcell-solver-entrypoint \
 && for exe in VCellChombo2D_x64 VCellChombo3D_x64; do \
        if ldd "/opt/vcell-chombo/$exe" | grep 'not found'; then exit 1; fi; \
    done \
 && /usr/local/bin/vcell-solver-entrypoint --help

ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
