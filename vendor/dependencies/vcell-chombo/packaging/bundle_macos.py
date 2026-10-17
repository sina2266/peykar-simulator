#!/usr/bin/env python3
"""Make a staged macOS release self-contained (SOLVER-RELEASE.md).

    bundle_macos.py <stage-dir>

The solvers are built with Homebrew GCC, so besides libSystem they link the GCC
runtime dylibs from /opt/homebrew (or /usr/local) -- libgfortran, libquadmath,
libstdc++, libgcc_s -- which users do not have. This copies the whole non-system
dependency closure next to the executables, gives each copied dylib the install
name @rpath/<name>, rewrites every reference to match, and leaves exactly one
LC_RPATH, @loader_path, on every Mach-O file. The archive then works from any
directory it is unpacked into, and nothing refers to a Homebrew path.

Everything is re-signed ad hoc at the end: install_name_tool invalidates the
signature the linker made, and arm64 refuses to run unsigned code.
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SYSTEM = ("/usr/lib/", "/System/")
EXES = ("VCellChombo2D_x64", "VCellChombo3D_x64")


def run(*cmd: str) -> str:
    return subprocess.run(cmd, check=True, capture_output=True, text=True).stdout


def deps(f: Path) -> list[str]:
    lines = run("otool", "-L", str(f)).splitlines()[1:]
    out = [ln.strip().split(" (")[0] for ln in lines if ln.strip()]
    if f.suffix == ".dylib" and out:
        own = run("otool", "-D", str(f)).splitlines()[-1].strip()
        out = [d for d in out if d != own]
    return out


def rpaths(f: Path) -> list[str]:
    text = run("otool", "-l", str(f))
    return re.findall(r"cmd LC_RPATH\n\s+cmdsize \d+\n\s+path (.+?) \(offset", text)


def resolve(ref: str, loader_origin: Path, loader_rpaths: list[str]) -> Path:
    def expand(p: str) -> str:
        return p.replace("@loader_path", str(loader_origin)).replace("@executable_path", str(loader_origin))

    if ref.startswith("@rpath/"):
        tail = ref[len("@rpath/"):]
        for rp in loader_rpaths:
            cand = Path(expand(rp)) / tail
            if cand.exists():
                return cand.resolve()
        # Homebrew GCC's own libraries refer to each other through @rpath and
        # rely on the executable's rpath; fall back to the loader's directory.
        cand = loader_origin / tail
        if cand.exists():
            return cand.resolve()
        raise SystemExit(f"cannot resolve {ref} from {loader_origin} (rpaths {loader_rpaths})")
    if ref.startswith(("@loader_path/", "@executable_path/")):
        return Path(expand(ref)).resolve()
    return Path(ref).resolve()


def main() -> None:
    stage = Path(sys.argv[1]).resolve()
    # Where each staged file originally came from: @loader_path in its load
    # commands means *that* directory, not the stage.
    origin: dict[Path, Path] = {stage / e: stage for e in EXES}
    queue = [stage / e for e in EXES]
    seen: set[Path] = set()

    while queue:
        f = queue.pop()
        if f in seen:
            continue
        seen.add(f)
        orig_dir = origin[f]
        rps = rpaths(f)
        for ref in deps(f):
            if ref.startswith(SYSTEM):
                continue
            src = resolve(ref, orig_dir, rps)
            name = Path(ref).name
            dst = stage / name
            if not dst.exists():
                shutil.copy2(src, dst)
                os.chmod(dst, 0o755)
                run("install_name_tool", "-id", f"@rpath/{name}", str(dst))
                origin[dst] = src.parent
                queue.append(dst)
                print(f"  bundled {name} (from {src})")
            if ref != f"@rpath/{name}":
                run("install_name_tool", "-change", ref, f"@rpath/{name}", str(f))
        for rp in rps:
            if rp != "@loader_path":
                run("install_name_tool", "-delete_rpath", rp, str(f))
        if "@loader_path" not in rps:
            run("install_name_tool", "-add_rpath", "@loader_path", str(f))

    # Verify and sign.
    problems = []
    for f in sorted(seen):
        for ref in deps(f):
            if not (ref.startswith(SYSTEM) or ref.startswith("@rpath/")):
                problems.append(f"{f.name} -> {ref}")
            elif ref.startswith("@rpath/") and not (stage / ref[len("@rpath/"):]).exists():
                problems.append(f"{f.name} -> {ref} (not in the archive)")
        if rpaths(f) != ["@loader_path"]:
            problems.append(f"{f.name} rpaths {rpaths(f)}")
        run("codesign", "--force", "--sign", "-", str(f))
        minos = re.findall(r"minos (\S+)", run("otool", "-l", str(f)))
        print(f"  {f.name}: minos {','.join(sorted(set(minos))) or '?'}")
    if problems:
        raise SystemExit("bundle_macos.py:\n  " + "\n  ".join(problems))


if __name__ == "__main__":
    main()
