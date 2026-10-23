"""Explicit bounded publication-reproduction operator; never started by API admission."""

import argparse
import json
import os
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from peykar.integrations.vcornea_publication import (
    PARAMETER_PROFILES,
    RECIPES,
    execute_native,
    prepare_publication,
)
from peykar_core.hashing import bytes_digest, canonical_json, digest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--pins", required=True, type=Path)
    parser.add_argument("--prefix", required=True, type=Path)
    parser.add_argument("--profile", required=True, type=Path)
    parser.add_argument("--protocol", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--case", action="append", choices=tuple(RECIPES), required=True)
    parser.add_argument("--seed", type=int, action="append", required=True)
    parser.add_argument("--parameter-profile", choices=PARAMETER_PROFILES, default="local-1.0.0")
    parser.add_argument("--max-parallel", type=int, choices=(1, 2, 3, 4), default=2)
    parser.add_argument("--wall-seconds", type=float, default=86400)
    args = parser.parse_args()
    if len(args.case) != len(set(args.case)) or len(args.seed) != len(set(args.seed)):
        parser.error("Cases and seeds must be unique")
    args.output.mkdir(parents=True, exist_ok=False)
    protocol = json.loads(args.protocol.read_bytes())
    if not protocol.get("frozen_before_full_candidate_execution"):
        parser.error("A previously frozen comparison protocol is required")
    plan = {
        "schema_version": "1.0.0",
        "status": "preparing",
        "supervisor_pid": os.getpid(),
        "protocol_digest": bytes_digest(args.protocol.read_bytes()),
        "profile_digest": digest(json.loads(args.profile.read_bytes())),
        "parameter_profile": args.parameter_profile,
        "max_parallel": args.max_parallel,
        "wall_seconds_per_run": args.wall_seconds,
        "jobs": [],
        "interpretation": "Reproduction is pending until full runs and frozen comparisons pass.",
    }
    (args.output / "comparison.json").write_bytes(args.protocol.read_bytes())
    for case in args.case:
        for seed in args.seed:
            directory = args.output / f"{case}-seed-{seed}"
            manifest = prepare_publication(
                args.source,
                args.pins,
                directory,
                case,
                seed,
                parameter_profile=args.parameter_profile,
            )
            manifest["comparison_protocol_digest"] = plan["protocol_digest"]
            (directory / "manifest.json").write_text(canonical_json(manifest) + "\n")
            plan["jobs"].append(
                {
                    "case": case,
                    "seed": seed,
                    "directory": str(directory.resolve()),
                    "manifest_digest": digest(manifest),
                    "status": "prepared",
                }
            )
    plan["status"] = "running"
    (args.output / "plan.json").write_text(canonical_json(plan) + "\n")
    with ThreadPoolExecutor(max_workers=args.max_parallel) as executor:
        futures = {
            executor.submit(
                execute_native,
                Path(job["directory"]),
                args.prefix,
                args.profile,
                wall_seconds=args.wall_seconds,
            ): job
            for job in plan["jobs"]
        }
        for future in as_completed(futures):
            job = futures[future]
            try:
                result = future.result()
                job["status"] = result["status"]
                job["elapsed_seconds"] = result["elapsed_seconds"]
            except BaseException as error:
                job["status"] = "failed"
                job["failure"] = type(error).__name__
            temporary = args.output / "plan.tmp"
            temporary.write_text(canonical_json(plan) + "\n")
            temporary.replace(args.output / "plan.json")
            print(job["case"], "seed", job["seed"], job["status"], flush=True)
    plan["status"] = (
        "execution_completed"
        if all(j["status"] == "succeeded" for j in plan["jobs"])
        else "execution_failed"
    )
    (args.output / "plan.json").write_text(canonical_json(plan) + "\n")
    print(
        "Execution phase",
        plan["status"],
        "; original analysis/frozen comparison still required.",
        flush=True,
    )
    if plan["status"] != "execution_completed":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
