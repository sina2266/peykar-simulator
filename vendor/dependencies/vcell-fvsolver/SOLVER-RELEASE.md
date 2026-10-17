# Releases of vcell-fvsolver (the VCell solver release contract)

This repository meets the release contract every VCell solver repository follows (VCell's
`docs/plan-solver-repos.md`, section 1). What VCell consumes, and where it comes from:

## Cutting a release

1. Bump `version` in `pyproject.toml` to `X.Y.Z` on `main` (the Python wheels publish to PyPI under it).
2. Publish a GitHub release whose tag is `vX.Y.Z`, targeting `main`
   (`gh release create vX.Y.Z --target main --title ... --notes ...`).
3. `cd.yml` runs on the published release: it builds every platform, runs the smoke tests, attaches the
   assets below to the release, and (through `container.yml`) pushes the image and the SIF. The wheels go to
   PyPI as before.

Pull requests run the same builds and smoke tests without publishing anything (the assembled assets are kept
as the `release-assets` workflow artifact).

## Release assets

| asset | contents |
|---|---|
| `linux64.tgz` | x86_64, built on `manylinux_2_28` (gcc-toolset-14, glibc 2.28): runs on any Linux with glibc >= 2.28 |
| `linux64arm.tgz` | aarch64, the same build |
| `mac64.tgz` | universal (x86_64 + arm64) Mach-O, dylibs through `@executable_path`/`@loader_path`, ad-hoc signed |
| `win64.zip` | the `.exe` files and any DLLs they need |
| `SHA256SUMS` | `sha256sum` of the four archives |

Every archive is flat, with `FiniteVolume_x64` and `smoldyn_x64` (`.exe` on Windows) at the root, the
non-system shared libraries they need, `LICENSE` and `VERSION` (`X.Y.Z`). No test binaries, no static
libraries, and no glibc: the Linux executables are linked with `-static-libstdc++ -static-libgcc` and
every other dependency is static (HDF5, libzip, zlib, libcurl), so they need only the host's glibc.

Download URLs: `https://github.com/virtualcell/vcell-fvsolver/releases/download/vX.Y.Z/<asset>`.

## Container image and SIF

- `ghcr.io/virtualcell/vcell-fvsolver:X.Y.Z` and `:latest` — one multi-arch image (linux/amd64, linux/arm64).
  Ubuntu 24.04 with the `linux64` archive's contents in `/opt/vcell-fvsolver` on `PATH`.
- `oras://ghcr.io/virtualcell/vcell-fvsolver_singularity:X.Y.Z` and `:latest` — the amd64 image as an
  Apptainer/Singularity SIF, for VCell's Slurm cluster.

The Linux build (archive and image alike) has **VCell messaging on**: `FiniteVolume_x64` accepts the trailing
`-tid <n>` that SlurmProxy appends and posts its status (`JOB_STARTING` ... `JOB_COMPLETED`) to the broker the
`.fvinput`'s `JMS_PARAM` block names. Without `-tid` it reports on stdout, as before. The macOS and Windows
builds keep messaging off (desktop runs never pass `-tid`); `smoldyn_x64` accepts `-tid` on every platform.

### Entrypoint

`/usr/local/bin/vcell-solver-entrypoint` (`docker/entrypoint.sh`; `ENTRYPOINT [...]`, `CMD ["--help"]`):

- no argument or `--help`: prints the version and the executables (`FiniteVolume_x64`, `smoldyn_x64`); exit 0;
- first argument `FiniteVolume_x64` or `smoldyn_x64`: `exec`s it with the remaining arguments unchanged, so
  the exit code and SIGTERM reach the solver directly;
- anything else: usage on stderr, exit 2.

It writes nothing itself and runs as any uid from a read-only SIF, with argv exactly as SlurmProxy writes it:

```
singularity run --containall --bind /share/.../users:/simdata ... <sif> \
    FiniteVolume_x64 /simdata/<user>/SimID_<key>_0_.fvinput -tid 0
```

The solvers write their results next to the input; FiniteVolume stages each `.sim` file in `$TMPDIR`
(default `/tmp`) before adding it to the zip, so pass `--env TMPDIR=<a bound scratch dir>` for large runs.

## Smoke test (CI, every pull request and release)

`docker/smoke/run-smoke.sh` runs, for each of the image (`docker run --read-only`, non-root uid), the SIF
(`apptainer run --containall`, non-root uid) and the bare `linux64` archive on AlmaLinux 8 (glibc 2.28):

- `--help` exits 0, an unknown command exits 2;
- `FiniteVolume_x64 /simdata/fv/SimID_1585623750_0_.fvinput -tid 0` against a stand-in broker
  (`docker/smoke/broker.py`): the output must match `VCell/tests/smoke/SimID_1585623750_0_00.zip.expected`
  element-wise (relative 1e-6) — that file is identical to the 0.9.7 image's output — and the broker must
  receive `JOB_STARTING` and `JOB_COMPLETED` for task 0;
- `FiniteVolume_x64 /simdata/fv3d/SimID_11538992_0_.fvinput` (the 3-D FV-solver fixture, two membranes):
  sum, sum of squares, min and max of every variable at every time point must match
  `docker/smoke/reference/fv3d-summary.json` (0.9.7's output) within 1e-6 relative;
- `smoldyn_x64 /simdata/smoldyn/input.smoldynInput -tid 0`: per-variable totals must match
  `docker/smoke/reference/smoldyn-summary.json` (0.9.7's output) in their time mean (within 25%), and the
  molecule total (A + B = 5) must be conserved at every one of the 201 time points. (Smoldyn's own Mersenne
  twister makes a run deterministic for a given binary; today every build reproduces 0.9.7's trajectory
  exactly, but the check is statistical so that a compiler change does not break it.)

The macOS universal binaries run the same three comparisons natively (arm64) in `cd.yml`.

All builds are compiled with `-ffp-contract=off` (top-level `CMakeLists.txt`): compilers fuse
multiply-adds by default on arm64, which moved the 3-D fixture's membrane areas by 5e-5 and its
concentrations by up to 2% against the x86_64 builds and 0.9.7. Without fusion the arm64 builds match
0.9.7 bit for bit.
