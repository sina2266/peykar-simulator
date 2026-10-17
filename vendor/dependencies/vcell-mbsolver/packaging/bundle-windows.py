"""Package the Windows build as a VCell release archive (win64.zip).

    python packaging/bundle-windows.py <MovingBoundarySolver.exe> <out.zip> <dll-dir> [<dll-dir> ...]

At the archive root: MovingBoundary_x64.exe (the name VCell resolves), every
DLL it transitively imports that is not part of Windows — the vcpkg-built HDF5
and friends, found in the given <dll-dir>s, and the MSVC C++ runtime
(msvcp140*/vcruntime140*, app-local as Microsoft permits) — plus LICENSE and
VERSION. Needs `pefile` (pip install pefile).
"""

from __future__ import annotations

import re
import shutil
import sys
import tempfile
import zipfile
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parent.parent
# The MSVC runtime is redistributable app-locally; take it from System32, where the
# VC++ redistributable on the build machine installed it.
MSVC_RUNTIME = re.compile(r"^(msvcp140.*|vcruntime140.*|concrt140)\.dll$", re.IGNORECASE)
SYSTEM32 = Path(r"C:\Windows\System32")


def imports(path: Path) -> list[str]:
    pe = pefile.PE(str(path), fast_load=True)
    try:
        pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
        return [entry.dll.decode() for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", [])]
    finally:
        pe.close()  # release the file mapping, or Windows cannot delete the staging dir


def main(argv: list[str]) -> int:
    exe, out, *dll_dirs = (Path(a) for a in argv[1:])
    version = re.search(r"project\(VCellMovingBoundary VERSION ([0-9.]+)", (ROOT / "CMakeLists.txt").read_text())
    assert version, "no project() version in CMakeLists.txt"

    with tempfile.TemporaryDirectory() as tmp:
        stage = Path(tmp)
        shutil.copy2(exe, stage / "MovingBoundary_x64.exe")
        shutil.copy2(ROOT / "LICENSE", stage / "LICENSE")
        (stage / "VERSION").write_text(version.group(1) + "\n")

        todo, seen = [stage / "MovingBoundary_x64.exe"], set()
        while todo:
            for dll in imports(todo.pop()):
                key = dll.lower()
                if key in seen:
                    continue
                seen.add(key)
                found = next((d / dll for d in dll_dirs if (d / dll).is_file()), None)
                if found is None and MSVC_RUNTIME.match(dll) and (SYSTEM32 / dll).is_file():
                    found = SYSTEM32 / dll
                if found is None:
                    print(f"  system: {dll}")
                    continue
                print(f"  bundle: {dll} <- {found}")
                shutil.copy2(found, stage / dll)
                todo.append(stage / dll)

        out.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zf:
            for f in sorted(stage.iterdir()):
                zf.write(f, f.name)
        print(f"packaged {out}: {sorted(f.name for f in stage.iterdir())}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
