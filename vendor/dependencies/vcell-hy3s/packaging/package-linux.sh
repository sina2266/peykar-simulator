#!/bin/sh
# Assemble the linux64 / linux64arm release directory from a build tree.
#
#   packaging/package-linux.sh <build/bin> <out dir> <version>
#
# The three solvers go at the root under VCell's names, next to every shared
# library they need that is not part of glibc, each with a $ORIGIN rpath so the
# directory runs wherever it is unpacked. glibc itself (libc, libm, libpthread,
# libdl, librt, libmvec, the loader) and libgcc_s are left to the host: the build runs on
# a manylinux_2_28 base, so glibc >= 2.28 is all a host needs. Messaging is
# compiled in (a static, HTTP-only libcurl built in docker/Dockerfile), so the same
# directory is what the container image puts on PATH.
set -eu

bindir=$1
out=$2
version=$3
here=$(cd "$(dirname "$0")" && pwd)
exes="Hybrid_EM_x64 Hybrid_MIL_x64 Hybrid_MIL_Adaptive_x64"

# Libraries every Linux host provides, matched on the soname.
system='^(linux-vdso|linux-gate|ld-linux[^ ]*|libc|libm|libpthread|libdl|librt|libutil|libresolv|libmvec|libgcc_s)\.so'

rm -rf "$out"
mkdir -p "$out"
for exe in $exes; do
    install -m 0755 "$bindir/$exe" "$out/$exe"
done

# ldd prints the whole closure, transitive dependencies included.
for exe in $exes; do
    ldd "$out/$exe"
done | awk '$2 == "=>" && $3 ~ /^\// { print $1, $3 }' | sort -u | while read -r name path; do
    if echo "$name" | grep -Eq "$system"; then
        continue
    fi
    if [ ! -e "$out/$name" ]; then
        echo "bundling $name  ($path)"
        cp -L "$path" "$out/$name"
        chmod 0644 "$out/$name"
    fi
done

# RUNPATH is not inherited by a library's own dependencies, so every bundled
# library needs $ORIGIN too (libgfortran finds libquadmath through it).
for f in "$out"/*; do
    patchelf --set-rpath '$ORIGIN' "$f"
done

# Verify: everything not from glibc must now resolve inside $out.
status=0
for f in "$out"/*; do
    ldd "$f" | awk '$2 == "=>" { print $1, $3 }' | while read -r name path; do
        if [ "$path" = "not" ]; then
            echo "error: $f: $name not found" >&2; exit 1
        fi
        if ! echo "$name" | grep -Eq "$system" && [ "$(dirname "$path")" != "$(cd "$out" && pwd)" ]; then
            echo "error: $f: $name resolves outside the bundle ($path)" >&2; exit 1
        fi
    done || status=1
done
# ...and no symbol may need a glibc newer than the manylinux_2_28 floor.
newest=$(for f in "$out"/*; do objdump -T "$f"; done | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)
echo "newest glibc symbol version required: $newest"
if [ "$(printf '%s\nGLIBC_2.28\n' "$newest" | sort -V | tail -1)" != "GLIBC_2.28" ]; then
    echo "error: requires $newest, newer than GLIBC_2.28" >&2
    status=1
fi
[ "$status" -eq 0 ]

printf '%s\n' "$version" > "$out/VERSION"
cat "$here/../LICENSE" "$here/THIRD-PARTY-NOTICES.txt" > "$out/LICENSE"
echo "packaged $version into $out:"
ls -l "$out"
