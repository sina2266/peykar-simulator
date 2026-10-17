#!/usr/bin/env bash
# Build the third-party libraries the release executable links, as static
# libraries, into one prefix — so MovingBoundary_x64 carries HDF5 (and, on
# Linux, libcurl) inside itself and needs nothing but the OS at run time.
#
#   packaging/build-deps.sh <prefix>
#
# Installs into <prefix>:
#   include/boost/...   Boost headers (header-only use: multi_array, polygon, ...)
#   HDF5 (C, C++, HL)   static, no zlib/szip filters (the solver writes no
#                       compressed datasets), CMake package config under <prefix>
#   libcurl             static, HTTP only, no TLS — only when WITH_CURL=1.
#                       (vcell-messaging posts to the broker's plain-http REST
#                       API.) macOS links the system libcurl instead.
#
# Environment: WITH_CURL=0|1 (default 0), CMAKE_OSX_ARCHITECTURES and
# MACOSX_DEPLOYMENT_TARGET are honoured by CMake as usual, JOBS (default: all cores).
set -euo pipefail

prefix="${1:?usage: build-deps.sh <prefix>}"
mkdir -p "${prefix}"
prefix="$(cd "${prefix}" && pwd)"
jobs="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)}"
work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

HDF5_VERSION=1.14.6
HDF5_URL="https://github.com/HDFGroup/hdf5/releases/download/hdf5_${HDF5_VERSION}/hdf5-${HDF5_VERSION}.tar.gz"
HDF5_SHA256=e4defbac30f50d64e1556374aa49e574417c9e72c6b1de7a4ff88c4b1bea6e9b
CURL_VERSION=8.16.0
CURL_URL="https://curl.se/download/curl-${CURL_VERSION}.tar.xz"
CURL_SHA256=40c8cddbcb6cc6251c03dea423a472a6cea4037be654ba5cf5dec6eb2d22ff1d
BOOST_VERSION=1.86.0
BOOST_URL="https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION//./_}.tar.bz2"
BOOST_SHA256=1bed88e40401b2cb7a1f76d4bab499e352fa4d0c5f31c0dbae64e24d34d7513b

fetch() { # url sha256 dest
    curl -fsSL --retry 5 -o "$3" "$1"
    if command -v sha256sum >/dev/null; then
        echo "$2  $3" | sha256sum -c -
    else
        echo "$2  $3" | shasum -a 256 -c -
    fi
}

cmake_common=(
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX="${prefix}"
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_PREFIX_PATH="${prefix}"
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_TESTING=OFF
)

echo "=== Boost ${BOOST_VERSION} headers"
fetch "${BOOST_URL}" "${BOOST_SHA256}" "${work}/boost.tar.bz2"
tar -xjf "${work}/boost.tar.bz2" -C "${work}" "boost_${BOOST_VERSION//./_}/boost"
mkdir -p "${prefix}/include"
rm -rf "${prefix}/include/boost"
mv "${work}/boost_${BOOST_VERSION//./_}/boost" "${prefix}/include/boost"

echo "=== HDF5 ${HDF5_VERSION} (static)"
fetch "${HDF5_URL}" "${HDF5_SHA256}" "${work}/hdf5.tar.gz"
tar -xzf "${work}/hdf5.tar.gz" -C "${work}"
cmake -S "${work}/hdf5-${HDF5_VERSION}" -B "${work}/hdf5-build" "${cmake_common[@]}" \
    -DBUILD_STATIC_LIBS=ON \
    -DHDF_PACKAGE_NAMESPACE=hdf5:: \
    -DHDF5_BUILD_CPP_LIB=ON \
    -DHDF5_BUILD_HL_LIB=ON \
    -DHDF5_BUILD_FORTRAN=OFF \
    -DHDF5_BUILD_JAVA=OFF \
    -DHDF5_BUILD_TOOLS=OFF \
    -DHDF5_BUILD_HL_TOOLS=OFF \
    -DHDF5_BUILD_UTILS=OFF \
    -DHDF5_BUILD_EXAMPLES=OFF \
    -DHDF5_ENABLE_Z_LIB_SUPPORT=OFF \
    -DHDF5_ENABLE_SZIP_SUPPORT=OFF \
    -DHDF5_ENABLE_THREADSAFE=OFF \
    -DHDF5_ENABLE_PARALLEL=OFF
cmake --build "${work}/hdf5-build" --parallel "${jobs}"
cmake --install "${work}/hdf5-build"

if [ "${WITH_CURL:-0}" = "1" ]; then
    echo "=== curl ${CURL_VERSION} (static, HTTP only)"
    fetch "${CURL_URL}" "${CURL_SHA256}" "${work}/curl.tar.xz"
    tar -xJf "${work}/curl.tar.xz" -C "${work}"
    cmake -S "${work}/curl-${CURL_VERSION}" -B "${work}/curl-build" "${cmake_common[@]}" \
        -DBUILD_STATIC_LIBS=ON \
        -DBUILD_CURL_EXE=OFF \
        -DBUILD_LIBCURL_DOCS=OFF \
        -DBUILD_MISC_DOCS=OFF \
        -DENABLE_CURL_MANUAL=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DHTTP_ONLY=ON \
        -DCURL_ENABLE_SSL=OFF \
        -DCURL_USE_OPENSSL=OFF \
        -DCURL_USE_LIBPSL=OFF \
        -DCURL_USE_LIBSSH2=OFF \
        -DCURL_ZLIB=OFF \
        -DCURL_BROTLI=OFF \
        -DCURL_ZSTD=OFF \
        -DUSE_NGHTTP2=OFF \
        -DUSE_LIBIDN2=OFF \
        -DUSE_LIBRTMP=OFF \
        -DCURL_DISABLE_LDAP=ON \
        -DENABLE_THREADED_RESOLVER=OFF
    cmake --build "${work}/curl-build" --parallel "${jobs}"
    cmake --install "${work}/curl-build"
fi

echo "=== installed into ${prefix}"
ls "${prefix}" "${prefix}/lib"
