"""Explicit bounded publication-reproduction operator; never started by API admission."""

import argparse
import json
import os
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from xml.etree import ElementTree

from peykar.integrations.vcornea_publication import (
    PARAMETER_PROFILES,
    RECIPES,
    execute_native,
    prepare_publication,
    write_operator_json,
)
from peykar_core.hashing import bytes_digest, canonical_json, digest


def validate_parallelism(jobs, maximum):
    """Bound requested CC3D workers; BLAS environment controls cannot replace XML."""
    workers = []
    for job in jobs:
        tree = ElementTree.parse(Path(job["directory"]) / "project/Simulation/vCornea_v2.xml")
        count = int(tree.findtext("./Metadata/NumberOfProcessors") or "0")
        if count not in (1, 4):
            raise ValueError("Unreviewed native worker count")
        workers.append(count)
    reserved = sum(sorted(workers, reverse=True)[:maximum])
    if not 1 <= maximum <= 6 or reserved > 16 or (maximum > 4 and any(n != 1 for n in workers)):
        raise ValueError("At most four multiworker jobs or six single-worker jobs are supported")
    return {
        "requested_workers_per_job": workers,
        "maximum_simultaneous_requested_workers": reserved,
    }


def series_definition(plan):
    return {
        "protocol_digest": plan["protocol_digest"],
        "profile_digest": plan["profile_digest"],
        "jobs": [
            {key: job[key] for key in ["case", "seed", "directory", "manifest_digest"]}
            for job in plan["jobs"]
        ],
    }


def wait_for_series(requirements, output, budget):
    from resume_vcornea import verified_completed

    started = time.monotonic()
    expected = []
    for ordinal, directory in enumerate(requirements):
        raw = (directory / "plan.json").read_bytes()
        (output / f"prerequisite-{ordinal}.json").write_bytes(raw)
        expected.append((directory, series_definition(json.loads(raw))))
    while expected:
        if time.monotonic() - started > budget:
            raise TimeoutError("Prerequisite series wait deadline exceeded")
        remaining = []
        for directory, definition in expected:
            plan = json.loads((directory / "plan.json").read_bytes())
            if series_definition(plan) != definition:
                raise ValueError("Prerequisite native job/protocol/runtime identities changed")
            if plan["status"] == "execution_failed":
                raise ValueError("Prerequisite native series failed; preserve its evidence")
            if plan["status"] != "execution_completed":
                remaining.append((directory, definition))
                continue
            if not all(
                verified_completed(job, plan["profile_digest"], plan["protocol_digest"])
                for job in plan["jobs"]
            ):
                raise ValueError("Prerequisite completed records could not be verified")
        expected = remaining
        if expected:
            time.sleep(5)


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
    parser.add_argument("--injury-state", type=Path)
    parser.add_argument("--max-parallel", type=int, choices=(1, 2, 3, 4, 5, 6), default=2)
    parser.add_argument("--wall-seconds", type=float, default=86400)
    parser.add_argument("--wait-for-series", type=Path, action="append", default=[])
    parser.add_argument("--wait-seconds", type=int, default=86400)
    args = parser.parse_args()
    if len(args.case) != len(set(args.case)) or len(args.seed) != len(set(args.seed)):
        parser.error("Cases and seeds must be unique")
    if not 1 <= args.wait_seconds <= 172800:
        parser.error("Prerequisite wait budget must be between1 and172800 seconds")
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
        "execution_prerequisites": [str(p.resolve()) for p in args.wait_for_series],
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
                injury_state=args.injury_state,
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
    plan["native_worker_budget"] = validate_parallelism(plan["jobs"], args.max_parallel)
    if args.wait_for_series:
        plan["status"] = "waiting_for_prerequisites"
        write_operator_json(args.output / "plan.json", plan)
        try:
            wait_for_series(args.wait_for_series, args.output, args.wait_seconds)
        except Exception as error:
            plan.update(status="execution_failed", failure=type(error).__name__)
            write_operator_json(args.output / "plan.json", plan)
            raise
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
