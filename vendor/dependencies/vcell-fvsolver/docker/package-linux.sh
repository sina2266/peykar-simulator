#!/usr/bin/env bash
# Assemble the Linux release archive root (the contents of linux64.tgz / linux64arm.tgz) from a
# finished CMake build:
#
#   package-linux.sh <build-dir> <dest-dir> <version>
#
# Layout, per the VCell solver release contract (SOLVER-RELEASE.md): the executables under the names
# VCell resolves (FiniteVolume_x64, smoldyn_x64), every shared library they need that is not part of
# glibc, LICENSE and VERSION -- flat, at the root. Bundled libraries are found through a $ORIGIN
# rpath. glibc itself (libc, libm, libpthread, libdl, librt, the loader) is never bundled: the
# archive runs on the host's glibc, which must be >= the build image's (2.28, manylinux_2_28).
set -euo pipefail

build=$1
dest=$2
version=$3
here=$(cd "$(dirname "$0")" && pwd)
executables=(FiniteVolume_x64 smoldyn_x64)

# Shared objects that come with glibc (or the kernel) and must be taken from the host.
is_system_lib() {
    case "$1" in
        linux-vdso.so.*|linux-gate.so.*|ld-linux*.so.*|libc.so.*|libm.so.*|libpthread.so.*|\
        libdl.so.*|librt.so.*|libutil.so.*|libresolv.so.*|libnsl.so.*|libanl.so.*|libmvec.so.*)
            return 0 ;;
    esac
    return 1
}

rm -rf "$dest"
mkdir -p "$dest"
for exe in "${executables[@]}"; do
    install -m 0755 "$build/bin/$exe" "$dest/$exe"
done

# Walk the dependency closure until no new library turns up.
while :; do
    added=0
    for f in "$dest"/*; do
        [ -f "$f" ] || continue
        case "$f" in */LICENSE|*/VERSION) continue ;; esac
        while read -r name path; do
            name=${name##*/}
            is_system_lib "$name" && continue
            [ -e "$dest/$name" ] && continue
            if [ -z "$path" ] || [ ! -e "$path" ]; then
                echo "error: $f needs $name, which was not found" >&2
                exit 1
            fi
            install -m 0755 "$(readlink -f "$path")" "$dest/$name"
            added=1
        done < <(ldd "$f" | awk '/=>/ {print $1, $3; next} /^\s*[^ ]+\.so/ {print $1, ""}')
    done
    [ "$added" = 0 ] && break
done

# $ORIGIN rpath on every file that needs a bundled library (DT_RUNPATH is not transitive, so each
# bundled library that needs another one gets its own). Files that need only glibc stay untouched.
for f in "$dest"/*; do
    bundled=
    for name in $(ldd "$f" | awk '/=>/ {print $1}'); do
        is_system_lib "${name##*/}" || bundled=1
    done
    if [ -n "$bundled" ]; then
        patchelf --set-rpath '$ORIGIN' "$f"
    fi
done

install -m 0644 "$here/../LICENSE" "$dest/LICENSE"
printf '%s\n' "$version" > "$dest/VERSION"

# Report, and fail if anything outside the archive and glibc is still needed.
echo "archive root $dest:"
ls -l "$dest"
for f in "$dest"/*; do
    case "$f" in */LICENSE|*/VERSION) continue ;; esac
    echo "== $(basename "$f") needs:"
    LD_LIBRARY_PATH= ldd "$f" | sed 's/^/   /'
    while read -r name path; do
        name=${name##*/}
        is_system_lib "$name" && continue
        case "$path" in "$dest"/*) ;; *)
            echo "error: $f resolves $name to $path, outside the archive" >&2
            exit 1 ;;
        esac
    done < <(LD_LIBRARY_PATH= ldd "$f" | awk '/=>/ {print $1, $3}')
done
# The newest glibc symbol version referenced: must not exceed the manylinux_2_28 baseline.
newest=$(objdump -T "$dest"/* 2>/dev/null | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1 || true)
echo "newest glibc symbol version required: ${newest:-none}"
