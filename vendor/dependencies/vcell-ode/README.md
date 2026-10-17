<p align="center" width="100%">
 <a href="https://vcell.org">
    <img width="10%" src="https://github.com/biosimulations/biosimulations/blob/dev/docs/src/assets/images/about/partners/vcell.svg">
 </a>
</p>

## The Virtual Cell Project
The Virtual Cell is a modeling and simulation framework for computational biology.  For details see http://vcell.org and http://github.com/virtualcell.

---
# vcell-ode
![CI](https://github.com/virtualcell/vcell-ode/actions/workflows/cd.yml/badge.svg)

Virtual Cell ODE [virtualcell/vcell-ode](https://github.com/virtualcell/vcell-ode) is a collection of numerical 
simulation libraries and protocols used to process ODEs in the Virtual Cell framework [virtualcell/vcell](https://github.com/virtualcell/vcell)).

## Building VCell ODE with Conan on Windows

Windows builds require a native compiler. MinGW is not supported. Install Visual Studio
Build Tools with the C++ workload and Windows SDK, plus Python 3.10 or newer. Conan 2,
CMake 3.16 or newer, and Ninja 1.12 or newer are required; Conan installs CMake and
Ninja as build requirements, so they do not need to be installed separately.

From a Visual Studio Developer PowerShell, install Conan and create a detected host
profile:

```powershell
py -m pip install --upgrade conan
$pythonScripts = py -c "import sysconfig; print(sysconfig.get_path('scripts'))"
$env:Path = "$pythonScripts;$env:Path"
conan profile detect --force
```

If PowerShell still cannot find `conan`, run the following once, then open a new
PowerShell window:

```powershell
$pythonScripts = py -c "import sysconfig; print(sysconfig.get_path('scripts'))"
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
[Environment]::SetEnvironmentVariable('Path', "$userPath;$pythonScripts", 'User')
```

Build the solver without messaging:

```powershell
conan install . --build=missing -o "&:include_messaging=False" -s compiler.cppstd=20
conan build . -s compiler.cppstd=20
```

The recipe invokes CMake and Ninja, and produces the executable and DLL under
`build/bin`. To use the checked-in LLVM/Clang-CL profile instead, replace the install
command with:

```powershell
conan install . `
  --profile:host conan-profiles/CI-CD/Windows-AMD64_profile.txt `
  --profile:build default `
  --build=missing -o "&:include_messaging=False" -s:h compiler.cppstd=20
conan build . -s compiler.cppstd=20
```

The checked-in profile requires LLVM/Clang 21 (`clang-cl`) on `PATH` and Visual Studio
2022 Build Tools with the v143 toolset. The auto-detected MSVC profile is recommended
when using the compiler supplied by Visual Studio. The checked-in profile is configured
for Visual Studio 2022; it does not target Visual Studio 18.

Messaging is disabled automatically for Windows Conan builds. It remains enabled by
default on Linux and macOS, where the libcurl dependency is supported. The
`include_messaging` option can be used to disable it on those platforms when needed.

## Release contract (SOLVER-RELEASE)

What VCell consumes from this repo, per VCell's solver-repo plan (`docs/plan-solver-repos.md` in
[virtualcell/vcell](https://github.com/virtualcell/vcell), §1). Executable: **`SundialsSolverStandalone_x64`**
(`.exe` on Windows), run as `SundialsSolverStandalone_x64 <input.cvodeInput|input.idaInput> <output.ida> [-tid <n>]`.
The solver (CVODE or IDA) is chosen by the input's `SOLVER` line.

**Releases** are cut from `master`: bump `version` in `pyproject.toml`, merge, then publish a GitHub
release with tag `vX.Y.Z` (it must equal the `pyproject.toml` version, or the build fails). `cd.yml` then
attaches

| asset | contents |
|---|---|
| `linux64.tgz` | x86_64, built on manylinux_2_28 (runs on glibc ≥ 2.28); glibc not bundled, other libraries (libc++, …) bundled with a `$ORIGIN` rpath |
| `linux64arm.tgz` | aarch64, same |
| `mac64.tgz` | universal (x86_64 + arm64) executable and dylibs, `@loader_path` references only, ad-hoc signed |
| `win64.zip` | x86_64 `.exe` and DLLs |
| `win64arm.zip` | arm64 `.exe` and DLLs |
| `SHA256SUMS` | sha256 of each asset above |

Each archive is flat: the executable, its bundled libraries, `LICENSE` and `VERSION` — no test binaries or
static libraries — and is checked on a clean runner (and on Rocky Linux 8 for `linux64.tgz`) against the
reference outputs before anything is attached. The release also publishes the Python wheels to PyPI.

The archive builds have messaging OFF (the desktop client reads progress from stdout); they still accept
`-tid <n>` so VCell's command line is the same everywhere.

**Container** (`container.yml`, `Dockerfile`): `ghcr.io/virtualcell/vcell-ode:<X.Y.Z>` (and `:latest`),
linux/amd64 + linux/arm64, a slim `debian:trixie-slim` runtime stage, built with **messaging ON** (the input's
`JMS_PARAM` block plus `-tid` report status to VCell's broker). The amd64 image is also published as a SIF,
`oras://ghcr.io/virtualcell/vcell-ode_singularity:<X.Y.Z>`. Pushes to `master` publish `:master` and `:sha-<short>`.

**Entrypoint** `/usr/local/bin/vcell-solver-entrypoint` (`docker/entrypoint.sh`):
no argument or `--help` prints the version and executables and exits 0; `SundialsSolverStandalone_x64 …` is
`exec`'d (exit code and signals pass through); anything else prints usage and exits 2. It writes nothing,
runs as any uid, and works from a read-only SIF, e.g. as VCell's SlurmProxy runs it:

```bash
singularity run --containall --bind /path/to/simdata:/simdata vcell-ode_singularity_<X.Y.Z>.sif \
    SundialsSolverStandalone_x64 /simdata/SimID_1_0_.cvodeInput /simdata/SimID_1_0_.ida -tid 0
```

**Reference outputs** (`tests/reference/`): two CVODE inputs (one with discontinuities and events) and one IDA
DAE input, with the output of the legacy `vcell-solvers` v0.0.44-dev4 Linux binary. `compare_ida.py` checks a
run against them (atol 1e-8 × column scale; rtol 1e-5 row by row where the time grids coincide, 1e-4 when the
adaptive steps differ and the run is interpolated onto the reference's time points). CI runs them against every archive, the Docker image (non-root, read-only root,
`-tid`) and the SIF (`apptainer run --containall`, bind-mounted `/simdata`, `-tid`).
