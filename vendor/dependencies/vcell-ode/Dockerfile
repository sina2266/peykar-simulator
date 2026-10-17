# vcell-ode solver image: ghcr.io/virtualcell/vcell-ode:<X.Y.Z> (VCell solver-repo plan §1.3–1.5).
#
#   build    — clang-19/libc++ build with messaging ON (the solver takes a trailing `-tid <n>` and reports
#              status to VCell's broker), unit tests run here
#   runtime  — debian:trixie-slim + the solver, its non-distro libraries, and exactly the distro runtime
#              packages its shared libraries come from; the standard vcell-solver-entrypoint
#
#   docker run --rm ghcr.io/virtualcell/vcell-ode:<X.Y.Z>                       # version + executables
#   docker run --rm -v $PWD:/simdata ghcr.io/virtualcell/vcell-ode:<X.Y.Z> \
#       SundialsSolverStandalone_x64 /simdata/in.cvodeInput /simdata/out.ida -tid 0
FROM debian:trixie-slim AS build
SHELL ["/bin/bash", "-c"]

RUN apt-get -y update && apt-get install -y apt-utils && apt-get remove --purge gcc
# line 1 => build dependencies
# line 2 => build tools
# line 3 => dependencies
RUN apt-get install -y -qq -o=Dpkg::Use-Pty=0 \
            git curl wget ca-certificates python3 python3-pip patchelf \
            mold ninja-build cmake clang-19 clang++-19 clang-tools-19 libc++-19-dev libc++abi-19-dev \
            libspdlog-dev libcurl4-openssl-dev

RUN rm $(which gcc) || true
RUN rm $(which g++) || true
RUN rm -rf /var/lib/apt/lists/* && rm /usr/bin/ld
RUN ln -s $(which mold) /usr/bin/ld
RUN clang-19 --version
RUN ln -s $(which clang-19) /usr/bin/clang
RUN clang++-19 --version
RUN ln -s $(which clang++-19) /usr/bin/clang++
COPY . /vcellroot

ARG VERSION=dev
RUN mkdir -p /vcellroot/build/bin
WORKDIR /vcellroot/build

RUN /usr/bin/cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DOPTION_TARGET_MESSAGING=ON -DOPTION_TARGET_DOCS=OFF \
        -DOPTION_TEST_WITH_LOCALHOST=ON -DVCELL_ODE_VERSION="${VERSION}"
RUN ninja --verbose
RUN ctest -VV --output-on-failure

# Stage /opt/vcell-ode: the executable, the libraries that no Debian package owns (the project's own),
# and runtime-packages.txt naming the packages that own every other library it loads.
RUN set -euo pipefail; \
    dest=/opt/vcell-ode; mkdir -p "$dest/bin" "$dest/lib"; \
    cp bin/SundialsSolverStandalone_x64 "$dest/bin/"; \
    : > "$dest/runtime-packages.txt"; \
    for lib in $(ldd bin/SundialsSolverStandalone_x64 | awk '/=> \// {print $3}'); do \
        real="$(readlink -f "$lib")"; \
        pkg="$(dpkg -S "$real" 2>/dev/null || dpkg -S "${real#/usr}" 2>/dev/null || dpkg -S "$lib" 2>/dev/null || true)"; \
        if [ -n "$pkg" ]; then echo "${pkg%%:*}" >> "$dest/runtime-packages.txt"; \
        else cp -L "$lib" "$dest/lib/"; fi; \
    done; \
    sort -u -o "$dest/runtime-packages.txt" "$dest/runtime-packages.txt"; \
    patchelf --set-rpath '$ORIGIN/../lib' "$dest/bin/SundialsSolverStandalone_x64"; \
    for so in "$dest"/lib/*; do [ -e "$so" ] && patchelf --set-rpath '$ORIGIN' "$so"; done; \
    cp /vcellroot/LICENSE "$dest/LICENSE"; \
    echo "${VERSION}" > "$dest/VERSION"; \
    echo "runtime packages:"; cat "$dest/runtime-packages.txt"; ls -l "$dest" "$dest/bin" "$dest/lib"


FROM debian:trixie-slim AS runtime

COPY --from=build /opt/vcell-ode/runtime-packages.txt /tmp/runtime-packages.txt
RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates $(cat /tmp/runtime-packages.txt) \
    && rm -rf /var/lib/apt/lists/* /tmp/runtime-packages.txt

COPY --from=build /opt/vcell-ode /opt/vcell-ode
COPY docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint
RUN chmod 0755 /usr/local/bin/vcell-solver-entrypoint \
    && if ldd /opt/vcell-ode/bin/SundialsSolverStandalone_x64 | grep 'not found'; then exit 1; fi \
    && /usr/local/bin/vcell-solver-entrypoint --help

ENV PATH="/opt/vcell-ode/bin:${PATH}"
LABEL org.opencontainers.image.source="https://github.com/virtualcell/vcell-ode" \
      org.opencontainers.image.description="VCell ODE/DAE solvers (SUNDIALS CVODE, IDA): SundialsSolverStandalone_x64" \
      org.opencontainers.image.licenses="MIT"
WORKDIR /tmp
ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
