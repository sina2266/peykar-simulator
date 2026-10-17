# vcell-nfsim — VCell solver release

How this repository meets VCell's solver-release contract (VCell `docs/plan-solver-repos.md` §1), and
what VCell consumes from it. Everything below is produced by `.github/workflows/release.yml`.

## What a release carries

A tag `vX.Y.Z` on `main` (it must equal `version` in `pyproject.toml`) creates the GitHub release with:

| asset | contents |
|---|---|
| `linux64.tgz` | x86_64 `NFsim_x64`, built on manylinux_2_28 (needs glibc ≥ 2.28, GLIBCXX ≥ 3.4.25) |
| `linux64arm.tgz` | the same for aarch64 |
| `mac64.tgz` | universal (arm64 + x86_64) `NFsim_x64`, macOS ≥ 10.15 (arm64 ≥ 11), ad-hoc signed |
| `win64.zip` | `NFsim_x64.exe`, MinGW-w64 gcc linked fully static (no DLLs beyond Windows' own) |
| `pyvcell_nfsim-X.Y.Z-*.whl`, `pyvcell_nfsim-X.Y.Z.tar.gz` | the Python package (unchanged) |
| `SHA256SUMS` | `sha256sum` of every asset above |

Each archive holds, at its root: `NFsim_x64` (`.exe` on Windows), `LICENSE` (this repository's MIT
license), `LICENSE-GPL-3.0.txt` (NFsim's own license) and `VERSION` (`X.Y.Z`). NFsim needs no shared
library beyond the platform's C/C++ runtime, so there are no bundled libraries.

Images, pushed by the same workflow (and on every push to `main` as `:latest` and `:sha-<short>`):

- `ghcr.io/virtualcell/vcell-nfsim:X.Y.Z` — linux/amd64 + linux/arm64; a `debian:bookworm-slim` base with
  the archive's files in `/opt/vcell-nfsim` (on `PATH`).
- `oras://ghcr.io/virtualcell/vcell-nfsim_singularity:X.Y.Z` — the amd64 image as an Apptainer SIF.

## `NFsim_x64`

`NFsim_x64` is NFsim with VCell's `main` (`VCell/src/VCellNFSim.cpp`, the same glue the legacy
`vcell-solvers` build used). VCell runs it as

    NFsim_x64 -seed <n> -vcell -xml <SimID>.nfsimInput -o <SimID>.gdat -ss <SimID>.species \
              -sim <t> -oSteps <n> [-notf -utl <n> -cb -pcmatch ...] [-tid <n>]

and it prints `[[[progress:…%]]]` markers on stdout, which the desktop client reads. The plain `NFsim`
target (BioNetGen's model tests, the Python binding) is unchanged.

**Messaging.** The Linux build (archives and image) is built with VCell messaging ON
(`-DOPTION_VCELL_MESSAGING=ON`): a trailing `-tid <n>` makes it post its status (JOB_STARTING, progress,
JOB_COMPLETED / JOB_FAILURE) to the ActiveMQ REST endpoint named in the `<jms>` element that VCell writes
into the `.nfsimInput`, as the legacy v0.8.2 image did. libcurl is linked statically as a minimal
HTTP-only build (`packaging/build-linux.sh`), so the Linux archive still has no extra dependencies.
Without `-tid` it behaves exactly as the desktop binary. The macOS and Windows builds are messaging-OFF
(desktop only; `-tid` is accepted and ignored), like the legacy desktop binaries.

## Container entry point

`/usr/local/bin/vcell-solver-entrypoint` (`docker/entrypoint.sh`), `ENTRYPOINT [...]`, `CMD ["--help"]`:

- no argument, `--help` or `-h`: print the version and the provided executables (`NFsim_x64`), exit 0;
- `NFsim_x64 <args>`: `exec` it, so the exit code and SIGTERM pass straight through;
- anything else: usage on stderr, exit 2.

It writes nothing itself, runs as any uid, and works from the read-only SIF under
`singularity run --containall --bind <dir>:/simdata <sif> NFsim_x64 ... -tid <n>` — argv exactly as VCell's
SlurmProxy writes it.

## Checks (every pull request, push and tag)

- **Smoke test** (`tests/container/smoke.sh`): the committed reference model
  (`tests/smoke/SimID_273069657_0_.*`, fixed seed) through the Docker image as uid 12345 with a bind mount,
  and through the SIF under `apptainer run --containall` as the (non-root) runner user; both with
  `-tid 0` and the input's broker pointed at a local fake broker. The outputs must equal the committed
  reference byte for byte, the broker must receive JOB_STARTING and JOB_COMPLETED for SimKey/TaskID, and
  `--help` must exit 0 (an unknown executable, 2).
- **Archives**: each platform's `NFsim_x64` reproduces the reference; the Linux one also runs on
  AlmaLinux 8 (glibc 2.28); the dependency checks reject any non-system library.
- **Equivalence with the legacy binary** (`tests/equivalence/compare_legacy.sh`): the reference model with
  five seeds, 5 s and 50 output steps, through `vcell-solvers` v0.0.44-dev4 `NFsim_x64` and the new one —
  byte-identical `.gdat` and `.species` on linux64 and on both mac64 slices (NFsim is a seeded Mersenne
  twister; the source is the same NFsim 1.12.1). The results go into the release notes.

## Cutting a release

1. Bump `version` in `pyproject.toml` on a branch; merge the PR (all checks green, merge commit).
2. `git tag vX.Y.Z origin/main && git push origin vX.Y.Z` — the workflow builds, checks, publishes the
   image and SIF, and creates the release. Existing releases and tags are never rewritten.

The standalone `publish-python-package.yml` (wheels only, by hand) still works as before.
