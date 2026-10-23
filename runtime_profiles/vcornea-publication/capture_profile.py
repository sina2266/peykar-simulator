"""Capture an explicitly installed local CC3D runtime; never install or execute it."""

import argparse
import ast
import hashlib
import json
from pathlib import Path


def capture(prefix, destination):
    if destination.exists():
        raise ValueError("Preserve previous runtime profiles; choose a new path")
    metadata = [json.loads(p.read_bytes()) for p in sorted((prefix / "conda-meta").glob("*.json"))]
    python = next(p for p in metadata if p["name"] == "python")
    cc3d = next(p for p in metadata if p["name"] == "cc3d")
    if cc3d["subdir"] != "osx-arm64" or cc3d["version"] not in ("4.6.0", "4.7.0"):
        raise ValueError("Only reviewed macOS ARM64 CC3D builds are supported")
    minor = ".".join(python["version"].split(".")[:2])
    python_binary = f"bin/python{minor}"
    installed = prefix / f"lib/python{minor}/site-packages/cc3d"
    values = {}
    for node in ast.parse((installed / "__init__.py").read_bytes()).body:
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id in ("__version__", "__githash__"):
                    values[target.id] = ast.literal_eval(node.value)
    if values["__version__"] != cc3d["version"]:
        raise ValueError("Installed CC3D Python version differs from its package metadata")

    def sha(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()

    profile = {
        "platform": "macos-arm64",
        "cc3d_version": cc3d["version"],
        "cc3d_build_label": values["__githash__"],
        "python_binary": python_binary,
        "python_digest": sha(prefix / python_binary),
        "packages": [
            {
                key: p.get(key) if key == "license" else p[key]
                for key in ("name", "version", "build", "subdir", "url", "sha256", "md5", "license")
            }
            for p in metadata
        ],
        "installed_cc3d_hashes": {
            p.relative_to(prefix).as_posix(): sha(p)
            for p in sorted(installed.rglob("*"))
            if p.is_file() and "__pycache__" not in p.parts
        },
    }
    destination.write_text(json.dumps(profile, indent=2) + "\n")
    print(cc3d["version"], profile["cc3d_build_label"], len(metadata), "pinned package identities")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prefix", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    capture(args.prefix, args.output)
