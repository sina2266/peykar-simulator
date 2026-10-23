"""Private native-child supervisor that durably records the actual solver exit.

This stdlib-only file is copied into an explicitly prepared operator workspace.
It survives loss of the outer CLI session; it never fabricates a missing exit code.
"""

import argparse
import hashlib
import json
import math
import os
import signal
import subprocess
import sys
import time
from pathlib import Path
from typing import Any


def identity(value: Any) -> str:
    raw = json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False, allow_nan=False
    ).encode()
    return hashlib.sha256(raw).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--driver", required=True, type=Path)
    parser.add_argument("--wall-seconds", required=True, type=float)
    args, remaining = parser.parse_known_args()
    if not math.isfinite(args.wall_seconds) or not 1 <= args.wall_seconds <= 86400:
        raise ValueError("Invalid native deadline")
    root = Path(__file__).resolve().parent
    if args.driver.resolve() != root / "driver.py" or (root / "completion.json").exists():
        raise ValueError("Invalid driver or previously completed workspace")
    manifest = json.loads((root / "manifest.json").read_bytes())
    runtime = json.loads((root / "runtime-profile.json").read_bytes())
    if hashlib.sha256(args.driver.read_bytes()).hexdigest() != manifest["driver_digest"]:
        raise ValueError("Native driver changed")
    # The solver has its own kernel alarm; bound the guard independently too.
    signal.signal(signal.SIGALRM, signal.SIG_DFL)
    signal.alarm(math.ceil(args.wall_seconds) + 30)
    started = time.monotonic()
    process = subprocess.run(
        [sys.executable, str(args.driver), "--wall-seconds", str(args.wall_seconds), *remaining],
        check=False,
    )
    outputs = {}
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        original = relative.parts[0] == "project" and path.suffix in (".csv", ".parquet")
        observation = path.parent == root and (
            path.name.startswith(("cells-", "lattice-")) or path.name == "progress.jsonl"
        )
        if original or observation:
            outputs[relative.as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    proof = {
        "schema_version": "1.0.0",
        "scope": "observed_native_child_exit",
        "manifest_digest": identity(manifest),
        "runtime_digest": identity(runtime),
        "returncode": process.returncode,
        "elapsed_seconds": time.monotonic() - started,
        "outputs": outputs,
    }
    temporary = root / "completion.json.tmp"
    with temporary.open("x") as stream:
        stream.write(json.dumps(proof, sort_keys=True, separators=(",", ":")) + "\n")
        stream.flush()
        os.fsync(stream.fileno())
    temporary.replace(root / "completion.json")
    raise SystemExit(process.returncode if process.returncode >= 0 else 128 - process.returncode)


if __name__ == "__main__":
    main()
