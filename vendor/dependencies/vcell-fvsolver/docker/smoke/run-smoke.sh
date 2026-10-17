#!/usr/bin/env bash
# Smoke-test a vcell-fvsolver image (Docker) or SIF (Apptainer) the way VCell's cluster runs it:
# a bare executable name, inputs and outputs under /simdata (a bind mount), a trailing `-tid 0`,
# status messages posted to a broker. Reproduces committed reference outputs:
#
#   FiniteVolume_x64  VCell/tests/smoke  (2-D SUNDIALS PDE, 3 time points) -- compared element-wise
#                     against SimID_1585623750_0_00.zip.expected, which is 0.9.7's output; and
#                     VCell/tests/testFiles/input/FVSolver (3-D FV solver, 2 membranes) -- compared
#                     against docker/smoke/reference/fv3d-summary.json (0.9.7's per-variable statistics)
#   smoldyn_x64       VCell/tests/testFiles/input/Smoldyn (3-D membrane A<->B, 201 time points) --
#                     compared against docker/smoke/reference/smoldyn-summary.json (0.9.7's totals)
#
# usage: run-smoke.sh <workdir> <runner...>
#   <workdir>  a fresh host directory; mounted by the runner as /simdata
#   <runner>   the command prefix that runs the container with <workdir> at /simdata, e.g.
#                docker run --rm --network host --user "$(id -u):$(id -g)" -v "$work:/simdata" vcell-fvsolver:ci
#                apptainer run --containall --bind "$work:/simdata" vcell-fvsolver.sif
#
# Environment: PYTHON (with numpy) runs the comparisons (default python3); BROKER_HOST is the
# address the container reaches this host's broker at (default 127.0.0.1: host networking);
# FV_RTOL / SMOLDYN_RTOL loosen the comparisons; SKIP_ENTRYPOINT_CHECKS=1 skips the --help/exit-2
# checks (for the legacy 0.9.7 image, which has no entrypoint).
set -euo pipefail

here=$(cd "$(dirname "$0")" && pwd)
repo=$(cd "$here/../.." && pwd)
work=$1
shift
runner=("$@")
py=${PYTHON:-python3}
broker_host=${BROKER_HOST:-127.0.0.1}
port=${BROKER_PORT:-18165}
fv_rtol=${FV_RTOL:-1e-6}
smoldyn_rtol=${SMOLDYN_RTOL:-0.25}

mkdir -p "$work/fv" "$work/smoldyn" "$work/ref"
log="$work/broker.jsonl"
: > "$log"
"$py" "$here/broker.py" "$port" "$log" &
broker=$!
trap 'kill $broker 2>/dev/null || true' EXIT
sleep 1

step() { echo; echo "=== $*"; }

if [ -z "${SKIP_ENTRYPOINT_CHECKS-}" ]; then   # set for the 0.9.7 image, which has no entrypoint
step "--help exits 0 and names both executables"
"${runner[@]}" --help | tee "$work/help.txt"
grep -q FiniteVolume_x64 "$work/help.txt"
grep -q smoldyn_x64 "$work/help.txt"

step "an unknown command exits 2"
set +e
"${runner[@]}" not-a-solver > /dev/null 2>&1
rc=$?
set -e
[ "$rc" = 2 ] || { echo "expected exit 2, got $rc"; exit 1; }
fi

step "FiniteVolume_x64 with -tid, status to the broker"
base=SimID_1585623750_0_
cp "$repo/VCell/tests/smoke/$base.vcg" "$work/fv/"
{
    cat <<EOF
JMS_PARAM_BEGIN
JMS_BROKER $broker_host:$port
JMS_USER smoke smoke
JMS_QUEUE workerEventSmoke
JMS_TOPIC serviceControlSmoke
VCELL_USER smoke
SIMULATION_KEY 1585623750
JOB_INDEX 0
JMS_PARAM_END

EOF
    sed "s#@BASE_FILE_NAME@#/simdata/fv/$base#" "$repo/VCell/tests/smoke/$base.fvinput.in"
} > "$work/fv/$base.fvinput"
"${runner[@]}" FiniteVolume_x64 "/simdata/fv/$base.fvinput" -tid 0 | tee "$work/fv/stdout.txt"
ls -l "$work/fv"
cp "$repo/VCell/tests/smoke/${base}00.zip.expected" "$work/ref/${base}00.zip"
"$py" "$here/simdata.py" compare "$work/fv" "$base" "$work/ref" "$fv_rtol"

step "FiniteVolume_x64, 3-D FV_SOLVER with membranes (VCell/tests/testFiles/input/FVSolver)"
base3=SimID_11538992_0_
mkdir -p "$work/fv3d"
cp "$repo/VCell/tests/testFiles/input/FVSolver/$base3.vcg" "$work/fv3d/"
sed "s#^BASE_FILE_NAME .*#BASE_FILE_NAME /simdata/fv3d/$base3#" \
    "$repo/VCell/tests/testFiles/input/FVSolver/$base3.fvinput" > "$work/fv3d/$base3.fvinput"
"${runner[@]}" FiniteVolume_x64 "/simdata/fv3d/$base3.fvinput" > "$work/fv3d/stdout.txt"
tail -3 "$work/fv3d/stdout.txt"
"$py" "$here/simdata.py" compare "$work/fv3d" "$base3" "$here/reference/fv3d-summary.json" "$fv_rtol"

step "the broker received JOB_STARTING (999) and JOB_COMPLETED (1003) for task 0"
sleep 1
cat "$log"
"$py" - "$log" <<'PY'
import json, sys
events = [json.loads(line) for line in open(sys.argv[1]) if line.strip()]
status = [e["query"].get("WorkerEvent_Status") for e in events if e["path"].endswith("/api/message/workerEvent")]
tasks = {e["query"].get("TaskID") for e in events}
print("statuses:", status, "task ids:", tasks)
assert "999" in status and "1003" in status, "missing JOB_STARTING / JOB_COMPLETED"
assert tasks == {"0"}, tasks
PY

step "smoldyn_x64 with -tid"
cp "$repo"/VCell/tests/testFiles/input/Smoldyn/input.* "$work/smoldyn/"
"${runner[@]}" smoldyn_x64 /simdata/smoldyn/input.smoldynInput -tid 0 > "$work/smoldyn/stdout.txt"
tail -5 "$work/smoldyn/stdout.txt"
ls -l "$work/smoldyn"
"$py" "$here/simdata.py" compare "$work/smoldyn" SimID_1371782956_0_ "$here/reference/smoldyn-summary.json" "$smoldyn_rtol"

echo
echo "smoke test passed"
