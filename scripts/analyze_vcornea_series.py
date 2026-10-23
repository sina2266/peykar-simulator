"""Explicit durable follow-on analysis for an already running, frozen corneal series."""

import argparse
import json
import os
import subprocess
import time
from datetime import UTC, datetime
from pathlib import Path

from resume_vcornea import active_record, verified_completed

from peykar.integrations.vcornea_publication import write_operator_json
from peykar_core.hashing import bytes_digest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runs", required=True, type=Path)
    parser.add_argument("--python", required=True, type=Path)
    parser.add_argument("--references", required=True, type=Path)
    parser.add_argument("--original-source", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--wait-seconds", type=int, default=90000)
    parser.add_argument("--diagnostic-case", action="append")
    args = parser.parse_args()
    if not 1 <= args.wait_seconds <= 172800:
        parser.error("Analysis wait budget must be between1 and172800 seconds")
    args.output.mkdir(parents=True, exist_ok=False)
    state = {
        "schema_version": "1.0.0",
        "supervisor_pid": os.getpid(),
        "started_utc": datetime.now(UTC).isoformat(),
        "status": "waiting_for_native_runs",
        "runs": str(args.runs.resolve()),
        "stages": [],
        "independent_biological_validation": False,
        "diagnostic_subset": args.diagnostic_case,
    }
    write_operator_json(args.output / "status.json", state)
    started = time.monotonic()
    while True:
        raw = (args.runs / "plan.json").read_bytes()
        plan = json.loads(raw)
        if plan["status"] in ("execution_completed", "execution_failed"):
            break
        if time.monotonic() - started > args.wait_seconds:
            state.update(status="native_wait_deadline", finished_utc=datetime.now(UTC).isoformat())
            write_operator_json(args.output / "status.json", state)
            raise SystemExit(1)
        # Reconcile only actual successful child-exit proofs after the outer process is gone.
        parent = subprocess.run(
            ["ps", "-ww", "-p", str(plan["supervisor_pid"]), "-o", "command="],
            capture_output=True,
            text=True,
            check=False,
        ).stdout
        if "resume_vcornea.py" not in parent and "reproduce_vcornea.py" not in parent:
            verified = []
            active = False
            for job in plan["jobs"]:
                root = Path(job["directory"])
                path = root / "record.json"
                if path.exists() and active_record(root, json.loads(path.read_bytes())):
                    active = True
                    verified.append(False)
                else:
                    verified.append(
                        verified_completed(job, plan["profile_digest"], plan["protocol_digest"])
                    )
            all_done = all(verified)
            if all_done:
                (args.output / "plan-before-reconciliation.json").write_bytes(raw)
                for job in plan["jobs"]:
                    job["status"] = "succeeded"
                plan["status"] = "execution_completed"
                plan["completion_reconciled_utc"] = datetime.now(UTC).isoformat()
                write_operator_json(args.runs / "plan.json", plan)
                break
            if not active:
                state.update(
                    status="native_evidence_incomplete", finished_utc=datetime.now(UTC).isoformat()
                )
                write_operator_json(args.output / "status.json", state)
                raise SystemExit(1)
        time.sleep(5)
    if plan["status"] != "execution_completed":
        state.update(status="native_execution_failed", finished_utc=datetime.now(UTC).isoformat())
        write_operator_json(args.output / "status.json", state)
        raise SystemExit(1)
    state.update(
        status="analyzing",
        plan_digest=bytes_digest((args.runs / "plan.json").read_bytes()),
        protocol_digest=plan["protocol_digest"],
    )
    write_operator_json(args.output / "status.json", state)
    profile_scripts = Path(__file__).resolve().parents[1] / "runtime_profiles/vcornea-publication"
    jobs = [
        ("comparison", "compare.py", ["--references", str(args.references)]),
        ("original-analysis", "analyze_candidates.py", ["--source", str(args.original_source)]),
        ("spatial-analysis", "analyze_spatial.py", []),
    ]
    if args.diagnostic_case:
        selection = [part for case in args.diagnostic_case for part in ("--diagnostic-case", case)]
        for name, _script, extra in jobs:
            if name != "spatial-analysis":
                extra.extend(selection)
    temporary = args.output / "tmp"
    temporary.mkdir()
    environment = {
        "PATH": str(args.python.resolve().parent) + ":/usr/bin:/bin",
        "HOME": str(temporary.resolve()),
        "TMPDIR": str(temporary.resolve()),
        "QT_QPA_PLATFORM": "offscreen",
        "MPLBACKEND": "Agg",
        "MPLCONFIGDIR": str((temporary / "matplotlib").resolve()),
        "PYTHONDONTWRITEBYTECODE": "1",
        "OMP_NUM_THREADS": "1",
        "OPENBLAS_NUM_THREADS": "1",
        "MKL_NUM_THREADS": "1",
    }
    for name, script, extra in jobs:
        source = profile_scripts / script
        command = [
            str(args.python.resolve()),
            str(source),
            "--runs",
            str(args.runs.resolve()),
            "--output",
            str((args.output / name).resolve()),
            *extra,
        ]
        stage = {"name": name, "script_digest": bytes_digest(source.read_bytes())}
        with (args.output / f"{name}.log").open("xb") as log:
            try:
                result = subprocess.run(
                    command,
                    env=environment,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    timeout=900,
                    check=False,
                )
                stage["returncode"] = result.returncode
            except subprocess.TimeoutExpired:
                stage["failure"] = "AnalysisDeadlineExceeded"
        state["stages"].append(stage)
        write_operator_json(args.output / "status.json", state)
        print(name, stage, flush=True)
    completed = state["stages"][0].get("returncode") in (0, 2) and all(
        s.get("returncode") == 0 for s in state["stages"][1:]
    )
    state["status"] = "analysis_failed"
    if completed:
        state["status"] = (
            "comparison_passed" if state["stages"][0]["returncode"] == 0 else "not_reproduced"
        )
        if args.diagnostic_case:
            state["status"] = "diagnostic_" + state["status"]
    state["finished_utc"] = datetime.now(UTC).isoformat()
    state["files"] = {
        p.relative_to(args.output).as_posix(): bytes_digest(p.read_bytes())
        for p in args.output.rglob("*")
        if p.is_file() and p.name != "status.json" and "tmp" not in p.relative_to(args.output).parts
    }
    write_operator_json(args.output / "status.json", state)
    print(state["status"], flush=True)
    passed = state["status"] in ("comparison_passed", "diagnostic_comparison_passed")
    raise SystemExit(0 if passed else 2 if completed else 1)


if __name__ == "__main__":
    main()
