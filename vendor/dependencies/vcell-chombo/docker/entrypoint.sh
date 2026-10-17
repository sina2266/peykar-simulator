#!/bin/sh
# vcell-solver-entrypoint -- the standard VCell solver-image entry point
# (SOLVER-RELEASE.md; VCell's docs/plan-solver-repos.md section 1.5).
#
#   <image>                      print the version and the executables, exit 0
#   <image> --help               the same
#   <image> VCellChombo2D_x64 /simdata/<user>/SimID_<k>_0_.fvinput -tid <n>
#                                exec that solver with exactly those arguments
#   <image> anything-else        usage on stderr, exit 2
#
# SlurmProxy runs the SIF as
#   singularity run --containall <binds> <env> <sif> <executable> <args> -tid <n>
# so the first argument is a bare executable name and the paths are container
# paths under /simdata. `exec` keeps the solver as the container's main process:
# its exit code and SIGTERM (a Slurm cancel) pass straight through.
#
# Nothing here writes anywhere, and the solver writes only next to the input's
# BASE_FILE_NAME (under /simdata on the cluster) -- including the per-timepoint
# .sim.hdf5 scratch file it zips into the results -- so the image works
# read-only and as any uid.
set -eu

root=/opt/vcell-chombo
exes="VCellChombo2D_x64 VCellChombo3D_x64"

describe() {
	echo "vcell-chombo $(cat "$root/VERSION" 2>/dev/null || echo unknown)"
	echo "Chombo embedded-boundary finite-volume solver for the Virtual Cell (serial; messaging on)"
	echo
	echo "executables:"
	for e in $exes; do echo "  $e"; done
	echo
	echo "usage: <image> <executable> [-tid <taskID>] <input.fvinput>"
	echo "       <image> <executable> -ccd <input.fvinput>   (convert Chombo output to VCell output)"
}

case "${1-}" in
	"" | --help | -h)
		describe
		exit 0
		;;
esac

for e in $exes; do
	if [ "$1" = "$e" ]; then
		exec "$@"
	fi
done

{
	echo "vcell-solver-entrypoint: unknown executable '$1'"
	echo
	describe
} >&2
exit 2
