# syntax=docker/dockerfile:1.7
#
# vcell-hy3s: the Hy3S hybrid stochastic solvers, built once and published two
# ways from the same files (SOLVER-RELEASE.md):
#
#   --target archive   the linux64 / linux64arm release directory, for
#                      `--output type=local,dest=...`
#   --target runtime   (default) that same directory on PATH in a slim image
#                      with the standard VCell solver entrypoint; the SIF is
#                      built from this image
#
#   docker build -f docker/Dockerfile --build-arg HY3S_VERSION=1.0.0 -t vcell-hy3s .
#
# The build stage is manylinux_2_28 (AlmaLinux 8, glibc 2.28, GCC from
# gcc-toolset), so the archive runs on any glibc >= 2.28 host. It builds
# natively per architecture -- CI runs amd64 and arm64 on their own runners --
# and needs the vcell-messaging submodule checked out.

ARG MANYLINUX=manylinux_2_28
ARG RUNTIME_BASE=debian:bookworm-slim

FROM quay.io/pypa/${MANYLINUX}_x86_64 AS base-amd64
FROM quay.io/pypa/${MANYLINUX}_aarch64 AS base-arm64

# ---------------------------------------------------------------------------
FROM base-${TARGETARCH} AS build
ENV PATH=/opt/python/cp312-cp312/bin:${PATH} \
    CC=gcc CXX=g++ FC=gfortran
RUN python -m pip install --no-cache-dir cmake ninja \
 && gcc --version | head -1 && gfortran --version | head -1

# libcurl, for messaging only, built here rather than taken from Conan: Conan's
# Linux recipe goes through autotools, and its m4 does not run on manylinux.
# Static, and HTTP only -- the JMS REST bridge is plain HTTP -- so it brings no
# OpenSSL, zlib or other library into the binaries, and needs no threads for
# name resolution. Pinned by checksum.
ARG CURL_VERSION=8.16.0
ARG CURL_SHA256=40c8cddbcb6cc6251c03dea423a472a6cea4037be654ba5cf5dec6eb2d22ff1d
RUN curl -fsSL -o /tmp/curl.tar.xz "https://curl.se/download/curl-${CURL_VERSION}.tar.xz" \
 && echo "${CURL_SHA256}  /tmp/curl.tar.xz" | sha256sum -c - \
 && mkdir /tmp/curl && tar -C /tmp/curl --strip-components=1 -xJf /tmp/curl.tar.xz \
 && cmake -S /tmp/curl -B /tmp/curl/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/curl -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_CURL_EXE=OFF \
      -DBUILD_TESTING=OFF -DBUILD_LIBCURL_DOCS=OFF -DBUILD_MISC_DOCS=OFF -DENABLE_CURL_MANUAL=OFF \
      -DHTTP_ONLY=ON -DCURL_ENABLE_SSL=OFF -DCURL_USE_LIBPSL=OFF -DCURL_USE_LIBSSH2=OFF \
      -DCURL_ZLIB=OFF -DCURL_BROTLI=OFF -DCURL_ZSTD=OFF -DUSE_NGHTTP2=OFF -DUSE_LIBIDN2=OFF \
      -DENABLE_THREADED_RESOLVER=OFF -DCURL_DISABLE_LDAP=ON -DCURL_USE_PKGCONFIG=OFF \
 && cmake --build /tmp/curl/build && cmake --install /tmp/curl/build \
 && rm -rf /tmp/curl /tmp/curl.tar.xz

WORKDIR /src
COPY . .
ARG HY3S_VERSION=unknown
# -static-libgfortran: gcc-toolset's libgfortran.so is a linker script onto the
# *system* libgfortran, GCC 8.5's, whose RANDOM_NUMBER is a different generator
# (xorshift1024*) from every GCC >= 9 (xoshiro256**) -- so the same seed gave a
# different trajectory here than on macOS or any current Linux, and the
# committed baseline failed. The toolset's own libgfortran.a is GCC 14's; static
# linking it (GCC Runtime Library Exception) puts every platform on one runtime.
RUN cmake -B build -S . -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXE_LINKER_FLAGS="-static-libgfortran" \
      -DCMAKE_PREFIX_PATH=/opt/curl \
      -DOPTION_TARGET_MESSAGING=ON \
      -DHY3S_VERSION_STAMP="${HY3S_VERSION}" \
 && cmake --build build \
 && ctest --test-dir build --output-on-failure \
 && packaging/package-linux.sh build/bin /dist "${HY3S_VERSION}"

# ---------------------------------------------------------------------------
FROM scratch AS archive
COPY --from=build /dist/ /

# ---------------------------------------------------------------------------
FROM ${RUNTIME_BASE} AS runtime
LABEL org.opencontainers.image.source="https://github.com/virtualcell/vcell-hy3s" \
      org.opencontainers.image.description="Hy3S hybrid stochastic-deterministic solvers for the Virtual Cell" \
      org.opencontainers.image.licenses="MIT"
COPY --from=build /dist/ /opt/vcell-hy3s/
COPY --chmod=0755 docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint
ENV PATH=/opt/vcell-hy3s:${PATH}
ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
