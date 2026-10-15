"""Exercise a running API and worker without resetting or deleting workspace data."""

import argparse
import json
import math
import os
import sys
import time
import uuid
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.parse import urlsplit
from urllib.request import HTTPRedirectHandler, Request, build_opener

from peykar_core.contracts import RunManifest, RunRecord, SimulationPlan, SimulationResult
from peykar_core.hashing import digest


class NoRedirect(HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        # A verification request must never forward credentials to another location.
        return None


def verify(base_url: str, token: str, plan: SimulationPlan, timeout: float) -> dict[str, object]:
    base_url = base_url.rstrip("/")
    parsed = urlsplit(base_url)
    if (
        parsed.scheme not in ("http", "https")
        or not parsed.netloc
        or parsed.query
        or parsed.fragment
        or parsed.username
        or parsed.password
    ):
        raise ValueError("Use an HTTP(S) API base URL without query or fragment")
    opener = build_opener(NoRedirect())
    deadline = time.monotonic() + timeout

    def request(path: str, body: object = None, key: str | None = None):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(
                "API/worker verification deadline exceeded; submitted run is retained"
            )
        headers = {"Authorization": f"Bearer {token}", "Content-Type": "application/json"}
        if key:
            headers["Idempotency-Key"] = key
        payload = None if body is None else json.dumps(body, allow_nan=False).encode()
        req = Request(base_url + path, data=payload, headers=headers)
        try:
            with opener.open(req, timeout=min(10, remaining)) as response:
                raw = response.read(32 * 1024 * 1024 + 1)
                if len(raw) > 32 * 1024 * 1024:
                    raise ValueError("API response exceeds verification limit")
                return json.loads(raw)
        except HTTPError as exc:
            # Do not print bodies, credentials, URLs containing credentials or raw datasets.
            raise ValueError(f"API returned HTTP {exc.code} for {path}") from None

    key = "verify-" + str(uuid.uuid4())
    body = plan.model_dump(mode="json")
    run = RunRecord.model_validate(request("/v1/simulations", body, key))
    print(json.dumps({"run_id": run.id, "status": run.status}), flush=True)
    prefix = f"/v1/simulations/{run.id}"
    repeated = RunRecord.model_validate(request("/v1/simulations", body, key))
    if repeated.id != run.id:
        raise ValueError("Idempotent submission returned a different run")
    while run.status in ("queued", "running"):
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(f"Timed out waiting for run {run.id}; run is retained")
        time.sleep(min(0.25, remaining))
        run = RunRecord.model_validate(request(prefix))
    if run.status != "succeeded":
        raise ValueError(f"Run {run.id} ended as {run.status} ({run.error or 'no error code'})")
    manifest = RunManifest.model_validate(request(prefix + "/manifest"))
    result = SimulationResult.model_validate(request(prefix + "/results"))
    if (
        manifest.plan != plan
        or manifest.run_id != run.id
        or digest(manifest) != run.manifest_digest
    ):
        raise ValueError("Returned manifest does not match the submitted run")
    if digest(result) != run.result_digest or result.seed != plan.seed:
        raise ValueError("Returned result does not match the committed run")
    events = request(prefix + "/events")
    if [event["event"] for event in events] != ["submitted", "started", "succeeded"]:
        raise ValueError("Unexpected lifecycle event sequence")
    return {
        "run_id": run.id,
        "status": run.status,
        "manifest_digest": run.manifest_digest,
        "result_digest": run.result_digest,
        "samples": len(result.time.values),
        "idempotency_verified": True,
        "evidence_class": "engineering_verification",
        "scientific_validity": "not_established",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--base-url", default="http://127.0.0.1:8000")
    parser.add_argument("--timeout", type=float, default=90)
    args = parser.parse_args()
    token = os.environ.get("PEYKAR_API_TOKEN", "")
    if len(token) < 24:
        parser.error("Set PEYKAR_API_TOKEN to the existing administrator token")
    if not math.isfinite(args.timeout) or args.timeout <= 0 or args.timeout > 3600:
        parser.error("Timeout must be greater than zero and at most 3600 seconds")
    try:
        plan = SimulationPlan.model_validate_json(args.manifest.read_bytes())
        print(json.dumps(verify(args.base_url, token, plan, args.timeout), indent=2))
    except (ValueError, TimeoutError, OSError, URLError) as exc:
        print(f"Verification failed: {exc}", file=sys.stderr)
        raise SystemExit(1) from None


if __name__ == "__main__":
    main()
