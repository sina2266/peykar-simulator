"""Resume an interrupted frozen series with real completed records and fresh retry jobs.

Start this operator detached from the terminal. Existing files/records are preserved;
missing solver exit codes never become fabricated successes.
"""

import argparse
import json
import os
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from peykar.integrations.vcornea_publication import (
    execute_native,
    prepare_publication,
    recover_native_completion,
    write_operator_json,
)
from peykar_core.hashing import bytes_digest, canonical_json, digest


def active_record(root, record):
    pid = record.get("pid")
    if not isinstance(pid, int):
        return False
    command = subprocess.run(
        ["ps", "-ww", "-p", str(pid), "-o", "command="], capture_output=True, text=True, check=False
    ).stdout
    return (
        str(root.resolve() / "driver.py") in command
        or str(root.resolve() / "completion_guard.py") in command
    )


def verified_completed(job, profile_digest, protocol_digest):
    root = Path(job["directory"]).resolve(strict=True)
    path = root / "record.json"
    if not path.exists():
        return False
    record = json.loads(path.read_bytes())
    if active_record(root, record):
        raise ValueError("A native job is still active; do not duplicate it")
    if record["status"] == "running" and (root / "completion.json").exists():
        record = recover_native_completion(root)
    if record["status"] != "succeeded" or record.get("returncode") != 0:
        return False
    manifest = json.loads((root / "manifest.json").read_bytes())
    if (
        digest(manifest) != job["manifest_digest"]
        or record["manifest_digest"] != job["manifest_digest"]
        or record["runtime_digest"] != profile_digest
        or manifest["comparison_protocol_digest"] != protocol_digest
    ):
        raise ValueError("Completed job identity differs")
    inventories = [
        (root / "project", manifest["execution_files"]),
        (root, {"driver.py": manifest["driver_digest"]}),
        (root, record["outputs"]),
    ]
    for base, files in inventories:
        for relative, expected in files.items():
            path = (base / relative).resolve(strict=True)
            if (
                not path.is_relative_to(base.resolve())
                or bytes_digest(path.read_bytes()) != expected
            ):
                raise ValueError("Retained completed source/artifact differs")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--previous", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--pins", required=True, type=Path)
    parser.add_argument("--prefix", required=True, type=Path)
    parser.add_argument("--profile", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--max-parallel", type=int, choices=range(1, 7), default=4)
    parser.add_argument("--injury-state", type=Path)
    args = parser.parse_args()
    original = (args.previous / "plan.json").read_bytes()
    previous = json.loads(original)
    protocol = (args.previous / "comparison.json").read_bytes()
    if (
        bytes_digest(protocol) != previous["protocol_digest"]
        or digest(json.loads(args.profile.read_bytes())) != previous["profile_digest"]
    ):
        raise ValueError("Preserve the original frozen protocol and runtime")
    reusable = {
        job["directory"]: verified_completed(
            job, previous["profile_digest"], previous["protocol_digest"]
        )
        for job in previous["jobs"]
    }
    args.output.mkdir(parents=True, exist_ok=False)
    (args.output / "previous-plan.json").write_bytes(original)
    (args.output / "comparison.json").write_bytes(protocol)
    plan = {key: value for key, value in previous.items() if key != "jobs"}
    plan.update(
        supervisor_pid=os.getpid(),
        max_parallel=args.max_parallel,
        predecessor_digest=bytes_digest(original),
        status="preparing",
        jobs=[],
    )
    for old in previous["jobs"]:
        if reusable[old["directory"]]:
            plan["jobs"].append(dict(old, reused_verified_completed=True))
            continue
        directory = args.output / f"{old['case']}-seed-{old['seed']}"
        manifest = prepare_publication(
            args.source,
            args.pins,
            directory,
            old["case"],
            old["seed"],
            parameter_profile=previous["parameter_profile"],
            injury_state=args.injury_state,
        )
        manifest["comparison_protocol_digest"] = previous["protocol_digest"]
        manifest["previous_attempt_manifest_digest"] = old["manifest_digest"]
        manifest["retry_reason"] = (
            "No captured successful native exit; retain previous output/record"
        )
        (directory / "manifest.json").write_text(canonical_json(manifest) + "\n")
        plan["jobs"].append(
            {
                "case": old["case"],
                "seed": old["seed"],
                "directory": str(directory.resolve()),
                "manifest_digest": digest(manifest),
                "status": "prepared",
            }
        )
    # Reused successful jobs consume no new capacity. Apply the same actual XML
    # worker budget as first execution before starting any retry child.
    from reproduce_vcornea import validate_parallelism

    plan["native_worker_budget"] = validate_parallelism(
        [job for job in plan["jobs"] if job["status"] == "prepared"], args.max_parallel
    )
    plan["status"] = "running"
    write_operator_json(args.output / "plan.json", plan)
    with ThreadPoolExecutor(max_workers=args.max_parallel) as executor:
        futures = {
            executor.submit(
                execute_native,
                Path(job["directory"]),
                args.prefix,
                args.profile,
                wall_seconds=plan["wall_seconds_per_run"],
            ): job
            for job in plan["jobs"]
            if job["status"] == "prepared"
        }
        for future in as_completed(futures):
            job = futures[future]
            try:
                record = future.result()
                job.update(status=record["status"], elapsed_seconds=record["elapsed_seconds"])
            except Exception as error:
                job.update(status="failed", failure=type(error).__name__)
            write_operator_json(args.output / "plan.json", plan)
            print(job["case"], job["seed"], job["status"], flush=True)
    plan["status"] = (
        "execution_completed"
        if all(j["status"] == "succeeded" for j in plan["jobs"])
        else "execution_failed"
    )
    write_operator_json(args.output / "plan.json", plan)
    print(plan["status"], flush=True)
    raise SystemExit(0 if plan["status"] == "execution_completed" else 1)


if __name__ == "__main__":
    main()
