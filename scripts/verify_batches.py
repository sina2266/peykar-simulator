"""Verify live cohort sweeps, native/reference orchestration and cancellation.

Requires the ordinary Worker and reviewed native host agent to be running.
All created versions/runs remain in the existing workspace. Never resets data.
"""

import argparse
import json
import math
import os
import statistics
import time
import uuid
from pathlib import Path

from peykar.simulation.native_agent import NativeAgentClient
from peykar_core.contracts import BatchReport, BatchSpec, SimulationResult
from peykar_core.hashing import digest


def verify(base_url: str, output: Path) -> dict:
    client = NativeAgentClient(base_url, os.getenv("PEYKAR_API_TOKEN", ""))
    output.mkdir(parents=True, exist_ok=False)
    root = Path(__file__).resolve().parents[1]
    reports = []

    def run_spec(path):
        spec = BatchSpec.model_validate_json((root / path).read_bytes())
        frozen = client.request("/v1/batches", spec.model_dump(mode="json"))
        assert client.request("/v1/batches", spec.model_dump(mode="json")) == frozen
        reference = {"id": spec.id, "version": spec.version}
        key = "verify-cohort-" + uuid.uuid4().hex
        record = client.request("/v1/batch-runs", reference, idempotency_key=key)
        repeated = client.request("/v1/batch-runs", reference, idempotency_key=key)
        assert repeated["id"] == record["id"]
        print(
            json.dumps({"batch_run_id": record["id"], "members": len(record["members"])}),
            flush=True,
        )
        return record, frozen

    def wait(id):
        deadline = time.monotonic() + 120
        while time.monotonic() < deadline:
            record = client.request("/v1/batch-runs/" + id)
            if record["status"] in ("succeeded", "cancelled", "failed"):
                return record
            time.sleep(0.25)
        raise TimeoutError("Cohort timeout; existing jobs retained for inspection")

    for fixture in ("reference-sweep.json", "mixed-native.json"):
        run, frozen = run_spec("experiments/batches/" + fixture)
        done = wait(run["id"])
        assert done["status"] == "succeeded", done
        prefix = "/v1/batch-runs/" + run["id"]
        result = BatchReport.model_validate(client.request(prefix + "/results"))
        assert all(group.complete for group in result.groups)
        assert result.run.batch_digest == digest(frozen)
        templates = {member["id"]: member for member in frozen["members"]}
        reference_seeds = []
        for group in result.groups:
            ids = [
                m.id
                for m in BatchSpec.model_validate(frozen["spec"]).cases
                if m.id == group.case_id
            ]
            assert ids
            members = [
                m
                for m in result.members
                if templates[m.member_id]["case_id"] == group.case_id
                and templates[m.member_id]["grid_index"] == group.grid_index
            ]
            if members[0].queue == "reference":
                samples = []
                for member in members:
                    raw = client.request("/v1/simulations/" + member.run_id + "/results")
                    value = SimulationResult.model_validate(raw)
                    assert digest(value) == member.result_digest
                    reference_seeds.append(value.seed)
                    samples.append(value.outputs[0].values[-1])
                endpoint = group.endpoints[0]
                assert math.isclose(endpoint.mean, statistics.mean(samples), rel_tol=1e-12)
                if len(samples) > 1:
                    assert math.isclose(
                        endpoint.sample_standard_deviation,
                        statistics.stdev(samples),
                        rel_tol=1e-12,
                        abs_tol=1e-12,
                    )
        assert len(reference_seeds) == len(set(reference_seeds))
        reports.append(result.model_dump(mode="json"))
    cancelled, _ = run_spec("experiments/batches/native-cancel.json")
    prefix = "/v1/batch-runs/" + cancelled["id"]
    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        state = client.request(prefix)
        if state["members"][0]["status"] == "running":
            break
        time.sleep(0.05)
    else:
        raise TimeoutError("Native cohort member did not start")
    client.request(prefix + "/cancel", {})
    state = wait(cancelled["id"])
    assert state["status"] == "cancelled"
    assert state["members"][0]["status"] == "cancelled"
    assert all(m["status"] == "skipped" for m in state["members"][1:])
    cancelled_report = client.request(prefix + "/results")
    assert all(not group["complete"] for group in cancelled_report["groups"])
    proof = {
        "successful_cohorts": reports,
        "running_native_cancellation": cancelled_report,
        "scientific_validity": "not_established",
    }
    (output / "verification.json").write_text(json.dumps(proof, indent=2) + "\n")
    return proof


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-url", default="http://127.0.0.1:8000")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    verify(args.base_url, args.output)
    print(json.dumps({"status": "passed", "report": str(args.output / "verification.json")}))
