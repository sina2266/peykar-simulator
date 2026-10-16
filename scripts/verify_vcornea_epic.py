"""Verify real archived outputs and seeded bounded vCornea runs, without clinical claims."""

import argparse
import json
import os
import subprocess
from pathlib import Path

from peykar.data_fabric.service import DatasetService
from peykar.data_fabric.vcornea_import import VCorneaImporter
from peykar.integrations.vcornea_runtime import (
    ARCHIVE_DOI,
    ARCHIVE_SHA256,
    SOURCE_TREE_DIGEST,
    execute_run,
    prepare_run,
)
from peykar.simulation.service import SimulationService
from peykar.validation.repeated_seeds import RepeatedSeedService
from peykar_core.contracts import (
    Quantity,
    Reference,
    RepeatedSeedProtocol,
    ScientificContext,
    VCorneaOutputSpec,
)
from peykar_core.hashing import bytes_digest, canonical_json, digest

ROOT = Path(__file__).resolve().parents[1]
NORMALIZER = ROOT / "scripts/vcornea_normalize.py"
CONTEXT = ScientificContext(
    species="not_assessed",
    biological_system="corneal epithelium (vCornea simulation)",
    conditions=("archived paper_version",),
)


def decode_parquet(file: Path, destination: Path, image_id: str) -> bytes:
    destination.parent.mkdir(exist_ok=True, parents=True)
    command = (
        "docker",
        "run",
        "--rm",
        "--platform",
        "linux/amd64",
        "--network",
        "none",
        "--read-only",
        "--cap-drop",
        "ALL",
        "--security-opt",
        "no-new-privileges:true",
        "--memory",
        "2g",
        "--cpus",
        "1",
        "--pids-limit",
        "64",
        "--user",
        f"{os.getuid()}:{os.getgid()}",
        "--tmpfs",
        "/tmp:size=64m,mode=1777",
        "--mount",
        f"type=bind,src={file.parent.resolve()},dst=/input,readonly",
        "--mount",
        f"type=bind,src={destination.parent.resolve()},dst=/decoded",
        "--mount",
        f"type=bind,src={NORMALIZER},dst=/normalizer.py,readonly",
        "--entrypoint",
        "/opt/conda/envs/cc3d/bin/python",
        image_id,
        "/normalizer.py",
        f"/input/{file.name}",
        "--output",
        f"/decoded/{destination.name}",
    )
    subprocess.run(command, check=True, timeout=120, stdout=subprocess.PIPE)
    return destination.read_bytes()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--references", type=Path, required=True)
    parser.add_argument("--acquisition", type=Path, required=True)
    parser.add_argument("--pins", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--image", required=True)
    parser.add_argument("--steps", type=int, default=60)
    parser.add_argument("--wall-seconds", type=float, default=600)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    image_id = subprocess.check_output(
        ("docker", "image", "inspect", "--format", "{{.Id}}", args.image), text=True
    ).strip()
    acceptance = {
        "schema_version": "1.0.0",
        "archive_doi": ARCHIVE_DOI,
        "image_id": image_id,
        "steps_mcs": args.steps,
        "seeds": [7, 11],
        "paired_seed_absolute_tolerance": 0.0,
        "unit": "count",
        "endpoint": "vcornea.basal_count",
        "independent_biological_validation": False,
        "purpose": "Native reference ingestion and seeded integration regression",
    }
    (args.output / "acceptance.json").write_text(canonical_json(acceptance) + "\n")
    service = SimulationService(args.data_dir)
    importer = VCorneaImporter(service.db, service.artifacts)
    reference_outputs = []
    acquired = json.loads(args.acquisition.read_bytes())
    if acquired["record"] != "16884414" or len(acquired["members"]) != 6:
        raise ValueError("Use the pinned six-member reference acquisition")
    for member in acquired["members"]:
        file = args.references / member["path"]
        raw = file.read_bytes()
        if bytes_digest(raw) != member["sha256"] or len(raw) != member["bytes"]:
            raise ValueError("Acquired reference changed")
        severity = next(
            name for name in ("slight", "mild", "moderate") if name in str(file).lower()
        )
        kind = "cell_count" if file.suffix == ".csv" else "thickness"
        id = f"vcornea.published.{severity}.{kind.replace('_', '-')}"
        spec = VCorneaOutputSpec(
            id=id,
            version="1.0.0",
            title=f"Archived vCornea {severity} {kind}",
            kind=kind,
            origin="simulation",
            context=CONTEXT,
            license="CC-BY-4.0; Vanin et al., Zenodo 16884414",
            source=(
                f"{acquired['source_url']}#{member['path']}; associated archive source; "
                "actual seed/parameters unknown"
            ),
            source_archive_doi=ARCHIVE_DOI,
            source_archive_digest=ARCHIVE_SHA256,
            variant="paper_version",
            source_tree_digest=SOURCE_TREE_DIGEST,
            normalizer_digest=bytes_digest(NORMALIZER.read_bytes())
            if kind == "thickness"
            else None,
        )
        if kind == "cell_count":
            lineage = importer.import_cell_counts(spec, raw)
        else:
            decoded = decode_parquet(file, args.output / "decoded" / f"{severity}.json", image_id)
            lineage = importer.import_thickness(spec, raw, decoded)
        output = importer.get_output(lineage.reference)
        reference_outputs.append(
            {
                "id": id,
                "kind": kind,
                "samples": len(output.time.values),
                "time_unit": output.time.unit,
                "last_hour": output.time.values[-1],
                "raw_digest": lineage.raw_digest,
                "seed": output.seed,
            }
        )
        print(json.dumps({"stage": "reference_import", **reference_outputs[-1]}), flush=True)

    def run(role: str, seed: int) -> Reference:
        directory = args.output / "runs" / f"{role}-{seed}"
        manifest = prepare_run(
            args.source, args.pins, directory, scenario="injury", steps=args.steps, seed=seed
        )
        print(json.dumps({"stage": "run_started", "role": role, "seed": seed}), flush=True)
        record = execute_run(directory, image_id, wall_seconds=args.wall_seconds)
        manifest_digest = service.artifacts.put(canonical_json(manifest).encode())
        id = f"vcornea.epic.{args.output.name}.{role}.{seed}"
        spec = VCorneaOutputSpec(
            id=id,
            version="1.0.0",
            title=f"Bounded native run {role}, seed {seed}",
            kind="cell_count",
            origin="simulation",
            context=CONTEXT,
            license="CC-BY-4.0; archived model; Peykar execution controls recorded separately",
            source=f"{ARCHIVE_DOI}; bounded derivative controls; manifest sha256:{manifest_digest}",
            source_archive_doi=ARCHIVE_DOI,
            source_archive_digest=ARCHIVE_SHA256,
            variant="paper_version",
            source_tree_digest=SOURCE_TREE_DIGEST,
            runtime_digest=image_id.removeprefix("sha256:"),
            parameter_digest=bytes_digest(
                (directory / "project/Simulation/Parameters.py").read_bytes()
            ),
            seed=seed,
            seed_evidence=f"manifest sha256:{manifest_digest}; {manifest['seed_scope']}",
        )
        csv = next((directory / "project").rglob("cell_count_*.csv"))
        native = importer.import_cell_counts(spec, csv.read_bytes())
        scalar = Reference(id=id + ".basal", version="1.0.0")
        importer.extract_scalar(
            native.reference, scalar, "Basal", f"bounded-simulation-seed:{seed}"
        )
        print(
            json.dumps(
                {
                    "stage": "run_complete",
                    "role": role,
                    "seed": seed,
                    "elapsed_seconds": record["elapsed_seconds"],
                }
            ),
            flush=True,
        )
        return scalar

    refs = tuple(run("reference", seed) for seed in (7, 11))
    evaluator = RepeatedSeedService(DatasetService(service.db, service.artifacts))
    protocol = RepeatedSeedProtocol(
        id=f"vcornea.epic.{args.output.name}",
        version="1.0.0",
        context=CONTEXT,
        endpoint="vcornea.basal_count",
        references=refs,
        tolerance=Quantity(value=0.0, unit="count"),
    )
    frozen = evaluator.freeze(protocol)
    (args.output / "frozen-seeds.json").write_text(frozen.model_dump_json(indent=2) + "\n")
    candidates = tuple(run("candidate", seed) for seed in (7, 11))
    report = evaluator.compare(Reference(id=protocol.id, version=protocol.version), candidates)
    if evaluator.get_report(report.id) != report:
        raise ValueError("Stored paired-seed report differs")
    summary = {
        "acceptance_digest": digest(acceptance),
        "image_id": image_id,
        "published_reference_outputs": reference_outputs,
        "seed_report": report.model_dump(mode="json"),
        "epic_engineering_checks_passed": report.passed,
        "published_scenario_reproduction": "not_established_by_bounded_runs",
        "independent_biological_validation": False,
    }
    (args.output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary, indent=2))
    if not report.passed:
        raise SystemExit("Paired-seed regression failed; preserve the recorded negative result")


if __name__ == "__main__":
    main()
