#!/usr/bin/env bash
# Equivalence with the legacy VCell NFsim (vcell-solvers v0.0.44-dev4 NFsim_x64, the binary VCell's
# desktop client ships today): run the committed reference model through both executables with the
# same seeds, VCell's argument list, and compare the observables (.gdat) and final species (.species).
# NFsim draws from a seeded Mersenne twister, so an equivalent build reproduces the legacy output
# exactly; the comparison is byte-for-byte.
#
#   tests/equivalence/compare_legacy.sh <legacy NFsim_x64> <new NFsim_x64> [workdir]
#
# Either executable may be a command prefix in quotes (e.g. "arch -x86_64 ./NFsim_x64").
set -euo pipefail

legacy=$1
new=$2
work=${3:-$(mktemp -d)}
here=$(cd "$(dirname "$0")" && pwd)
input="${here}/../smoke/SimID_273069657_0_.nfsimInput"
seeds="505790288 1 42 1807259453 2147483646"

mkdir -p "${work}"
fail=0
for seed in ${seeds}; do
    for which in legacy new; do
        exe=${!which}
        # shellcheck disable=SC2086  # $exe may be a command prefix
        ${exe} -seed "${seed}" -vcell -xml "${input}" -o "${work}/${which}_${seed}.gdat" -sim 5.0 \
            -ss "${work}/${which}_${seed}.species" -oSteps 50 -notf -utl 1000 -cb -pcmatch \
            > "${work}/${which}_${seed}.stdout" 2>&1 \
            || { echo "error: ${which} NFsim failed for seed ${seed}" >&2; tail -20 "${work}/${which}_${seed}.stdout" >&2; exit 1; }
    done
    if cmp -s "${work}/legacy_${seed}.gdat" "${work}/new_${seed}.gdat" \
        && cmp -s "${work}/legacy_${seed}.species" "${work}/new_${seed}.species"; then
        echo "seed ${seed}: identical ($(wc -l < "${work}/new_${seed}.gdat") gdat lines)"
    else
        echo "seed ${seed}: DIFFERENT"
        diff "${work}/legacy_${seed}.gdat" "${work}/new_${seed}.gdat" | head -10 || true
        fail=1
    fi
done
exit ${fail}
