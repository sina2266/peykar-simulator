"""Retain package-provided notices before removing the installation download cache."""

import json
import shutil
from pathlib import Path

cache = Path("/opt/conda/pkgs")
destination = Path("/opt/conda/cc3d-runtime/notices")
destination.mkdir(parents=True, exist_ok=True)
inventory = []
for package in sorted(cache.iterdir()):
    if not package.is_dir() or not (package / "info/index.json").is_file():
        continue
    metadata = json.loads((package / "info/index.json").read_text())
    notices = package / "info/licenses"
    if notices.is_dir():
        shutil.copytree(notices, destination / package.name, dirs_exist_ok=True)
    inventory.append(
        {
            "package": package.name,
            "license": metadata.get("license"),
            "provided_notices_copied": notices.is_dir(),
        }
    )
(destination / "inventory.json").write_text(json.dumps(inventory, indent=2) + "\n")
