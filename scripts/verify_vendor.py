"""Verify permanent source bytes and dependency completeness without executing them."""

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def contained(root: Path, relative: str) -> Path:
    path = PurePosixPath(relative)
    if path.is_absolute() or not path.parts or any(p in ("..", ".git") for p in path.parts):
        raise ValueError("Unsafe inventory path")
    target = root.joinpath(*path.parts)
    if any(parent.is_symlink() for parent in target.parents if parent != root.parent):
        raise ValueError("Inventory path traverses a symlink")
    return target


def verify(root: Path) -> dict[str, int]:
    manifest = json.loads((root / "vendor/manifest.json").read_bytes())
    seen: set[str] = set()
    files = 0
    dependencies = 0
    total = 0
    for release in manifest["releases"]:
        if release["id"] in seen or release.get("missing_dependencies"):
            raise ValueError("Duplicate source identity or incomplete dependency tree")
        seen.add(release["id"])
        source = contained(root, release["path"])
        if release["id"] == "vcornea":
            pins = json.loads(
                (root / "engine_adapters/compucell3d/vcornea-source-16884414.json").read_bytes()
            )
            entries = [{"path": p, "kind": "file", "sha256": h} for p, h in pins["files"].items()]
        else:
            raw = contained(root, release["inventory"]).read_bytes()
            if sha(raw) != release["inventory_sha256"]:
                raise ValueError("Inventory checksum mismatch")
            entries = json.loads(raw)
        expected = set()
        for entry in entries:
            path = contained(source, entry["path"])
            if entry["kind"] == "gitlink":
                if not entry["materialized"] or not path.is_dir():
                    raise ValueError("Unmaterialized source dependency")
                dependencies += 1
                continue
            expected.add(entry["path"])
            if entry["kind"] == "symlink":
                if not path.is_symlink():
                    raise ValueError("Missing source symlink")
                data = os.readlink(path).encode()
            else:
                if path.is_symlink():
                    raise ValueError("Unexpected source symlink")
                data = path.read_bytes()
                if data.startswith(b"version https://git-lfs.github.com/spec/v1\n"):
                    raise ValueError("Unresolved Git LFS pointer: " + str(path))
                if "mode" in entry and bool(path.stat().st_mode & 0o111) != (
                    entry["mode"] == "100755"
                ):
                    raise ValueError("Source executable mode differs")
            if sha(data) != entry["sha256"]:
                raise ValueError("Source checksum mismatch: " + str(path))
            files += 1
            total += len(data)
        actual = {
            str(p.relative_to(source)) for p in source.rglob("*") if p.is_file() or p.is_symlink()
        }
        if actual != expected:
            raise ValueError("Unexpected/missing files in source tree: " + release["id"])
        for name, checksum in release.get("license_evidence", {}).items():
            if sha(contained(source, name).read_bytes()) != checksum:
                raise ValueError("Original license evidence changed")
    return {
        "releases": len(seen),
        "files": files,
        "bytes": total,
        "materialized_gitlinks": dependencies,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    print(json.dumps(verify(args.root.resolve()), indent=2))


if __name__ == "__main__":
    main()
