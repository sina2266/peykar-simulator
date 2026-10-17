#!/usr/bin/env bash
# Solver-image smoke test (SOLVER-RELEASE.md): run the committed reference model through an image
# exactly the way VCell's SlurmProxy does — bare executable name, inputs and outputs under /simdata,
# a trailing "-tid <n>" — and check that
#   * the outputs reproduce the committed reference (tests/smoke/*.expected) exactly (fixed seed),
#   * with -tid, status messages reach the broker named in the input's <jms> element
#     (a local fake broker here), ending in JOB_COMPLETED (1003),
#   * `--help` exits 0 and an unknown executable exits 2.
#
#   tests/container/smoke.sh <workdir> <runner...>
#
# <runner...> is the command that runs the image with <workdir> bound at /simdata, e.g.
#   docker run --rm --network host --user 12345:12345 -v "$work:/simdata" vcell-nfsim:ci
#   apptainer run --containall --bind "$work:/simdata" vcell-nfsim.sif
set -euo pipefail

work=$1; shift
here=$(cd "$(dirname "$0")" && pwd)
ref="${here}/../smoke"
sim=SimID_273069657_0_
port=${BROKER_PORT:-18165}
broker_host=${BROKER_HOST:-127.0.0.1}   # as the solver sees the host

mkdir -p "${work}"
chmod 0777 "${work}"   # the image runs as an arbitrary uid
rm -f "${work}/${sim}.gdat" "${work}/${sim}.species" "${work}/broker.log"

# The reference input, with its <jms> broker pointed at the fake broker.
sed -E "s#<broker>[^<]*</broker>#<broker>${broker_host}:${port}</broker>#" \
    "${ref}/${sim}.nfsimInput" > "${work}/${sim}.nfsimInput"

python3 "${here}/fake_broker.py" "${port}" "${work}/broker.log" &
broker=$!
trap 'kill ${broker} 2>/dev/null || true' EXIT
sleep 1

echo "--- help"
"$@" --help
help=$("$@")   # no argument: the same help (captured, not piped: grep -q would close the pipe early)
grep -q NFsim_x64 <<< "${help}"
set +e
"$@" not-a-solver > /dev/null 2>&1
rc=$?
set -e
[ "${rc}" -eq 2 ] || { echo "error: unknown executable exited ${rc}, expected 2" >&2; exit 1; }

echo "--- NFsim_x64 with -tid"
"$@" NFsim_x64 -seed 505790288 -vcell -xml "/simdata/${sim}.nfsimInput" -o "/simdata/${sim}.gdat" \
    -sim 1.0 -ss "/simdata/${sim}.species" -oStep 20 -notf -utl 1000 -cb -pcmatch -tid 0 \
    | tee "${work}/stdout.txt"

cmp "${work}/${sim}.gdat" "${ref}/${sim}.gdat.expected"
cmp "${work}/${sim}.species" "${ref}/${sim}.species.expected"
echo "outputs match the committed reference"

echo "--- broker log"
cat "${work}/broker.log"
grep -q 'SimKey=273069657' "${work}/broker.log"
grep -q 'TaskID=0' "${work}/broker.log"
grep -q 'WorkerEvent_Status=1003' "${work}/broker.log" \
    || { echo "error: no JOB_COMPLETED message reached the broker" >&2; exit 1; }
if grep -q 'WorkerEvent_Status=1002' "${work}/broker.log"; then
    echo "error: the solver reported JOB_FAILURE" >&2; exit 1
fi
echo "smoke test passed"
