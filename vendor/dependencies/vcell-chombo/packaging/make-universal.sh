#!/usr/bin/env bash
# Merge two bundled macOS stages (bundle_macos.py) into one universal stage:
#
#   make-universal.sh <arm64-stage> <x86_64-stage> <out-stage>
#
# Every Mach-O file must be present in both, under the same name -- the two
# builds use the same Homebrew GCC major version, so the runtime dylibs match
# -- and is lipo'd into a fat file and re-signed ad hoc. Everything else
# (LICENSE, VERSION, the licence tree) must be identical and is copied once.
set -euo pipefail

arm=${1:?arm64 stage}
x86=${2:?x86_64 stage}
out=${3:?output stage}

rm -rf "$out"
mkdir -p "$out"

status=0
while IFS= read -r rel; do
	a="$arm/$rel" x="$x86/$rel" o="$out/$rel"
	mkdir -p "$(dirname "$o")"
	if [ ! -e "$x" ]; then
		echo "make-universal.sh: $rel is only in the arm64 build" >&2; status=1; continue
	fi
	if file -b "$a" | grep -q 'Mach-O'; then
		lipo -create "$a" "$x" -output "$o"
		chmod 0755 "$o"
		codesign --force --sign - "$o"
		echo "  $rel: $(lipo -archs "$o")"
	elif cmp -s "$a" "$x"; then
		cp -p "$a" "$o"
	else
		echo "make-universal.sh: $rel differs between the two builds" >&2; status=1
	fi
done < <(cd "$arm" && find . -type f | sed 's|^\./||' | sort)

while IFS= read -r rel; do
	[ -e "$arm/$rel" ] || { echo "make-universal.sh: $rel is only in the x86_64 build" >&2; status=1; }
done < <(cd "$x86" && find . -type f | sed 's|^\./||' | sort)

for exe in VCellChombo2D_x64 VCellChombo3D_x64; do
	archs=$(lipo -archs "$out/$exe")
	case "$archs" in
		*arm64*x86_64*|*x86_64*arm64*) ;;
		*) echo "make-universal.sh: $exe is '$archs', not universal" >&2; status=1 ;;
	esac
done
exit $status
