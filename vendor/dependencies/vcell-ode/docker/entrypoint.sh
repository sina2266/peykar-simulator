#!/usr/bin/env bash
# vcell-solver-entrypoint — the standard VCell solver-image entry point (solver-repo plan §1.5).
#
#   <image>                                            version + the executables this image provides
#   <image> --help                                     same
#   <image> SundialsSolverStandalone_x64 in out -tid 0 VCell's HPC form (SlurmProxy): exec the solver
#   <image> anything-else                              usage, exit 2
#
# The solver is exec'd, so its exit code and signals (SIGTERM from Slurm) pass straight through. The
# script writes nothing itself, runs as any uid, and works from a read-only SIF under
# `singularity run --containall`.
set -eu

VERSION_FILE="${VCELL_SOLVER_VERSION_FILE:-/opt/vcell-ode/VERSION}"
EXECUTABLES="SundialsSolverStandalone_x64"

usage() {
    version="unknown"
    [ -r "${VERSION_FILE}" ] && version="$(cat "${VERSION_FILE}")"
    echo "vcell-ode ${version} — VCell ODE/DAE solvers (SUNDIALS CVODE and IDA)"
    echo
    echo "usage: <image> <executable> [args...]"
    echo
    echo "executables:"
    for exe in ${EXECUTABLES}; do
        echo "  ${exe} <input.cvodeInput|input.idaInput> <output.ida> [-tid <n>]"
    done
}

case "${1-}" in
    "" | --help | -h)
        usage
        exit 0
        ;;
esac

for exe in ${EXECUTABLES}; do
    if [ "$1" = "${exe}" ]; then
        exec "$@"
    fi
done

echo "error: '$1' is not an executable provided by this image" >&2
usage >&2
exit 2
