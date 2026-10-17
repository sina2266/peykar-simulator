#!/usr/bin/env bash
# Package a built MovingBoundarySolver as a VCell release archive
# (SOLVER-RELEASE in README.md): at the archive root, the executable under the
# name VCell resolves (MovingBoundary_x64), LICENSE and VERSION — and fail if
# the executable depends on anything outside the OS.
#
#   packaging/package-unix.sh <MovingBoundarySolver> <out.tgz>
set -euo pipefail

exe="${1:?usage: package-unix.sh <MovingBoundarySolver> <out.tgz>}"
out="${2:?usage: package-unix.sh <MovingBoundarySolver> <out.tgz>}"
here="$(cd "$(dirname "$0")/.." && pwd)"
version="$(sed -n 's/^project(VCellMovingBoundary VERSION \([0-9.]*\).*/\1/p' "${here}/CMakeLists.txt")"

stage="$(mktemp -d)"
trap 'rm -rf "${stage}"' EXIT
cp "${exe}" "${stage}/MovingBoundary_x64"
chmod 755 "${stage}/MovingBoundary_x64"
cp "${here}/LICENSE" "${stage}/LICENSE"
echo "${version}" > "${stage}/VERSION"

"${here}/packaging/check-portable.sh" "${stage}/MovingBoundary_x64"

mkdir -p "$(dirname "${out}")"
tar -C "${stage}" -czf "${out}" MovingBoundary_x64 LICENSE VERSION
echo "packaged ${out}:"
tar -tzvf "${out}"
