"""Verify actual native/reference DAG transfers, source bytes and cancellation.

Requires the ordinary Worker, reviewed COPASI native agent and admitted transfer
model. Creates retained records; never resets existing models/data or pulls images.
"""

import argparse
import json
import math
import os
import time
import uuid
from pathlib import Path

from peykar.simulation.native_agent import NativeAgentClient
from peykar_core.hashing import bytes_digest, digest


def verify(base_url: str, output: Path) -> dict:
    client = NativeAgentClient(base_url, os.getenv("PEYKAR_API_TOKEN", ""))
    output.mkdir(parents=True, exist_ok=False)
    root = Path(__file__).resolve().parents[1]

    def load(name):
        return json.loads((root / "experiments/flows" / name).read_bytes())

    client.request("/v1/models", load("reference-model.json"))
    client.request("/v1/native-interfaces", load("native-interface.json"))
    original = client.request(
        "/v1/native-models/native.transfer/versions/1.0.0/content", binary=True
    )
    reports = []

    def wait(id):
        deadline = time.monotonic() + 90
        while time.monotonic() < deadline:
            value = client.request("/v1/hybrid-runs/" + id)
            if value["status"] in ("succeeded", "failed", "cancelled"):
                return value
            time.sleep(0.2)
        raise TimeoutError("Flow timeout; all submitted jobs are retained")

    for file, horizon in [("native-reference-native.json", 30), ("native-native.json", 20)]:
        spec = load(file)
        frozen = client.request("/v1/hybrid-flows", spec)
        assert client.request("/v1/hybrid-flows", spec) == frozen
        ref = {"id": spec["id"], "version": spec["version"]}
        key = "verify-flow-" + uuid.uuid4().hex
        run = client.request("/v1/hybrid-runs", ref, idempotency_key=key)
        assert client.request("/v1/hybrid-runs", ref, idempotency_key=key)["id"] == run["id"]
        print(json.dumps({"flow_run_id": run["id"], "nodes": len(run["nodes"])}), flush=True)
        done = wait(run["id"])
        assert done["status"] == "succeeded", done
        report = client.request("/v1/hybrid-runs/" + run["id"] + "/results")
        actual = report["nodes"][-1]["terminal_values"]["A"]["value"]
        assert abs(actual - math.exp(-0.1 * horizon)) < 5e-6, (actual, horizon)
        for node in done["nodes"]:
            transfers = client.request(
                f"/v1/hybrid-runs/{run['id']}/nodes/{node['node_id']}/transfers"
            )
            assert digest(transfers) == node["transfer_digest"]
            if node["derivation"]:
                ref = node["derivation"]
                receipt = client.request(
                    f"/v1/native-derivations/{ref['id']}/versions/{ref['version']}"
                )
                assert ref["id"] == "derived." + digest(receipt)
                assert receipt["parent"]["content_digest"] == bytes_digest(original)
                content = client.request(
                    f"/v1/native-models/{ref['id']}/versions/{ref['version']}/content", binary=True
                )
                assert bytes_digest(content) == receipt["child_content_digest"]
                assert content != original
        reports.append(
            {
                "result": report,
                "expected_terminal": math.exp(-0.1 * horizon),
                "actual_terminal": actual,
                "idempotency_verified": True,
            }
        )
    run = client.request(
        "/v1/hybrid-runs", {"id": "flow.native-reference-native", "version": "1.0.0"}
    )
    prefix = "/v1/hybrid-runs/" + run["id"]
    deadline = time.monotonic() + 30
    while time.monotonic() < deadline:
        state = client.request(prefix)
        if state["nodes"][0]["status"] == "running":
            break
        time.sleep(0.05)
    else:
        raise TimeoutError("Native source did not start")
    client.request(prefix + "/cancel", {})
    cancelled = wait(run["id"])
    assert cancelled["status"] == "cancelled"
    assert [n["status"] for n in cancelled["nodes"]] == ["cancelled", "skipped", "skipped"]
    assert (
        client.request("/v1/native-models/native.transfer/versions/1.0.0/content", binary=True)
        == original
    )
    proof = {
        "successful_flows": reports,
        "running_cancellation": cancelled,
        "original_model_bytes_unchanged": True,
        "scientific_validity": "not_established",
    }
    (output / "verification.json").write_text(json.dumps(proof, indent=2) + "\n")
    return proof


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-url", default="http://127.0.0.1:8000")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    verify(args.base_url, args.output)
    print(json.dumps({"status": "passed", "report": str(args.output / "verification.json")}))
