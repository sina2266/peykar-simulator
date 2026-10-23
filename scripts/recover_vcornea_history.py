"""Recover the reviewed historical snapshot from the pinned licensed code archive.

Only Git objects are copied into a fresh trusted object database. Downloaded Git
configuration, hooks, refs and alternates are never loaded. No model code executes.
"""

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import tempfile
import zipfile
from pathlib import Path, PurePosixPath

from peykar.integrations.vcornea_history import HISTORICAL_COMMIT, HISTORICAL_TREE
from peykar.integrations.vcornea_runtime import ARCHIVE_SHA256, _file_inventory
from peykar_core.hashing import canonical_json, digest


def recover(archive: Path, pins: Path, output: Path) -> None:
    if output.exists():
        raise ValueError("Preserve existing source evidence; choose a fresh destination")
    sha = hashlib.sha256()
    with archive.open("rb") as stream:
        while chunk := stream.read(1024 * 1024):
            sha.update(chunk)
    if sha.hexdigest() != ARCHIVE_SHA256:
        raise ValueError("Code ZIP differs from the pinned licensed archive")
    evidence = json.loads(pins.read_bytes())
    if (
        evidence["git_commit"] != HISTORICAL_COMMIT
        or evidence["original_archive_sha256"] != ARCHIVE_SHA256
        or digest(evidence["files"]) != HISTORICAL_TREE
    ):
        raise ValueError("Historical source pins differ")
    with tempfile.TemporaryDirectory(prefix="peykar-vcornea-history-") as temporary:
        root = Path(temporary)
        environment = {
            "PATH": os.environ["PATH"],
            "HOME": temporary,
            "GIT_CONFIG_NOSYSTEM": "1",
            "GIT_CONFIG_GLOBAL": os.devnull,
            "GIT_TERMINAL_PROMPT": "0",
        }
        database = root / "objects.git"

        def git(*args):
            return subprocess.check_output(
                ["git", "--git-dir=" + str(database), *args],
                env=environment,
                cwd=root,
                stderr=subprocess.PIPE,
                timeout=60,
            )

        git("init", "--bare", "--template=", str(database))
        prefix = "vCornea/.git/objects/"
        with zipfile.ZipFile(archive) as source:
            for info in source.infolist():
                if not info.filename.startswith(prefix) or info.is_dir():
                    continue
                relative = info.filename[len(prefix) :]
                if not re.fullmatch(
                    r"(?:[0-9a-f]{2}/[0-9a-f]{38}|pack/pack-[0-9a-f]{40}\.(pack|idx|rev))",
                    relative,
                ):
                    continue
                destination = database / "objects" / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                with source.open(info) as original, destination.open("xb") as target:
                    shutil.copyfileobj(original, target)
        entries = git("ls-tree", "-rz", "--full-tree", HISTORICAL_COMMIT).split(b"\0")
        recovered = root / "snapshot"
        recovered.mkdir()
        seen = set()
        for entry in entries:
            if not entry:
                continue
            header, name = entry.split(b"\t", 1)
            mode, kind, blob = header.decode().split()
            relative = name.decode("utf-8")
            path = PurePosixPath(relative)
            if (
                kind != "blob"
                or mode not in ("100644", "100755")
                or path.is_absolute()
                or any(p in ("..", ".git") for p in path.parts)
                or relative not in evidence["files"]
                or relative in seen
                or evidence["git_blob_ids"][relative] != blob
            ):
                raise ValueError("Historical Git tree differs from reviewed regular blobs")
            data = git("cat-file", "blob", blob)
            if hashlib.sha256(data).hexdigest() != evidence["files"][relative]:
                raise ValueError("Historical blob checksum differs")
            target = recovered / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            target.chmod(0o755 if mode == "100755" else 0o644)
            seen.add(relative)
        if _file_inventory(recovered) != evidence["files"]:
            raise ValueError("Incomplete historical snapshot")
        shutil.copytree(recovered, output)
    receipt = {
        "schema_version": "1.0.0",
        "archive_sha256": ARCHIVE_SHA256,
        "git_commit": HISTORICAL_COMMIT,
        "source_tree_digest": HISTORICAL_TREE,
        "files": len(seen),
        "model_code_executed": False,
    }
    output.with_name(output.name + "-recovery.json").write_text(canonical_json(receipt) + "\n")
    print("Verified historical snapshot:", len(seen), "original files")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", required=True, type=Path)
    parser.add_argument(
        "--pins",
        type=Path,
        default=Path("engine_adapters/compucell3d/vcornea-history-b6123adf.json"),
    )
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    recover(args.archive, args.pins, args.output)
