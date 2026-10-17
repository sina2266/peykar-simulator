"""Verify actual native model admission, durable jobs and pinned local solvers.

Uses the existing API token and data. Stop other native agents first; all new runs
and artifacts remain recorded. Does not install runtimes or pull Docker images.
"""

import argparse
import csv
import io
import json
import math
import os
import uuid
from pathlib import Path

from peykar.simulation.native_agent import NativeAgent, NativeAgentClient
from peykar_core.contracts import NativeRunManifest, NativeRunRecord, NativeRunResult, Quantity
from peykar_core.hashing import bytes_digest, digest
from peykar_core.units import convert


def verify(base_url: str, catalog: Path, output: Path, pkml_model_id: str | None) -> dict:
    client = NativeAgentClient(base_url, os.environ.get("PEYKAR_API_TOKEN", ""))
    active = client.request("/v1/native-runs?limit=200")
    if any(run["status"] in ("queued", "running") for run in active):
        raise ValueError("Verify in an idle native queue; preserve existing jobs")
    output.mkdir(parents=True, exist_ok=False)
    fixture = Path(__file__).resolve().parents[1] / "model_packages/native/transfer/1.0.0"
    spec = json.loads((fixture / "spec.json").read_bytes())
    raw = (fixture / "model.sbml").read_bytes()
    admitted = client.request("/v1/native-models", {"spec": spec, "xml_text": raw.decode()})
    if admitted["content_digest"] != bytes_digest(raw):
        raise ValueError("Admitted model bytes changed")
    plans = [
        {
            "model": {"id": spec["id"], "version": spec["version"]},
            "profile": {"id": "copasi.sbml", "version": "1.0.0"},
            "duration": {"value": 1, "unit": "minute"},
            "intervals": 10,
            "wall_seconds": 120,
        }
    ]
    if pkml_model_id:
        plans.append(
            {
                "model": {"id": pkml_model_id, "version": "1.0.0"},
                "profile": {"id": "osp.pkml", "version": "1.0.0"},
                "wall_seconds": 180,
            }
        )
    ids = []
    for plan in plans:
        key = "verify-native-" + uuid.uuid4().hex
        run = client.request("/v1/native-runs", plan, idempotency_key=key)
        repeated = client.request("/v1/native-runs", plan, idempotency_key=key)
        if repeated["id"] != run["id"]:
            raise ValueError("Native idempotency failed")
        ids.append(run["id"])
        print(json.dumps({"run_id": run["id"], "status": run["status"]}), flush=True)
    agent = NativeAgent(client, catalog, output / "runs")
    executed = [agent.run_once() for _ in ids]
    if set(executed) != set(ids):
        raise ValueError("Queue changed during verification; inspect retained runs")
    reports = []
    for id in ids:
        prefix = "/v1/native-runs/" + id
        record = NativeRunRecord.model_validate(client.request(prefix))
        manifest = NativeRunManifest.model_validate(client.request(prefix + "/manifest"))
        result = NativeRunResult.model_validate(client.request(prefix + "/results"))
        if record.status != "succeeded" or digest(manifest) != record.manifest_digest:
            raise ValueError("Native run/manifest identity differs")
        if digest(result) != record.result_digest or result.manifest_digest != digest(manifest):
            raise ValueError("Native result identity differs")
        blobs = {
            name: client.request(prefix + "/artifacts/" + name, binary=True)
            for name in result.artifacts
        }
        if any(bytes_digest(raw) != result.artifacts[name] for name, raw in blobs.items()):
            raise ValueError("Downloaded native artifact bytes differ")
        metadata = json.loads(blobs["quantities.json"])
        rows = list(csv.DictReader(io.StringIO(blobs["native.csv"].decode())))
        report = {
            "run": record.model_dump(mode="json"),
            "result": result.model_dump(mode="json"),
            "native_metadata": metadata,
            "idempotency_verified": True,
        }
        if manifest.profile_snapshot.kind == "copasi_sbml":
            unit = metadata["model_units"]["time_unit"]
            seconds = [convert(Quantity(value=float(r["Time"]), unit=unit), "second") for r in rows]
            errors = [
                abs(float(r["A"]) - math.exp(-0.1 * t)) for r, t in zip(rows, seconds, strict=True)
            ]
            conservation = max(abs(float(r["A"]) + float(r["B"]) - 1) for r in rows)
            if abs(seconds[-1] - 60) > 1e-9 or max(errors) > 5e-6 or conservation > 1e-9:
                raise ValueError("COPASI analytical/unit-conversion verification failed")
            report.update(max_analytic_error=max(errors), max_conservation_error=conservation)
        events = client.request(prefix + "/events")
        if [event["event"] for event in events] != ["submitted", "started", "succeeded"]:
            raise ValueError("Native lifecycle differs")
        reports.append(report)
    pending = client.request("/v1/native-runs", plans[0])
    cancelled = client.request("/v1/native-runs/" + pending["id"] + "/cancel", {})
    if cancelled["status"] != "cancelled" or agent.run_once() is not None:
        raise ValueError("Cancelled native job was claimable")
    report = {
        "runs": reports,
        "queued_cancellation": cancelled,
        "scientific_validity": "not_established",
        "image_installation_or_pull": False,
    }
    (output / "verification.json").write_text(json.dumps(report, indent=2) + "\n")
    return report


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-url", default="http://127.0.0.1:8000")
    parser.add_argument(
        "--catalog", type=Path, default=Path("runtime_profiles/native_adapters/local-catalog.json")
    )
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--pkml-model-id")
    args = parser.parse_args()
    verified = verify(args.base_url, args.catalog, args.output, args.pkml_model_id)
    print(
        json.dumps(
            {
                "status": "passed",
                "runs": len(verified["runs"]),
                "report": str(args.output / "verification.json"),
            }
        )
    )
