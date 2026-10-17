#!/usr/bin/env bash
# Build the Linux release archive's contents (see SOLVER-RELEASE.md) inside a manylinux_2_28
# container (quay.io/pypa/manylinux_2_28_{x86_64,aarch64}), so NFsim_x64 needs nothing newer
# than glibc 2.28 / GLIBCXX 3.4.25 and runs on the cluster's and users' Linux.
#
#   packaging/build-linux.sh <source dir> <output dir> <version>
#
# NFsim_x64 is built with VCell messaging ON (a trailing "-tid <n>" reports status to VCell's
# broker over HTTP). libcurl is built here as a minimal, HTTP-only static library and linked in,
# so the archive carries no shared libraries beyond the system's glibc, libstdc++ and libgcc_s.
set -euo pipefail

src=$(cd "$1" && pwd)
out=$2
version=$3
curl_version=${CURL_VERSION:-8.10.1}
jobs=$(nproc)

# CMakeLists.txt looks for Perl (for the BioNetGen model tests).
command -v perl > /dev/null || dnf -y install perl-interpreter > /dev/null

work=$(mktemp -d)
trap 'rm -rf "${work}"' EXIT

# --- a minimal static libcurl: http only, no TLS, no compression, no extra protocols ----------
curl -fsSL --retry 5 "https://curl.se/download/curl-${curl_version}.tar.gz" -o "${work}/curl.tgz"
tar -xzf "${work}/curl.tgz" -C "${work}"
(
    cd "${work}/curl-${curl_version}"
    ./configure --prefix="${work}/curl" --disable-shared --enable-static --with-pic \
        --without-ssl --without-zlib --without-brotli --without-zstd --without-nghttp2 \
        --without-libidn2 --without-libpsl --without-libssh2 --without-librtmp \
        --disable-ldap --disable-ldaps --disable-rtsp --disable-dict --disable-telnet \
        --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher \
        --disable-mqtt --disable-smb --disable-ftp --disable-file --disable-ipfs \
        --disable-websockets --disable-manual --disable-docs --disable-ntlm \
        --disable-alt-svc --disable-hsts --enable-http > "${work}/curl-configure.log"
    make -j"${jobs}" > "${work}/curl-make.log"
    make install > /dev/null
)

# --- NFsim_x64 --------------------------------------------------------------------------------
cmake -S "${src}" -B "${work}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPTION_VCELL_MESSAGING=ON \
    -DVCELL_NFSIM_VERSION="${version}" \
    -DCMAKE_PREFIX_PATH="${work}/curl" \
    -DCURL_INCLUDE_DIR="${work}/curl/include" \
    -DCURL_LIBRARY="${work}/curl/lib/libcurl.a" \
    -DCURL_NO_CURL_CMAKE=ON
cmake --build "${work}/build" --target NFsim_x64 -j"${jobs}"

exe="${work}/build/bin/NFsim_x64"
strip "${exe}"

# Only system libraries may remain (glibc, libstdc++, libgcc_s, the loader).
echo "NFsim_x64 needs:"
ldd "${exe}"
if ldd "${exe}" | grep -vE 'linux-vdso|ld-linux|lib(c|m|pthread|dl|rt|stdc\+\+|gcc_s)\.so' | grep -q '=>'; then
    echo "error: NFsim_x64 links a non-system shared library" >&2
    exit 1
fi
if ldd "${exe}" | grep -q 'libcurl'; then
    echo "error: libcurl should be linked statically" >&2
    exit 1
fi

mkdir -p "${out}"
cp "${exe}" "${out}/NFsim_x64"
cp "${src}/LICENSE" "${out}/LICENSE"
cp "${src}/LICENSE.txt" "${out}/LICENSE-GPL-3.0.txt"
printf '%s\n' "${version}" > "${out}/VERSION"
