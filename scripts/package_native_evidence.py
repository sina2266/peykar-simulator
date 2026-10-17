"""Package actual retained source/runtime evidence for offline registry admission."""

import argparse
import json
from pathlib import Path

from peykar_core.contracts import (
    LibrarySourceReceipt,
    LibrarySourceSpec,
    NativeRuntimeEvidence,
    Reference,
)
from peykar_core.hashing import bytes_digest, canonical_json, digest

ROOT = Path(__file__).resolve().parents[1]
SOURCES = {
    "copasi": ("copasi",),
    "neuron": ("neuron",),
    "gears": ("gears",),
    "gears-data": ("gears",),
    "physicell": ("physicell",),
    "chaste": ("chaste",),
    "vcell": ("vcell",),
    "chaste-delta-notch": ("chaste",),
    "physiboss": ("physicell", "maboss"),
    "osp": ("pk-sim", "mobi", "ospsuite-r", "rsharp"),
    "vcornea": ("compucell3d",),
}


def package(output: Path, runs: Path) -> dict[str, object]:
    bundle = output / "artifacts"
    bundle.mkdir(parents=True, exist_ok=True)

    def retain(raw: bytes) -> str:
        checksum = bytes_digest(raw)
        target = bundle / checksum
        if target.exists() and target.read_bytes() != raw:
            raise ValueError("Existing admission artifact differs")
        if not target.exists():
            target.write_bytes(raw)
        return checksum

    distribution = json.loads((ROOT / "vendor/manifest.json").read_bytes())
    source_receipts = []
    for release in distribution["releases"]:
        name = release["id"]
        spec_path = ROOT / "engine_adapters/source-specs" / (name + ".json")
        if spec_path.exists():
            spec = json.loads(spec_path.read_bytes())
            spec["version"] = "2.0.0"
            spec["limitations"] = [
                "Complete materialized source/dependency inventory, including original notices.",
                "Source acquisition does not establish adapter or scientific validity.",
            ]
        elif name in ("maboss", "ospsuite-r", "rsharp"):
            spec = {
                "id": name,
                "version": "2.0.0",
                "title": name + " pinned complete source",
                "repository": release["repository"],
                "commit": release["commit"],
                "license": "BSD-3-Clause"
                if name == "maboss"
                else "GPL-2.0; retained original terms",
                "license_files": ["LICENSE"],
                "capabilities": ["scientific_runtime_dependency"],
                "limitations": ["Source identity and notices; no independent biological validity."],
            }
        else:
            continue
        raw = (ROOT / release["inventory"]).read_bytes()
        if bytes_digest(raw) != release["inventory_sha256"] or release["missing_dependencies"]:
            raise ValueError("Source inventory is incomplete or changed")
        entries = json.loads(raw)
        licenses = {
            path: retain((ROOT / release["path"] / path).read_bytes())
            for path in spec["license_files"]
        }
        receipt = LibrarySourceReceipt(
            spec=LibrarySourceSpec.model_validate(spec),
            inventory_digest=retain(raw),
            license_digests=licenses,
            tracked_files=len(entries),
            tracked_bytes=sum(e.get("bytes", 0) for e in entries),
        )
        source_receipts.append(receipt.model_dump(mode="json"))
        (output / ("source-" + name + ".json")).write_text(receipt.model_dump_json(indent=2) + "\n")
    catalog = json.loads((ROOT / "runtime_profiles/catalog.json").read_bytes())["profiles"]
    retained_runs = json.loads(runs.read_bytes())
    evidence_records = []
    for profile, relative in retained_runs.items():
        directory = ROOT / relative
        execution_raw = (directory / "execution.json").read_bytes()
        execution = json.loads(execution_raw)
        if execution["status"] != "passed" or execution["profile"] != profile:
            raise ValueError("Native run did not pass")
        for name, expected in execution["artifacts"].items():
            path = directory / name
            if not path.resolve().is_relative_to(directory.resolve()) or path.is_symlink():
                raise ValueError("Unsafe native artifact path")
            if retain(path.read_bytes()) != expected:
                raise ValueError("Retained native output changed")
        record = NativeRuntimeEvidence(
            id="runtime." + profile,
            version="1.0.0",
            title=profile + " actual native execution evidence",
            engine=catalog[profile]["engine"].lower(),
            profile=profile,
            sources=tuple(Reference(id=name, version="2.0.0") for name in SOURCES[profile]),
            image_digest=execution["image_id"].removeprefix("sha256:"),
            execution_digest=retain(execution_raw),
            artifact_digests=execution["artifacts"],
            limitations=(
                "Retained operator evidence; management API does not execute this runtime.",
                (
                    "Native benchmark/QC execution does not establish original-paper reproduction "
                    "or independent biological validity."
                ),
                (
                    "Consult the original runtime receipt/model context; "
                    "general cross-engine feedback is unsupported."
                ),
            ),
        )
        evidence_records.append(record.model_dump(mode="json"))
        (output / ("runtime-" + profile + ".json")).write_text(
            record.model_dump_json(indent=2) + "\n"
        )
    # Retain the existing real injury-crossing CC3D run without rewriting its original record.
    directory = ROOT / ".peykar/external/injury600"
    original = json.loads((directory / "record.json").read_bytes())
    manifest = (directory / "manifest.json").read_bytes()
    if (
        original["status"] != "succeeded"
        or digest(json.loads(manifest)) != original["manifest_digest"]
    ):
        raise ValueError("Original vCornea run evidence differs")
    artifacts = dict(original["outputs"])
    for name in list(artifacts):
        if retain((directory / name).read_bytes()) != artifacts[name]:
            raise ValueError("Original vCornea output changed")
    artifacts["record.json"] = retain((directory / "record.json").read_bytes())
    artifacts["manifest.json"] = retain(manifest)
    execution = {
        "status": "passed",
        "profile": "vcornea",
        "image_id": original["image_id"],
        "artifacts": artifacts,
    }
    record = NativeRuntimeEvidence(
        id="runtime.vcornea",
        version="1.0.0",
        title="Actual 600-MCS vCornea/CC3D injury-crossing run",
        engine="compucell3d",
        profile="vcornea",
        sources=(Reference(id="compucell3d", version="2.0.0"),),
        image_digest=original["image_id"].removeprefix("sha256:"),
        execution_digest=retain(canonical_json(execution).encode()),
        artifact_digests=artifacts,
        limitations=(
            (
                "Admitted Zenodo 16884414 model; bounded engineering execution, "
                "not full published replay."
            ),
            (
                "Original execution record/manifest/outputs preserved; "
                "no independent tissue validation."
            ),
        ),
    )
    evidence_records.append(record.model_dump(mode="json"))
    (output / "runtime-vcornea.json").write_text(record.model_dump_json(indent=2) + "\n")
    result = {"sources": source_receipts, "runtimes": evidence_records}
    (output / "manifest.json").write_text(json.dumps(result, indent=2) + "\n")
    return {
        "source_receipts": len(source_receipts),
        "native_runtime_receipts": len(evidence_records),
        "artifacts": len(list(bundle.iterdir())),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--runs", type=Path, default=ROOT / "runtime_profiles/verified-runs.json")
    args = parser.parse_args()
    print(json.dumps(package(args.output, args.runs), indent=2))


if __name__ == "__main__":
    main()
