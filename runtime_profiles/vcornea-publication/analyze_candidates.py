"""Apply the authors' unchanged plotting mathematics to verified full native runs."""

import argparse
import json
import shutil
from pathlib import Path

from analyze_original import GROUPS, run
from compare import checksum, load_run, validate_series_jobs


def analyze(source, directory, output, *, diagnostic_cases=None):
    plan = json.loads((directory / "plan.json").read_bytes())
    protocol_path = directory / "comparison.json"
    protocol = json.loads(protocol_path.read_bytes())
    protocol_digest = checksum(protocol_path)
    if plan["status"] != "execution_completed" or plan["protocol_digest"] != protocol_digest:
        raise ValueError("Only completed frozen native series can enter original analysis")
    jobs = validate_series_jobs(plan, protocol, diagnostic_cases)
    loaded = [(job, load_run(plan, job, protocol_digest, protocol)) for job in jobs]
    cases = {job["case"] for job, _ in loaded}
    inputs = output / "verified-inputs"
    output.mkdir(parents=True, exist_ok=False)
    groups = {}
    for label, (relative, _) in GROUPS.items():
        if label not in cases:
            continue
        jobs = [(job, values) for job, values in loaded if job["case"] == label]
        if len(jobs) < 2 or len({job["seed"] for job, _ in jobs}) != len(jobs):
            raise ValueError("Each original-analysis group needs two distinct native seeds")
        groups[label] = (relative, len(jobs))
        for job, (root, _manifest, record, *_rest) in jobs:
            destination = inputs / relative / str(job["seed"]) / "Simulation"
            destination.mkdir(parents=True)
            for name in record["outputs"]:
                if Path(name).name.startswith(("cell_count_", "thickness_rep_")):
                    shutil.copyfile(root / name, destination / Path(name).name)
    run(source, inputs, output / "original-analysis", groups=groups)
    receipt = {
        "scope": "Authors' original mathematics on new native outputs; no biological validation",
        "plan_digest": checksum(directory / "plan.json"),
        "protocol_digest": protocol_digest,
        "diagnostic_subset": diagnostic_cases,
        "files": {
            p.relative_to(output).as_posix(): checksum(p) for p in output.rglob("*") if p.is_file()
        },
    }
    (output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--runs", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--diagnostic-case", action="append")
    args = parser.parse_args()
    analyze(args.source, args.runs, args.output, diagnostic_cases=args.diagnostic_case)
