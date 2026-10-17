"""Explicit operator agent for admitted native jobs, using a local image/driver allowlist."""

import base64
import json
import os
import stat
import subprocess
import time
import uuid
from contextlib import suppress
from pathlib import Path
from typing import Any
from urllib.error import HTTPError
from urllib.parse import urlsplit
from urllib.request import HTTPRedirectHandler, Request, build_opener

from peykar_core.contracts import NativeAgentClaim, NativeRunResult, Reference
from peykar_core.errors import Cancelled, Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest, digest
from peykar_core.units import convert

OUTPUT_NAMES = ("native.csv", "quantities.json")


def bounded_output(path: Path) -> bytes:
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK)
    with os.fdopen(fd, "rb") as stream:
        info = os.fstat(stream.fileno())
        if not stat.S_ISREG(info.st_mode) or info.st_size > 1_000_000:
            raise InvalidRequest("Native output must be a bounded regular file")
        raw = stream.read(1_000_001)
        if len(raw) > 1_000_000:
            raise InvalidRequest("Native output exceeds 1 MB")
        return raw


class NoRedirect(HTTPRedirectHandler):
    def redirect_request(
        self, req: Any, fp: Any, code: Any, msg: Any, headers: Any, newurl: Any
    ) -> None:
        return None


class NativeAgentClient:
    def __init__(self, base_url: str, token: str) -> None:
        parsed = urlsplit(base_url)
        if (
            parsed.scheme not in ("http", "https")
            or not parsed.hostname
            or parsed.query
            or parsed.fragment
            or parsed.username
            or parsed.password
            or (
                parsed.scheme == "http" and parsed.hostname not in ("localhost", "127.0.0.1", "::1")
            )
            or len(token) < 24
        ):
            raise InvalidRequest(
                "Use HTTPS (or local loopback HTTP) and an existing administrator token"
            )
        self.base_url, self.token = base_url.rstrip("/"), token
        self.opener = build_opener(NoRedirect())

    def request(
        self,
        path: str,
        body: Any = None,
        *,
        claim_token: str | None = None,
        binary: bool = False,
        idempotency_key: str | None = None,
    ) -> Any:
        headers = {"Authorization": "Bearer " + self.token, "Content-Type": "application/json"}
        if claim_token:
            headers["Native-Claim-Token"] = claim_token
        if idempotency_key:
            headers["Idempotency-Key"] = idempotency_key
        data = None if body is None else json.dumps(body, allow_nan=False).encode()
        try:
            with self.opener.open(
                Request(self.base_url + path, data=data, headers=headers), timeout=3
            ) as response:
                raw = response.read(12 * 1024**2 + 1)
        except HTTPError as error:
            raise Conflict(f"Native agent API request returned HTTP {error.code}") from None
        if len(raw) > 12 * 1024**2:
            raise InvalidRequest("Native agent response exceeds 12 MB")
        return raw if binary else json.loads(raw)


