#!/usr/bin/env bash
# Make a staged Linux release self-contained (SOLVER-RELEASE.md):
#
#   bundle-linux.sh <stage-dir> [max-glibc]
#
# Everything from Conan (HDF5, libzip, zlib, libcurl) is linked statically, so
# what is left is the GCC runtime. The executables' shared-library closure is
# walked with ldd; anything that is not part of glibc or the base C++ runtime
# every Linux system ships (libstdc++, libgcc_s) is copied next to them, and
# every ELF file gets RUNPATH=$ORIGIN so it finds its neighbours wherever the
# archive is unpacked. In practice that is libgfortran, libquadmath, and the
# libz the system libgfortran links (the solvers' own zlib is static).
#
# glibc is never bundled; instead the check at the end fails the build if any
# file needs a newer glibc symbol version than [max-glibc] (default 2.28, the
# manylinux_2_28 baseline the release is built on).
set -euo pipefail

stage=$(cd "${1:?stage dir}" && pwd)
max_glibc=${2:-2.28}

# Provided by the system everywhere; never bundled.
system_re='^(linux-vdso|linux-gate|ld-linux[^ ]*|libc|libm|libpthread|libdl|librt|libutil|libresolv|libstdc\+\+|libgcc_s)\.so'

exes=(VCellChombo2D_x64 VCellChombo3D_x64)

bundle_closure() {
	local file=$1
	ldd "$file" | while read -r name arrow path _; do
		[ "$arrow" = "=>" ] || continue
		if [[ "$name" =~ $system_re ]]; then continue; fi
		if [ "$path" = "not" ]; then
			echo "bundle-linux.sh: $file needs $name, which is not found" >&2
			exit 1
		fi
		if [ ! -e "$stage/$name" ]; then
			cp -L "$path" "$stage/$name"
			chmod 0755 "$stage/$name"
			echo "  bundled $name (from $path)"
			bundle_closure "$stage/$name"
		fi
	done
}

for exe in "${exes[@]}"; do
	bundle_closure "$stage/$exe"
done

shopt -s nullglob
elfs=("${exes[@]/#/$stage/}" "$stage"/*.so*)
for f in "${elfs[@]}"; do
	# strip first: it can undo patchelf's rewrite of the dynamic section.
	strip --strip-unneeded "$f"
	patchelf --set-rpath '$ORIGIN' "$f"
done

# Verify: every dependency resolves inside the archive or to the system list,
# and no file needs a glibc newer than the baseline.
status=0
for f in "${elfs[@]}"; do
	while read -r name arrow path _; do
		[ "$arrow" = "=>" ] || continue
		if [[ "$name" =~ $system_re ]]; then continue; fi
		case "$path" in
			"$stage"/*) ;;
			*) echo "bundle-linux.sh: $(basename "$f") resolves $name to $path, outside the archive" >&2; status=1 ;;
		esac
	done < <(LD_LIBRARY_PATH= ldd "$f")
	need=$(objdump -T "$f" | grep -o 'GLIBC_[0-9][0-9.]*' | sed 's/GLIBC_//' | sort -Vu | tail -1)
	if [ -n "$need" ] && [ "$(printf '%s\n%s\n' "$need" "$max_glibc" | sort -V | tail -1)" != "$max_glibc" ]; then
		echo "bundle-linux.sh: $(basename "$f") needs GLIBC_$need, newer than $max_glibc" >&2
		status=1
	fi
	cxx=$(objdump -T "$f" | grep -o 'GLIBCXX_[0-9][0-9.]*' | sort -Vu | tail -1 || true)
	echo "  $(basename "$f"): glibc >= ${need:-none}${cxx:+, $cxx}"
done
exit $status
