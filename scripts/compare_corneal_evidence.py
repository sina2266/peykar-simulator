"""Explicit offline experimental-data diagnostic on verified completed corneal runs.

Use the pinned native Python with platform/core src directories in PYTHONPATH.
No model is run, no parameter is fitted and no acceptance threshold is selected.
"""

import argparse
import csv
import io
import json
import sys
from hashlib import sha256
from pathlib import Path

from peykar.validation.cornea_evidence import (
    context_disagreements,
    epithelial_column_geometry,
    human_oct_summary,
    rabbit_wound_observations,
)


def bytes_digest(raw: bytes) -> str:
    return sha256(raw).hexdigest()


def run(evidence: Path, protocol_path: Path, series: list[Path], output: Path) -> None:
    import matplotlib
    import numpy as np

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    sys.path.insert(0, str(Path("runtime_profiles/vcornea-publication").resolve()))
    from compare import load_run

    protocol_raw = protocol_path.read_bytes()
    protocol = json.loads(protocol_raw)
    acquisition = json.loads((evidence / "acquisition.json").read_bytes())
    for relative, expected in acquisition["files"].items():
        path = (evidence / relative).resolve(strict=True)
        if (
            not path.is_relative_to(evidence.resolve())
            or bytes_digest(path.read_bytes()) != expected
        ):
            raise ValueError("Experimental source/normalized evidence changed")
    if (
        protocol["class"] != "exploratory_cross_context_diagnostic"
        or protocol["acceptance_threshold"] is not None
        or protocol["candidate_series"] != [str(p) for p in series]
    ):
        raise ValueError("Preserve the declared diagnostic and exact candidate series")
    human = human_oct_summary((evidence / "raw/buffault2020-manuscript.xml").read_bytes())
    if human != json.loads((evidence / "human-healthy-oct.json").read_bytes()):
        raise ValueError("Normalized clinical aggregates differ from original source")
    rabbit = rabbit_wound_observations((evidence / "raw/rabbit-figure2.xlsx").read_bytes())
    normalized = list(csv.DictReader(io.StringIO((evidence / "rabbit-wounds.csv").read_text())))
    if len(normalized) != len(rabbit) or any(
        row["source_cell"] != source["source_cell"]
        or float(row["wound_area_fraction"]) != source["wound_area_fraction"]
        for row, source in zip(normalized, rabbit, strict=True)
    ):
        raise ValueError("Normalized rabbit observations differ from measured cells")
    central = human["observations"]["Central"]
    observations = []
    candidate_hashes = {}
    for directory in series:
        plan_raw = (directory / "plan.json").read_bytes()
        plan = json.loads(plan_raw)
        frozen_raw = (directory / "comparison.json").read_bytes()
        frozen = json.loads(frozen_raw)
        if (
            plan["status"] != "execution_completed"
            or bytes_digest(frozen_raw) != plan["protocol_digest"]
        ):
            raise ValueError("Use complete frozen native series; never rerun here")
        candidate_hashes[str(directory)] = bytes_digest(plan_raw)
        for job in plan["jobs"]:
            if job["case"] != "self-organization":
                raise ValueError("The clinical healthy diagnostic requires formation-only inputs")
            root, manifest, record, *_ = load_run(plan, job, plan["protocol_digest"], frozen)
            horizon = manifest["controls"]["steps_mcs"]
            name = f"lattice-{horizon}.npz"
            if record["outputs"].get(name) != bytes_digest((root / name).read_bytes()):
                raise ValueError("Actual complete final lattice is not admitted")
            with np.load(root / name, allow_pickle=False) as saved:
                lattice = saved["cell_type"].tolist()
            geometry = epithelial_column_geometry(
                lattice, micrometres_per_site=protocol["nominal_micrometres_per_site"]
            )
            mean = geometry["mean_vertical_span_um"]
            observations.append(
                {
                    "series": str(directory),
                    "profile": plan["parameter_profile"],
                    "seed": job["seed"],
                    "manifest_digest": job["manifest_digest"],
                    "snapshot_sha256": record["outputs"][name],
                    "mcs": horizon,
                    "geometry": geometry,
                    "descriptive_mean_difference_um": None
                    if mean is None
                    else mean - central["mean"],
                    "clinical_accuracy_score": None,
                    "passed": None,
                }
            )
    report = {
        "schema_version": "1.0.0",
        "class": "exploratory_cross_context_diagnostic",
        "protocol_sha256": bytes_digest(protocol_raw),
        "operator_sha256": bytes_digest(Path(__file__).read_bytes()),
        "observation_operator_sha256": bytes_digest(
            Path("packages/platform/src/peykar/validation/cornea_evidence.py").read_bytes()
        ),
        "experimental_acquisition_sha256": bytes_digest(
            (evidence / "acquisition.json").read_bytes()
        ),
        "candidate_plan_sha256": candidate_hashes,
        "clinical_reference": human,
        "comparison_blocking_fields": context_disagreements(
            protocol["model_context"], protocol["clinical_context"]
        ),
        "rabbit_comparison_blocking_fields": context_disagreements(
            protocol["chemical_model_context"], protocol["rabbit_context"]
        ),
        "observations": observations,
        "independent_biological_validation": False,
        "passed": None,
        "limitations": protocol["limitations"],
    }
    output.mkdir(parents=True, exist_ok=False)
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    (output / "diagnostic-plan.json").write_bytes(protocol_raw)
    labels = [f"{o['profile']}\nseed{o['seed']}" for o in observations]
    fig, ax = plt.subplots(figsize=(11, 5), layout="constrained")
    ax.axhspan(
        central["mean"] - central["sample_sd"],
        central["mean"] + central["sample_sd"],
        color="#b7d9d4",
        alpha=0.6,
        label="22 healthy humans: central OCT mean ± subject SD",
    )
    ax.axhline(central["mean"], color="#18786f", linewidth=1.5)
    ax.scatter(
        range(len(labels)),
        [o["geometry"]["mean_vertical_span_um"] for o in observations],
        color="#c46c34",
        s=70,
        label="Model: final 2D vertical epithelial span, spatial mean",
    )
    ax.set(
        xticks=range(len(labels)),
        xticklabels=labels,
        ylabel="Micrometres (nominal model scale)",
        title="Exploratory geometry comparison — region/measurement mapping unresolved",
    )
    ax.legend(loc="best", fontsize=8)
    fig.savefig(output / "healthy-thickness-diagnostic.png", dpi=160)
    plt.close(fig)
    print("Verified completed native outputs:", len(observations))
    print("Clinical measurement/region blockers:", report["comparison_blocking_fields"])
    for observation in observations:
        print(
            observation["profile"],
            observation["seed"],
            observation["geometry"]["mean_vertical_span_um"],
            "µm; no accuracy/pass claim",
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--evidence", required=True, type=Path)
    parser.add_argument("--protocol", required=True, type=Path)
    parser.add_argument("--runs", action="append", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    run(args.evidence, args.protocol, args.runs, args.output)


if __name__ == "__main__":
    main()