class NativeAgent:
    def __init__(self, client: NativeAgentClient, catalog: Path, output: Path) -> None:
        self.client, self.output = client, output.resolve()
        self.output.mkdir(parents=True, exist_ok=True)
        entries = json.loads(catalog.read_bytes())
        self.entries: dict[tuple[str, str], dict[str, Any]] = {}
        for entry in entries:
            reference = Reference.model_validate(entry["profile"])
            driver = (catalog.parent / entry["driver_path"]).resolve(strict=True)
            if bytes_digest(driver.read_bytes()) != entry["driver_digest"]:
                raise Conflict("Locally admitted native driver changed")
            if entry["kind"] not in ("copasi_sbml", "osp_pkml"):
                raise InvalidRequest("Unsupported local native driver kind")
            key = (reference.id, reference.version)
            if key in self.entries:
                raise InvalidRequest("Duplicate local native execution profile")
            self.entries[key] = {**entry, "driver": driver}
        if not self.entries:
            raise InvalidRequest("Explicitly admit at least one local native execution profile")

    def run_once(self) -> str | None:
        raw_claim = self.client.request(
            "/v1/native-runs/claim",
            {
                "profiles": [
                    Reference(id=id, version=version).model_dump(mode="json")
                    for id, version in self.entries
                ]
            },
        )
        if raw_claim is None:
            return None
        claim = NativeAgentClaim.model_validate(raw_claim)
        manifest = claim.manifest
        prefix = "/v1/native-runs/" + manifest.run_id
        try:
            blobs = self.execute(claim)
            result = NativeRunResult(
                run_id=manifest.run_id,
                manifest_digest=digest(manifest),
                image_digest=manifest.runtime_snapshot.image_digest,
                driver_digest=manifest.profile_snapshot.driver_digest,
                artifacts={name: bytes_digest(raw) for name, raw in blobs.items()},
                limitations=(
                    "Actual native solver output; independent scientific validity remains open.",
                    "Original model, native units and exact image/driver identities retained.",
                ),
            )
            self.client.request(
                prefix + "/complete",
                {
                    "status": "succeeded",
                    "result": result.model_dump(mode="json"),
                    "blobs_base64": {
                        name: base64.b64encode(raw).decode() for name, raw in blobs.items()
                    },
                },
                claim_token=claim.token,
            )
        except (Exception, KeyboardInterrupt) as error:
            # Failure/lease diagnostics stay in the operator workspace; never publish fake output.
            with suppress(Exception):
                self.client.request(
                    prefix + "/complete",
                    {
                        "status": "cancelled"
                        if isinstance(error, (Cancelled, KeyboardInterrupt))
                        else "failed",
                    },
                    claim_token=claim.token,
                )
            raise
        return manifest.run_id

    def execute(self, claim: NativeAgentClaim) -> dict[str, bytes]:
        manifest = claim.manifest
        if str(uuid.UUID(manifest.run_id)) != manifest.run_id:
            raise InvalidRequest("Native run identity must be a canonical UUID")
        profile, runtime = manifest.profile_snapshot, manifest.runtime_snapshot
        entry = self.entries.get((profile.id, profile.version))
        if (
            entry is None
            or entry["kind"] != profile.kind
            or entry["driver_digest"] != profile.driver_digest
            or entry["image_digest"] != runtime.image_digest
        ):
            raise Conflict("Server job differs from the local image/driver allowlist")
        driver = Path(entry["driver"]).read_bytes()
        if bytes_digest(driver) != profile.driver_digest:
            raise Conflict("Native driver changed since agent initialization")
        raw = self.client.request(
            f"/v1/native-models/{manifest.plan.model.id}/versions/{manifest.plan.model.version}/content",
            binary=True,
        )
        if bytes_digest(raw) != manifest.model_snapshot.content_digest:
            raise Conflict("Native model bytes differ from the frozen manifest")
        job = self.output / manifest.run_id
        job.mkdir(mode=0o700, exist_ok=False)
        (job / "model.xml").write_bytes(raw)
        extension = ".py" if profile.kind == "copasi_sbml" else ".R"
        driver_file = job / ("driver" + extension)
        driver_file.write_bytes(driver)
        controls: dict[str, object] = {}
        if profile.kind == "copasi_sbml":
            if manifest.plan.duration is None or manifest.plan.intervals is None:
                raise InvalidRequest("Native duration/interval controls are missing")
            controls = {
                "duration_seconds": convert(manifest.plan.duration, "second"),
                "intervals": manifest.plan.intervals,
            }
        (job / "controls.json").write_text(json.dumps(controls) + "\n")
        (job / "manifest.json").write_text(manifest.model_dump_json(indent=2) + "\n")
        environment = {
            k: os.environ[k]
            for k in ("PATH", "HOME", "DOCKER_CONFIG", "DOCKER_HOST", "DOCKER_CONTEXT")
            if k in os.environ
        }
        image = "sha256:" + runtime.image_digest
        actual = subprocess.check_output(
            ["docker", "image", "inspect", image, "--format", "{{.Id}}"],
            env=environment,
            text=True,
            timeout=15,
        ).strip()
        if actual != image:
            raise Conflict("Approved native image is not installed locally")
        name = "peykar-native-" + uuid.uuid4().hex
        entrypoint = "/opt/runtime/.venv/bin/python" if profile.kind == "copasi_sbml" else "Rscript"
        command = [
            "docker",
            "create",
            "--rm",
            "--pull",
            "never",
            "--name",
            name,
            "--network",
            "none",
            "--hostname",
            "peykar-native",
            "--read-only",
            "--cap-drop",
            "ALL",
            "--security-opt",
            "no-new-privileges:true",
            "--cpus",
            "2",
            "--memory",
            "3g",
            "--pids-limit",
            "128",
            "--user",
            f"{os.getuid()}:{os.getgid()}" if os.getuid() else "10001:10001",
            "--ulimit",
            "fsize=1048576",
            "--tmpfs",
            "/tmp:size=128m,mode=1777",
            "--env",
            "HOME=/tmp",
            "--workdir",
            "/work",
            "--mount",
            f"type=bind,src={job},dst=/work",
        ]
        for filename in ("model.xml", "driver" + extension, "controls.json", "manifest.json"):
            command += ["--mount", f"type=bind,src={job / filename},dst=/work/{filename},readonly"]
        command += ["--entrypoint", entrypoint, image, "/work/driver" + extension]
        heartbeat = 0.0
        process = None
        created = False
        creation_attempted = False
        try:
            with (job / "engine.log").open("xb") as log:

                def check_cancel() -> None:
                    response = self.client.request(
                        "/v1/native-runs/" + manifest.run_id + "/heartbeat",
                        {},
                        claim_token=claim.token,
                    )
                    if response["cancel_requested"]:
                        raise Cancelled("Native run cancellation requested")

                check_cancel()
                # Acknowledge creation before starting/supervising the container.
                # Killing `docker run` during creation can otherwise race cleanup
                # and leave a late-created container after `docker rm` returned.
                creation_attempted = True
                subprocess.run(
                    command,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    env=environment,
                    timeout=15,
                    check=True,
                )
                created = True
                check_cancel()
                started = time.monotonic()
                process = subprocess.Popen(
                    ["docker", "start", "--attach", name],
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    env=environment,
                )
                while process.poll() is None:
                    elapsed = time.monotonic() - started
                    if elapsed > manifest.plan.wall_seconds:
                        raise TimeoutError("Native engine exceeded its wall budget")
                    if (job / "engine.log").stat().st_size > 1_000_000:
                        raise InvalidRequest("Native engine log exceeds 1 MB")
                    if elapsed >= heartbeat:
                        cancel = self.client.request(
                            "/v1/native-runs/" + manifest.run_id + "/heartbeat",
                            {},
                            claim_token=claim.token,
                        )
                        if cancel["cancel_requested"]:
                            raise Cancelled("Native run cancellation requested")
                        heartbeat = elapsed + 2
                    time.sleep(0.1)
            if process.returncode != 0:
                raise InvalidRequest("Native engine failed; inspect the retained operator log")
            if time.monotonic() - started > manifest.plan.wall_seconds:
                raise TimeoutError("Native engine finished after its wall budget")
            completion = json.loads(bounded_output(job / "native-completion.json"))
            if completion.get("status") != "succeeded":
                raise InvalidRequest("Native driver did not complete its output contract")
            blobs = {name: bounded_output(job / name) for name in OUTPUT_NAMES}
            if sum(map(len, blobs.values())) > 1_000_000:
                raise InvalidRequest("Native output paths/size violate the result contract")
            return blobs
        finally:
            # Stop the attached client before the final removal, so it cannot
            # submit another start after the cleanup command has finished.
            if process is not None and process.poll() is None:
                try:
                    with suppress(ProcessLookupError):
                        process.terminate()
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=5)
                except (OSError, subprocess.TimeoutExpired):
                    (job / "client-stop-warning.txt").write_text(
                        "Attached client stop was not confirmed; container cleanup follows.\n"
                    )
            try:
                cleanup = subprocess.run(
                    ["docker", "rm", "-f", name],
                    env=environment,
                    capture_output=True,
                    timeout=15,
                    check=False,
                )
                if (creation_attempted and not created) or (
                    cleanup.returncode and b"No such container" not in (cleanup.stderr or b"")
                ):
                    (job / "cleanup-warning.txt").write_text(
                        "Creation/cleanup could not be confirmed; inspect container " + name + "\n"
                    )
            except (OSError, subprocess.TimeoutExpired):
                (job / "cleanup-warning.txt").write_text(
                    "Docker daemon unreachable; inspect container " + name + "\n"
                )
