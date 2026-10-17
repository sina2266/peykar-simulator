#!/bin/sh
# vcell-solver-entrypoint -- the standard VCell solver-image entrypoint
# (SOLVER-RELEASE.md, contract section 5).
#
#   <image>                         print the version and the executables; exit 0
#   <image> --help                  the same
#   <image> <executable> <args...>  exec it, so its exit code and signals pass through
#   <image> <anything else>         usage on stderr; exit 2
#
# VCell's SlurmProxy runs the SIF as
#   singularity run --containall <binds> <env> <sif> Hybrid_EM_x64 /simdata/<user>/SimID_<k>_0_.nc <params> -OV -tid <n>
# i.e. a bare executable name first. Nothing here writes anywhere, so it works
# from a read-only SIF as any uid; the solver writes only its own .nc input.
set -eu

home=/opt/vcell-hy3s
exes="Hybrid_EM_x64 Hybrid_MIL_x64 Hybrid_MIL_Adaptive_x64"

usage() {
    echo "vcell-hy3s $(cat "$home/VERSION" 2>/dev/null || echo unknown) -- Hy3S hybrid stochastic solvers for the Virtual Cell"
    echo
    echo "executables:"
    for e in $exes; do echo "  $e"; done
    echo
    echo "usage: <image> <executable> <model.nc> <epsilon> <lambda> <MSR_tol> [<SDE_tol>] <SDE_dt> [-R <seed>] -OV [-tid <n>]"
    echo "       (<SDE_tol> for Hybrid_MIL_Adaptive_x64 only; -tid enables status messaging)"
}

case "${1-}" in
    "" | -h | --help)
        usage
        exit 0
        ;;
esac

for e in $exes; do
    if [ "$1" = "$e" ]; then
        shift
        exec "$home/$e" "$@"
    fi
done

usage >&2
echo >&2
echo "error: '$1' is not an executable this image provides" >&2
exit 2
