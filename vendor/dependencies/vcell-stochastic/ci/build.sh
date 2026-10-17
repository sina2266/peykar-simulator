#!/usr/bin/env bash
# Configure, build and unit-test VCellStoch against the static HDF5 from ci/build-hdf5.sh, then
# stage the release layout (the archive root, SOLVER-RELEASE.md in the README):
#
#   VCellStoch_x64[.exe]   the solver, under the name VCell resolves
#   LICENSE                this project's licence
#   COPYING-HDF5           HDF5's licence (statically linked)
#   VERSION                the release version, e.g. v1.0.0
#
#   ci/build.sh <hdf5-prefix> <stage-dir> [extra cmake args...]
#
# VCELLSTOCH_VERSION sets the version (default: git describe). BUILD_DIR sets the build tree.
set -euo pipefail

hdf5=$1; stage=$2; shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
build=${BUILD_DIR:-$root/build-release}
version=${VCELLSTOCH_VERSION:-$(git -C "$root" describe --tags --match 'v*' --always --dirty 2>/dev/null || echo dev)}

generator=()
if command -v ninja >/dev/null 2>&1; then generator=(-G Ninja); fi

cmake -S "$root" -B "$build" "${generator[@]}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON \
    -DHDF5_ROOT="$hdf5" \
    -DHDF5_USE_STATIC_LIBRARIES=ON \
    -DGIT_DESCRIBE="$version" \
    "$@"
cmake --build "$build" --config Release --parallel
ctest --test-dir "$build" --build-config Release --output-on-failure

ext=""
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) ext=".exe" ;; esac
exe="$build/bin/VCellStoch$ext"
[ -f "$exe" ] || exe="$build/bin/Release/VCellStoch$ext"   # multi-config generators

mkdir -p "$stage"
cp "$exe" "$stage/VCellStoch_x64$ext"
cp "$root/LICENSE" "$stage/LICENSE"
cp "$hdf5/COPYING-HDF5" "$stage/COPYING-HDF5"
echo "$version" > "$stage/VERSION"
echo "staged $(ls "$stage" | tr '\n' ' ') in $stage"
