#!/bin/sh
# vcell-solver-entrypoint — the standard VCell solver-image entry point
# (SOLVER-RELEASE in README.md; VCell docs/plan-solver-repos.md §1.5).
#
#   <image>                                  print the version and the executables, exit 0
#   <image> --help                           same
#   <image> MovingBoundary_x64 --config /simdata/<user>/SimID_<k>_0_mb.xml -tid 0
#                                            exec the solver (SlurmProxy's form: a bare name)
#   <image> anything-else                    usage on stderr, exit 2
#
# It writes nothing itself (a SIF is read-only; only $TMPDIR and the bound /simdata are
# writable), works as any uid, and execs the solver so its exit code and SIGTERM pass through.
set -eu

bindir=/opt/vcell/bin
executables="MovingBoundary_x64"
version="$(cat "${bindir}/VERSION" 2>/dev/null || echo unknown)"

info() {
    echo "vcell-mbsolver ${version} — VCell moving-boundary solver (FronTier)"
    echo "executables:"
    for e in ${executables}; do echo "  ${e}"; done
    echo "usage: <image> MovingBoundary_x64 --config <input_mb.xml> [-tid <n>]"
}

case "${1-}" in
    "" | --help | -h)
        info
        exit 0
        ;;
esac

for e in ${executables}; do
    if [ "$1" = "${e}" ]; then
        exec "$@"
    fi
done

info >&2
echo "error: '$1' is not an executable this image provides" >&2
exit 2
