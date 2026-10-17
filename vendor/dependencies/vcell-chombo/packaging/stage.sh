#!/usr/bin/env bash
# Lay out a release archive's contents (SOLVER-RELEASE.md, "Archive layout"):
#
#   stage.sh <build-dir> <stage-dir> <version>
#
# Copies the two solvers out of <build-dir>/bin, plus LICENSE, the licences of
# everything linked into them, and a VERSION file, into <stage-dir>. The
# platform bundlers (bundle-linux.sh, bundle_macos.py) then pull in the shared
# libraries the executables need and fix up their search paths.
#
# Third-party licences come from the Conan package folders: CMakeDeps records
# each one in build/generators/*-data.cmake, and Conan packages carry their
# licence files under licenses/. HDF5's, libzip's and curl's licences require
# the notice to travel with binary redistributions, as Chombo's does.
set -euo pipefail

build=${1:?build dir}
stage=${2:?stage dir}
version=${3:?version}
root=$(cd "$(dirname "$0")/.." && pwd)

rm -rf "$stage"
mkdir -p "$stage/THIRD-PARTY-LICENSES"

for exe in VCellChombo2D_x64 VCellChombo3D_x64; do
	install -m 0755 "$build/bin/$exe" "$stage/$exe"
done

printf '%s\n' "$version" > "$stage/VERSION"
cp "$root/LICENSE" "$stage/LICENSE"

tp="$stage/THIRD-PARTY-LICENSES"
cp "$root/chombo/Copyright.txt" "$tp/Chombo.txt"
cp "$root/vcell-messaging/LICENSE" "$tp/vcell-messaging.txt"
cp "$root/vcell-expressionparser/LICENSE" "$tp/vcell-expressionparser.txt"

found=0
for data in "$build"/generators/*-data.cmake; do
	[ -e "$data" ] || continue
	folder=$(sed -n 's/^set([A-Za-z0-9_]*_PACKAGE_FOLDER_[A-Z]* "\(.*\)")$/\1/p' "$data" | head -1)
	[ -n "$folder" ] && [ -d "$folder/licenses" ] || continue
	pkg=$(basename "$data" | sed -E 's/-(release|debug|relwithdebinfo|minsizerel)-.*-data\.cmake$//I; s/-data\.cmake$//')
	mkdir -p "$tp/$pkg"
	cp -R "$folder/licenses/." "$tp/$pkg/"
	found=$((found + 1))
done
if [ "$found" -eq 0 ]; then
	echo "stage.sh: found no Conan package licences under $build/generators" >&2
	exit 1
fi

echo "staged $version into $stage ($found third-party packages)"
