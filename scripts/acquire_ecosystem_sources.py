"""Explicitly fetch pinned upstream source trees; never install or run their code."""

import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import tempfile
from pathlib import Path


def acquire(entry: dict[str, str], root: Path) -> dict[str, str]:
    repository = entry["repository"]
    commit = entry["commit"]
    if not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
        raise ValueError("Expected a GitHub owner/repository")
    if not re.fullmatch(r"[0-9a-f]{40}", commit):
        raise ValueError("Expected an exact commit")
    name = repository.replace("/", "--")
    destination = root / name
    env = {
        **os.environ,
        "GIT_TERMINAL_PROMPT": "0",
        "GIT_LFS_SKIP_SMUDGE": "1",
    }
    base = ["git", "-c", "core.hooksPath=/dev/null", "-c", "filter.lfs.required=false"]

    def run(arguments: list[str], cwd: Path) -> str:
        return subprocess.check_output(
            base + arguments, cwd=cwd, env=env, stderr=subprocess.STDOUT, timeout=1800
        ).decode()

    if destination.exists():
        actual = run(["rev-parse", "HEAD"], destination).strip()
        if actual != commit:
            raise ValueError(f"Existing source {name} has another revision; preserved")
        return {"repository": repository, "commit": actual, "status": "existing_preserved"}
    with tempfile.TemporaryDirectory(prefix=f".{name}-", dir=root) as temporary:
        staging = Path(temporary)
        run(["init", "--template=", "."], staging)
        run(["remote", "add", "origin", f"https://github.com/{repository}.git"], staging)
        run(["fetch", "--depth", "1", "origin", commit], staging)
        run(["checkout", "--detach", commit], staging)
        if run(["rev-parse", "HEAD"], staging).strip() != commit:
            raise ValueError("Fetched source revision differs")
        staging.rename(destination)
    return {"repository": repository, "commit": commit, "status": "source_fetched"}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--destination", required=True, type=Path)
    parser.add_argument("--jobs", type=int, choices=(1, 2, 3), default=2)
    args = parser.parse_args()
    entries = json.loads(args.catalog.read_bytes())
    args.destination.mkdir(parents=True, exist_ok=True)
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        pending = {pool.submit(acquire, entry, args.destination): entry for entry in entries}
        for future in concurrent.futures.as_completed(pending):
            entry = pending[future]
            try:
                result = future.result()
            except Exception as exc:
                result = {"repository": entry["repository"], "status": "failed", "error": str(exc)}
            results.append(result)
            print(json.dumps(result), flush=True)
    (args.destination / "acquisition.json").write_text(json.dumps(results, indent=2) + "\n")
    if any(item["status"] == "failed" for item in results):
        raise SystemExit("Source acquisition incomplete; existing sources preserved")


if __name__ == "__main__":
    main()
