#!/bin/sh
# vcell-solver-entrypoint — the standard VCell solver-image entry point (SOLVER-RELEASE.md, §1.5 of
# VCell's docs/plan-solver-repos.md).
#
#   <image>                       print the version and the executables this image provides, exit 0
#   <image> --help | -h           the same
#   <image> NFsim_x64 <args...>   exec the solver (argv as VCell's SlurmProxy writes it, e.g.
#                                 NFsim_x64 -seed 1 -vcell -xml /simdata/x.nfsimInput ... -tid 0)
#   <image> <anything else>       print usage, exit 2
#
# It writes nothing (the solver writes only where its arguments point), runs as any uid, and works
# from a read-only SIF under `singularity run --containall`.
set -eu

home=${VCELL_SOLVER_HOME:-/opt/vcell-nfsim}
executables="NFsim_x64"

usage() {
    version=$(cat "${home}/VERSION" 2>/dev/null || echo unknown)
    echo "vcell-nfsim ${version} — NFsim for the Virtual Cell (https://github.com/virtualcell/vcell-nfsim)"
    echo
    echo "usage: <image> <executable> [arguments...]"
    echo
    echo "executables:"
    for exe in ${executables}; do
        echo "  ${exe}"
    done
    echo
    echo "example: <image> NFsim_x64 -seed 1 -vcell -xml /simdata/SimID_1_0_.nfsimInput \\"
    echo "           -o /simdata/SimID_1_0_.gdat -ss /simdata/SimID_1_0_.species -sim 1.0 -oSteps 20 -tid 0"
}

case "${1-}" in
    "" | --help | -h | help)
        usage
        exit 0
        ;;
esac

for exe in ${executables}; do
    if [ "$1" = "${exe}" ]; then
        shift
        exec "${home}/${exe}" "$@"   # absolute path: no reliance on PATH under --cleanenv
    fi
done

echo "error: '$1' is not an executable of this image" >&2
usage >&2
exit 2
