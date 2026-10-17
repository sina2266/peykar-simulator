#!/usr/bin/env python3
"""Assemble the universal mac64 release directory. Standard library only.

Two steps, because each architecture's GCC runtime only exists on the runner
that built it:

  package_macos.py bundle <build/bin> <out dir>
      On each architecture's runner: copy the three solvers, then every dylib
      they need that is not part of macOS (libgfortran, libquadmath, libgcc_s
      from Homebrew GCC), recursively, rewriting every reference to
      @loader_path/<name> and dropping absolute LC_RPATHs -- so nothing points
      into /opt/homebrew or /usr/local any more.

  package_macos.py universal <arm64 dir> <x86_64 dir> <out dir> <version> <license>
      On one runner: lipo each pair of Mach-O files into a universal file,
      ad-hoc sign every one of them (install_name_tool invalidates the
      linker's signature, and Apple silicon refuses to run an unsigned
      arm64 binary), write VERSION and LICENSE, and verify the result.
"""
import os
import re
import shutil
import subprocess
import sys

EXES = ["Hybrid_EM_x64", "Hybrid_MIL_x64", "Hybrid_MIL_Adaptive_x64"]
SYSTEM_PREFIXES = ("/usr/lib/", "/System/")


def run(*cmd: str) -> str:
    return subprocess.run(cmd, check=True, capture_output=True, text=True).stdout


def is_macho(path: str) -> bool:
    with open(path, "rb") as f:
        magic = f.read(4)
    return magic in (b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe", b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca")


def dependencies(path: str) -> list[str]:
    """The install names a Mach-O file links against (its own id excluded)."""
    # Header lines end in ':' -- one per file, or one per slice of a universal
    # file ("<path> (architecture arm64):"); everything else is an install name.
    def names(out: str) -> list[str]:
        return [line.strip().split(" (")[0] for line in out.splitlines()
                if line.strip() and not line.rstrip().endswith(":")]
    own_id = set(names(run("otool", "-D", path)))
    return sorted({d for d in names(run("otool", "-L", path)) if d not in own_id})


def rpaths(path: str) -> list[str]:
    out = run("otool", "-l", path)
    return re.findall(r"cmd LC_RPATH\n\s+cmdsize \d+\n\s+path (.+?) \(offset", out)


def resolve(dep: str, origin: str, exe_rpaths: list[str]) -> str:
    """Find the file an install name refers to, as dyld would from `origin`."""
    here = os.path.dirname(origin)
    if dep.startswith("@loader_path/") or dep.startswith("@executable_path/"):
        return os.path.realpath(os.path.join(here, dep.split("/", 1)[1]))
    if dep.startswith("@rpath/"):
        rest = dep[len("@rpath/"):]
        for rp in rpaths(origin) + exe_rpaths:
            rp = rp.replace("@loader_path", here).replace("@executable_path", here)
            candidate = os.path.join(rp, rest)
            if os.path.exists(candidate):
                return os.path.realpath(candidate)
        raise SystemExit(f"cannot resolve {dep} from {origin}")
    return os.path.realpath(dep)


def bundle(bindir: str, out: str) -> None:
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    origin_of: dict[str, str] = {}      # bundled name -> the file it was copied from
    queue = []
    for exe in EXES:
        shutil.copy2(os.path.join(bindir, exe), os.path.join(out, exe))
        origin_of[exe] = os.path.realpath(os.path.join(bindir, exe))
        queue.append(exe)
    exe_rpaths = rpaths(origin_of[EXES[0]])

    while queue:
        name = queue.pop()
        target = os.path.join(out, name)
        os.chmod(target, 0o755)
        for dep in dependencies(target):
            if dep.startswith(SYSTEM_PREFIXES):
                continue
            base = os.path.basename(dep)
            if base not in origin_of:
                src = resolve(dep, origin_of[name], exe_rpaths)
                print(f"bundling {base}  ({src})")
                shutil.copyfile(src, os.path.join(out, base))
                origin_of[base] = src
                queue.append(base)
            run("install_name_tool", "-change", dep, f"@loader_path/{base}", target)
        if name not in EXES:
            run("install_name_tool", "-id", f"@loader_path/{name}", target)
        for rp in rpaths(target):
            if not rp.startswith("@"):
                run("install_name_tool", "-delete_rpath", rp, target)


def universal(arm: str, x86: str, out: str, version: str, license_file: str) -> None:
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    names = sorted(os.listdir(arm))
    if names != sorted(os.listdir(x86)):
        raise SystemExit(f"the two architectures bundle different files:\n  arm64:  {names}\n"
                         f"  x86_64: {sorted(os.listdir(x86))}")
    for name in names:
        a, x, o = (os.path.join(d, name) for d in (arm, x86, out))
        if is_macho(a):
            run("lipo", "-create", a, x, "-output", o)
            os.chmod(o, 0o755)
        else:
            shutil.copy2(a, o)
    # Libraries first, then the executables that load them.
    for name in sorted(names, key=lambda n: n in EXES):
        path = os.path.join(out, name)
        if is_macho(path):
            run("codesign", "--force", "--sign", "-", path)

    with open(os.path.join(out, "VERSION"), "w") as f:
        f.write(version + "\n")
    shutil.copyfile(license_file, os.path.join(out, "LICENSE"))

    # Verify: both slices, only system or @loader_path references, valid signature.
    problems = []
    for name in names:
        path = os.path.join(out, name)
        if not is_macho(path):
            continue
        archs = set(run("lipo", "-archs", path).split())
        if archs != {"arm64", "x86_64"}:
            problems.append(f"{name}: architectures {sorted(archs)}")
        for dep in dependencies(path):
            if not dep.startswith(SYSTEM_PREFIXES + ("@loader_path/",)):
                problems.append(f"{name}: links {dep}")
        for rp in rpaths(path):
            if not rp.startswith("@"):
                problems.append(f"{name}: LC_RPATH {rp}")
        subprocess.run(["codesign", "--verify", "--strict", path], check=True)
        for arch in ("arm64", "x86_64"):
            minos = re.search(r"minos (\S+)", run("otool", "-arch", arch, "-l", path))
            print(f"{name:32s} {arch:7s} minos {minos.group(1) if minos else '?'}")
    if problems:
        raise SystemExit("mac64 bundle is not self-contained:\n  " + "\n  ".join(problems))
    print(f"packaged {version} into {out}: {', '.join(sorted(os.listdir(out)))}")


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "bundle":
        bundle(sys.argv[2], sys.argv[3])
    elif len(sys.argv) == 7 and sys.argv[1] == "universal":
        universal(*sys.argv[2:7])
    else:
        raise SystemExit(__doc__)
