#!/bin/sh
# vcell-solver-entrypoint: the standard entry point of a VCell solver image (SOLVER-RELEASE.md).
#
#   <image>                          print the version and the executables provided; exit 0
#   <image> --help                   the same
#   <image> FiniteVolume_x64 <args>  exec the solver with <args> unchanged (exit code, signals pass through)
#   <image> smoldyn_x64 <args>       ditto
#   anything else                    usage on stderr; exit 2
#
# This is the argv VCell's SlurmProxy writes:
#   singularity run --containall --bind ...:/simdata <sif> FiniteVolume_x64 /simdata/<user>/SimID_..._.fvinput -tid 0
# It needs no writable path of its own (a SIF is read-only) and runs as any uid. The solvers write
# their results next to the input and stage temporary files in $TMPDIR (default /tmp).
set -eu

dir="${VCELL_SOLVER_DIR:-/opt/vcell-fvsolver}"
executables="FiniteVolume_x64 smoldyn_x64"

info() {
    version=$(cat "$dir/VERSION" 2>/dev/null || echo unknown)
    echo "vcell-fvsolver $version"
    echo
    echo "usage: <image> <executable> [arguments...]"
    echo
    echo "executables:"
    for exe in $executables; do
        echo "  $exe"
    done
    echo
    echo "e.g.  <image> FiniteVolume_x64 /simdata/SimID_1_0_.fvinput -tid 0"
    echo "      <image> smoldyn_x64 /simdata/SimID_1_0_.smoldynInput -tid 0"
}

case "${1-}" in
    ""|--help|-h|help)
        info
        exit 0
        ;;
esac

for exe in $executables; do
    if [ "$1" = "$exe" ] || [ "$1" = "$dir/$exe" ]; then
        shift
        exec "$dir/$exe" "$@"
    fi
done

echo "vcell-solver-entrypoint: '$1' is not an executable this image provides" >&2
info >&2
exit 2
