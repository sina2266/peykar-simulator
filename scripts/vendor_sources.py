"""Materialize complete pinned source working trees as ordinary project-owned vendor files.

Git administrative data is kept in the acquisition cache. Copies preserve upstream
bytes, modes, links and notices. Existing different destination files are never overwritten.
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
SOURCE_POLICY = "Pristine source and notices; execution copies/patches are separate"


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def verify_blob(source: Path, relative: str, oid: str, data: bytes, mode: str) -> None:
    actual = hashlib.sha1(
        b"blob " + str(len(data)).encode() + b"\0" + data, usedforsecurity=False
    ).hexdigest()
    if actual == oid:
        return
    if mode == "120000":
        raise ValueError("Tracked source symlink changed: " + relative)
    attrs = subprocess.check_output(
        ["git", "-C", str(source), "check-attr", "-z", "text", "eol", "--", relative]
    ).split(b"\0")
    values = dict(zip(attrs[1::3], attrs[2::3], strict=True))
    text = values.get(b"text") in (b"set", b"auto") or values.get(b"eol") in (b"lf", b"crlf")
    if not text:
        raise ValueError("Tracked source bytes changed: " + relative)
    original = subprocess.check_output(["git", "-C", str(source), "cat-file", "blob", oid])
    if data.replace(b"\r\n", b"\n") != original.replace(b"\r\n", b"\n"):
        raise ValueError("Tracked source bytes changed: " + relative)


def copy_tree(
    source: Path, target: Path, inventory: list[dict[str, object]], prefix: str = ""
) -> None:
    raw = subprocess.check_output(["git", "-C", str(source), "ls-tree", "-rz", "HEAD"])
    for item in raw.split(b"\0"):
        if not item:
            continue
        metadata, name = item.split(b"\t", 1)
        mode, kind, oid = metadata.decode().split()
        relative = name.decode()
        p = PurePosixPath(relative)
        if p.is_absolute() or ".." in p.parts or ".git" in p.parts:
            raise ValueError("Unsafe source path")
        origin = source / relative
        destination = target / relative
        logical = prefix + relative
        if target.is_symlink() or any((target / parent).is_symlink() for parent in p.parents):
            raise ValueError("Vendor destination traverses a symlink")
        if any((source / parent).is_symlink() for parent in p.parents):
            raise ValueError("Source path traverses a symlink")
        if mode == "160000":
            actual = None
            if (origin / ".git").exists():
                actual = subprocess.check_output(
                    ["git", "-C", str(origin), "rev-parse", "HEAD"], text=True
                ).strip()
            inventory.append(
                {"path": logical, "kind": "gitlink", "commit": oid, "materialized": actual == oid}
            )
            if actual is not None:
                if actual != oid:
                    raise ValueError("Dependency commit differs: " + logical)
                copy_tree(origin, destination, inventory, logical + "/")
            continue
        destination.parent.mkdir(parents=True, exist_ok=True)
        if mode == "120000":
            data = os.readlink(origin).encode()
            verify_blob(source, relative, oid, data, mode)
            if destination.is_symlink():
                if os.readlink(destination) != os.readlink(origin):
                    raise ValueError("Existing vendor symlink differs")
            elif destination.exists():
                raise ValueError("Existing vendor file differs from source symlink")
            else:
                destination.symlink_to(os.readlink(origin))
        else:
            if origin.is_symlink():
                raise ValueError("Tracked regular source file is a symlink")
            data = origin.read_bytes()
            verify_blob(source, relative, oid, data, mode)
            if destination.is_symlink():
                raise ValueError("Existing vendor path is a symlink")
            if destination.exists():
                if sha(destination.read_bytes()) != sha(data):
                    raise ValueError("Existing vendor file differs: " + logical)
            else:
                shutil.copy2(origin, destination)
            destination.chmod(0o755 if mode == "100755" else 0o644)
        inventory.append(
            {
                "path": logical,
                "kind": "symlink" if mode == "120000" else "file",
                "mode": mode,
                "sha256": sha(data),
                "bytes": len(data),
            }
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cache", type=Path, default=ROOT / ".peykar/external/ecosystem/sources")
    parser.add_argument(
        "--only", nargs="+", help="Materialize selected IDs, preserving other manifest records"
    )
    args = parser.parse_args()
    records = json.loads((ROOT / "engine_adapters/ecosystem-sources.json").read_bytes())
    extra_catalog = ROOT / "runtime_profiles/additional-sources.json"
    extra_records = json.loads(extra_catalog.read_bytes()) if extra_catalog.exists() else []
    known = (
        {r["source_inspection_id"] for r in records}
        | {r["id"] for r in extra_records}
        | {"vcornea"}
    )
    if args.only and not set(args.only) <= known:
        raise ValueError("Unknown source selection")
    releases = []
    if (ROOT / "vendor/manifest.json").exists():
        releases = [
            r
            for r in json.loads((ROOT / "vendor/manifest.json").read_bytes())["releases"]
            if r["id"] not in (args.only or known)
        ]
    for record in records:
        id = record["source_inspection_id"]
        if args.only and id not in args.only:
            continue
        source = args.cache / record["repository"].replace("/", "--")
        actual = subprocess.check_output(
            ["git", "-C", str(source), "rev-parse", "HEAD"], text=True
        ).strip()
        if actual != record["commit"]:
            raise ValueError("Source pin differs")
        destination = ROOT / "vendor/upstream" / id
        inventory = []
        copy_tree(source, destination, inventory)
        raw = (json.dumps(inventory, sort_keys=True, separators=(",", ":")) + "\n").encode()
        index = ROOT / "vendor/inventories" / (id + "-" + sha(raw) + ".json")
        index.parent.mkdir(parents=True, exist_ok=True)
        index.write_bytes(raw)
        release = {
            "id": id,
            "repository": record["url"],
            "commit": actual,
            "path": str(destination.relative_to(ROOT)),
            "inventory": str(index.relative_to(ROOT)),
            "inventory_sha256": sha(raw),
            "files": sum(e["kind"] != "gitlink" for e in inventory),
            "bytes": sum(int(e.get("bytes", 0)) for e in inventory),
            "missing_dependencies": [
                e["path"] for e in inventory if e["kind"] == "gitlink" and not e["materialized"]
            ],
            "license_evidence": record["license_evidence"],
        }
        releases.append(release)
        print(
            json.dumps(
                {k: release[k] for k in ("id", "path", "files", "bytes", "missing_dependencies")}
            ),
            flush=True,
        )
    if extra_catalog.exists():
        for record in extra_records:
            if args.only and record["id"] not in args.only:
                continue
            source = (
                ROOT / ".peykar/external/extra-sources" / record["repository"].replace("/", "--")
            )
            actual = subprocess.check_output(
                ["git", "-C", str(source), "rev-parse", "HEAD"], text=True
            ).strip()
            if actual != record["commit"]:
                raise ValueError("Additional source pin differs")
            destination = ROOT / "vendor/dependencies" / record["id"]
            inventory = []
            copy_tree(source, destination, inventory)
            raw = (json.dumps(inventory, sort_keys=True, separators=(",", ":")) + "\n").encode()
            index = ROOT / "vendor/inventories" / (record["id"] + "-" + sha(raw) + ".json")
            index.write_bytes(raw)
            evidence = {
                e["path"]: e["sha256"]
                for e in inventory
                if e["kind"] == "file"
                and any(
                    word in Path(str(e["path"])).name.lower()
                    for word in ("license", "licence", "copyright", "copying", "notice")
                )
            }
            release = {
                "id": record["id"],
                "repository": "https://github.com/" + record["repository"],
                "commit": actual,
                "ref": record["ref"],
                "path": str(destination.relative_to(ROOT)),
                "inventory": str(index.relative_to(ROOT)),
                "inventory_sha256": sha(raw),
                "files": sum(e["kind"] != "gitlink" for e in inventory),
                "bytes": sum(int(e.get("bytes", 0)) for e in inventory),
                "missing_dependencies": [
                    e["path"] for e in inventory if e["kind"] == "gitlink" and not e["materialized"]
                ],
                "license_evidence": evidence,
            }
            releases.append(release)
            print(
                json.dumps({k: release[k] for k in ("id", "files", "missing_dependencies")}),
                flush=True,
            )
    if args.only and "vcornea" not in args.only:
        manifest = {
            "format": "peykar.vendor/1",
            "source_policy": SOURCE_POLICY,
            "releases": releases,
        }
        (ROOT / "vendor/manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        return
    archive_source = ROOT / ".peykar/external/vcornea-archive-16884414"
    pins = json.loads(
        (ROOT / "engine_adapters/compucell3d/vcornea-source-16884414.json").read_bytes()
    )
    destination = ROOT / "vendor/models/vcornea-16884414"
    for name, checksum in pins["files"].items():
        data = (archive_source / name).read_bytes()
        if sha(data) != checksum:
            raise ValueError("Original model source changed")
        target = destination / name
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() and sha(target.read_bytes()) != checksum:
            raise ValueError("Vendored model source differs")
        if not target.exists():
            shutil.copy2(archive_source / name, target)
    releases.append(
        {
            "id": "vcornea",
            "path": str(destination.relative_to(ROOT)),
            "archive": pins["doi"],
            "archive_sha256": pins["archive"]["sha256"],
            "files": len(pins["files"]),
            "license": pins["license"],
            "attribution": pins["attribution"],
        }
    )
    manifest = {
        "format": "peykar.vendor/1",
        "source_policy": SOURCE_POLICY,
        "releases": releases,
    }
    (ROOT / "vendor/manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    main()
