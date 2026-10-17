"""Fetch pinned submodule sources without executing upstream hooks or build scripts."""

import argparse
import concurrent.futures
import configparser
import json
import os
import re
import subprocess
import tempfile
from pathlib import Path, PurePosixPath
from urllib.parse import urljoin, urlparse

BASE = ["git", "-c", "core.hooksPath=/dev/null", "-c", "filter.lfs.required=false"]
ENV = {**os.environ, "GIT_TERMINAL_PROMPT": "0", "GIT_LFS_SKIP_SMUDGE": "1"}


def git(root: Path, *args: str) -> str:
    return subprocess.check_output(
        BASE + ["-C", str(root), *args], env=ENV, stderr=subprocess.STDOUT, timeout=900
    ).decode()


def discover(root: Path, parent: str) -> list[dict[str, str]]:
    if not (root / ".gitmodules").is_file():
        return []
    config = configparser.RawConfigParser()
    config.read(root / ".gitmodules")
    links = {}
    for entry in git(root, "ls-tree", "-r", "HEAD").splitlines():
        metadata, path = entry.split("\t", 1)
        mode, kind, commit = metadata.split()
        if mode == "160000":
            links[path] = commit
    result = []
    for section in config.sections():
        path = config.get(section, "path")
        url = config.get(section, "url")
        if url.startswith(("./", "../")):
            remote = git(root, "remote", "get-url", "origin").strip()
            url = urljoin(remote.rstrip("/") + "/", url)
        if path not in links:
            continue
        p = PurePosixPath(path)
        parsed = urlparse(url)
        if (
            p.is_absolute()
            or ".." in p.parts
            or any((root / part).is_symlink() for part in p.parents)
        ):
            raise ValueError("Unsafe dependency destination")
        if (
            parsed.scheme != "https"
            or parsed.hostname not in ("github.com", "gitlab.com")
            or parsed.username
        ):
            raise ValueError("Dependency requires reviewed HTTPS repository: " + url)
        if not re.fullmatch("[a-f0-9]{40}", links[path]):
            raise ValueError("Unpinned dependency")
        result.append(
            {
                "parent": parent,
                "path": path,
                "url": url,
                "commit": links[path],
                "root": str(root),
                "destination": str(root / path),
            }
        )
    return result


def fetch(entry: dict[str, str]) -> dict[str, str]:
    destination = Path(entry["destination"])
    if (destination / ".git").exists():
        if git(destination, "rev-parse", "HEAD").strip() != entry["commit"]:
            raise ValueError("Existing dependency pin differs; preserved")
        return entry | {"status": "existing_verified"}
    if destination.exists():
        destination.rmdir()  # Only an empty Git link placeholder.
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".source-", dir=destination.parent) as tmp:
        staging = Path(tmp)
        git(staging, "init", "--template=", ".")
        git(staging, "remote", "add", "origin", entry["url"])
        git(staging, "fetch", "--depth", "1", "origin", entry["commit"])
        git(staging, "checkout", "--detach", entry["commit"])
        assert git(staging, "rev-parse", "HEAD").strip() == entry["commit"]
        staging.rename(destination)
    return entry | {"status": "fetched"}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    pending = []
    for root in args.sources.iterdir():
        if (root / ".git").exists():
            pending.extend(discover(root, root.name))
    results = []
    for _depth in range(6):
        if not pending:
            break
        next_level = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
            tasks = {pool.submit(fetch, entry): entry for entry in pending}
            for future in concurrent.futures.as_completed(tasks):
                entry = tasks[future]
                try:
                    result = future.result()
                    next_level.extend(
                        discover(Path(result["destination"]), entry["parent"] + "/" + entry["path"])
                    )
                except Exception as exc:
                    result = entry | {"status": "failed", "error": str(exc)}
                results.append(result)
                args.receipt.parent.mkdir(parents=True, exist_ok=True)
                args.receipt.write_text(json.dumps(results, indent=2) + "\n")
                print(
                    json.dumps(
                        {k: v for k, v in result.items() if k not in ("root", "destination")}
                    ),
                    flush=True,
                )
        pending = next_level
    if pending or any(r["status"] == "failed" for r in results):
        raise SystemExit("Dependency acquisition incomplete; see receipt")


if __name__ == "__main__":
    main()
