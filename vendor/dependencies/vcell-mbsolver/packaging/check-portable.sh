#!/usr/bin/env bash
# Fail unless an executable links only OS-provided libraries — the release
# contract's "runs on a clean machine" rule. Everything else (HDF5, libcurl on
# Linux) must be linked statically.
#
#   Linux: NEEDED entries limited to glibc, libstdc++ and libgcc_s — the
#          manylinux_2_28 system set; the highest GLIBC_ symbol version is printed.
#   macOS: load commands limited to /usr/lib and /System; no /opt/homebrew,
#          /usr/local or @rpath references.
set -euo pipefail
exe="${1:?usage: check-portable.sh <executable>}"

case "$(uname -s)" in
Linux)
    needed="$(readelf -d "${exe}" | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p')"
    echo "NEEDED:"; echo "${needed}" | sed 's/^/  /'
    bad="$(echo "${needed}" | grep -Ev '^(libc|libm|libpthread|libdl|librt|libstdc\+\+|libgcc_s|ld-linux(-x86-64|-aarch64))\.so' || true)"
    if [ -n "${bad}" ]; then
        echo "::error::${exe} needs non-system libraries: ${bad}"; exit 1
    fi
    echo "highest GLIBC symbol version: $(objdump -T "${exe}" | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)"
    echo "highest GLIBCXX symbol version: $(objdump -T "${exe}" | grep -o 'GLIBCXX_[0-9.]*' | sort -uV | tail -1)"
    ;;
Darwin)
    # a universal binary lists each slice under its own "<file> (architecture ...):" header;
    # the load commands are the indented lines
    libs="$(otool -L "${exe}" | grep -E '^[[:space:]]' | awk '{print $1}' | sort -u)"
    echo "load commands:"; echo "${libs}" | sed 's/^/  /'
    bad="$(echo "${libs}" | grep -Ev '^(/usr/lib/|/System/)' || true)"
    if [ -n "${bad}" ]; then
        echo "::error::${exe} links non-system libraries: ${bad}"; exit 1
    fi
    lipo -info "${exe}"
    ;;
*)
    echo "unsupported OS $(uname -s)"; exit 1 ;;
esac
