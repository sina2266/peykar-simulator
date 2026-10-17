#!/usr/bin/env bash
# Stage a Linux release archive (solver-repo plan §1.1–1.2).
#
#   package_linux.sh <build-dir> <stage-dir> <version>
#
# <stage-dir> gets, flat: the executables under VCell's names, every shared library they load
# except glibc's (and libgcc_s, present on every system), LICENSE and VERSION — with a $ORIGIN rpath
# so the directory runs from wherever it is unpacked. No test binaries, no static libraries.
set -euo pipefail

build="$1"; stage="$2"; version="$3"
repo="$(cd "$(dirname "$0")/../.." && pwd)"
EXES=(SundialsSolverStandalone_x64)
# glibc's own libraries (and the loader/vdso), which must come from the host
SYSTEM_LIBS='^(ld-linux.*|linux-vdso|libc|libm|libmvec|libpthread|libdl|librt|libutil|libresolv|libnsl|libanl|libBrokenLocale|libgcc_s)\.so'

command -v patchelf >/dev/null || python3 -m pip install -q patchelf

rm -rf "$stage"; mkdir -p "$stage"
stage="$(cd "$stage" && pwd)"   # absolute, to match ldd's paths in the checks below
search="$build/lib:$build/bin"
# clang's own libc++/libc++abi/libunwind live in its resource tree (e.g.
# /usr/local/lib/aarch64-unknown-linux-gnu), which the loader does not search by default
for lib in libc++.so.1 libc++abi.so.1 libunwind.so.1; do
    p="$(${CXX:-clang++} -print-file-name="$lib" 2>/dev/null || true)"
    if [ -n "$p" ] && [ "${p#/}" != "$p" ] && [ -e "$p" ]; then search="$search:$(dirname "$p")"; fi
done
echo "library search path: $search"
for exe in "${EXES[@]}"; do
    cp "$build/bin/$exe" "$stage/"
    # ldd prints the full transitive closure; copy each non-system library under its soname
    LD_LIBRARY_PATH="$search" ldd "$build/bin/$exe" | while read -r name arrow path _; do
        [ "$arrow" = "=>" ] || continue
        if [ "$path" = "not" ]; then echo "error: $exe needs $name, not found" >&2; exit 1; fi
        if [[ "$name" =~ $SYSTEM_LIBS ]]; then continue; fi
        [ -e "$stage/$name" ] || cp -L "$path" "$stage/$name"
    done
done

chmod u+w,a+rx "$stage"/*
for f in "$stage"/*; do patchelf --set-rpath '$ORIGIN' "$f"; done
cp "$repo/LICENSE" "$stage/LICENSE"
echo "$version" > "$stage/VERSION"

# Verify: everything resolves from the stage dir alone, and nothing glibc-owned was bundled.
for exe in "${EXES[@]}"; do
    out="$(env -u LD_LIBRARY_PATH ldd "$stage/$exe")"
    echo "$out"
    if grep -q 'not found' <<<"$out"; then echo "error: unresolved libraries in $exe" >&2; exit 1; fi
    if grep '=> /' <<<"$out" | awk '{print $1, $3}' | grep -Ev "$SYSTEM_LIBS" | grep -v " $stage/"; then
        echo "error: $exe loads a non-system library from outside the archive (above)" >&2; exit 1
    fi
done
if ls "$stage" | grep -E "$SYSTEM_LIBS|\.a$|^unit_tests|gtest|gmock"; then
    echo "error: glibc, static or test files in the archive (above)" >&2; exit 1
fi
echo "staged $stage:"; ls -l "$stage"
