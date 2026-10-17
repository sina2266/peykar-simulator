#!/usr/bin/env bash
# vcell-solver-entrypoint: the standard entry point of VCell's solver images
# (VCell docs/plan-solver-repos.md §1.5; README "Solver release contract").
#
#   <image>                       print the version and the executables provided, exit 0
#   <image> --help                the same
#   <image> VCellStoch_x64 ARGS   exec the solver (VCell's HPC form, as SlurmProxy writes it:
#                                 VCellStoch_x64 gibson /simdata/..stochInput /simdata/..ida -tid <n>)
#   <image> anything else         usage on stderr, exit 2
#
# It writes nothing itself, so it runs as any uid from a read-only SIF under
# `singularity run --containall`; the solver writes only the output paths it is given.
set -eu

bindir=/usr/local/bin
provided="VCellStoch_x64"
version=$(cat /usr/local/share/vcell-stochastic/VERSION 2>/dev/null || echo unknown)

usage() {
    echo "vcell-stochastic ${version} -- VCell's Gibson (next-reaction) stochastic solver"
    echo
    echo "usage: <image> <executable> [args...]"
    echo
    echo "executables:"
    for exe in ${provided}; do
        echo "  ${exe}"
    done
    echo
    echo "e.g.   <image> VCellStoch_x64 gibson /simdata/SimID_1_0_.stochInput /simdata/SimID_1_0_.ida -tid 0"
}

case "${1-}" in
    ""|--help|-h)
        usage
        exit 0
        ;;
esac

for exe in ${provided}; do
    if [ "$1" = "${exe}" ]; then
        shift
        exec "${bindir}/${exe}" "$@"
    fi
done

echo "vcell-solver-entrypoint: unknown executable '$1'" >&2
usage >&2
exit 2
