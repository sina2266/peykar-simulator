#!/usr/bin/env bash
# Build a static, C-only HDF5 and install it under <prefix>, for the release builds to link
# against. No shared library, no C++/Fortran/HL/tools, no zlib/szip: VCellStoch only writes
# small uncompressed datasets (MultiTrialStats::writeHDF5), and a static C library keeps the
# release archives free of HDF5 dylibs/DLLs/.so files.
#
#   ci/build-hdf5.sh <prefix> [extra cmake args...]
#
# e.g. macOS slice:  ci/build-hdf5.sh /opt/hdf5 -DCMAKE_OSX_ARCHITECTURES=x86_64
#      Windows:      ci/build-hdf5.sh C:/hdf5 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DBUILD_STATIC_CRT_LIBS=ON
set -euo pipefail

prefix=$1; shift
HDF5_VERSION=${HDF5_VERSION:-1.14.6}
HDF5_SHA256=${HDF5_SHA256:-e4defbac30f50d64e1556374aa49e574417c9e72c6b1de7a4ff88c4b1bea6e9b}
url="https://github.com/HDFGroup/hdf5/releases/download/hdf5_${HDF5_VERSION}/hdf5-${HDF5_VERSION}.tar.gz"

work=${HDF5_WORK_DIR:-${TMPDIR:-/tmp}/hdf5-build-$$}
mkdir -p "$work"
cd "$work"
curl -fsSL -o hdf5.tar.gz "$url"
if command -v sha256sum >/dev/null 2>&1; then
    echo "${HDF5_SHA256}  hdf5.tar.gz" | sha256sum -c -
else
    echo "${HDF5_SHA256}  hdf5.tar.gz" | shasum -a 256 -c -
fi
tar xzf hdf5.tar.gz
src="$work/hdf5-${HDF5_VERSION}"

generator=()
if command -v ninja >/dev/null 2>&1; then generator=(-G Ninja); fi

cmake -S "$src" -B "$work/build" "${generator[@]}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$prefix" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_STATIC_LIBS=ON \
    -DBUILD_TESTING=OFF \
    -DHDF5_BUILD_TOOLS=OFF \
    -DHDF5_BUILD_UTILS=OFF \
    -DHDF5_BUILD_EXAMPLES=OFF \
    -DHDF5_BUILD_HL_LIB=OFF \
    -DHDF5_BUILD_CPP_LIB=OFF \
    -DHDF5_BUILD_FORTRAN=OFF \
    -DHDF5_BUILD_JAVA=OFF \
    -DHDF5_BUILD_DOC=OFF \
    -DHDF5_ENABLE_Z_LIB_SUPPORT=OFF \
    -DHDF5_ENABLE_SZIP_SUPPORT=OFF \
    -DHDF5_ENABLE_PARALLEL=OFF \
    "$@"
cmake --build "$work/build" --config Release --parallel
cmake --install "$work/build" --config Release

# HDF5's BSD-style licence has to travel with binaries that embed it.
cp "$src/COPYING" "$prefix/COPYING-HDF5"
echo "HDF5 ${HDF5_VERSION} installed in ${prefix}"
