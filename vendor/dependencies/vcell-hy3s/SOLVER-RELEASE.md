# Releases: the VCell solver-repo contract, as vcell-hy3s meets it

VCell consumes this repository's releases in two places: the desktop client
downloads the archives into `localsolvers/{linux64,mac64,win64}/`, and the
cluster runs the SIF. Both follow the contract every split solver repo meets
([VCell `docs/plan-solver-repos.md`](https://github.com/virtualcell/vcell/blob/master/docs/plan-solver-repos.md),
section 1). This page records how it is met here, and the decisions that were
this repo's to make.

## Cutting a release

Merge to `main`, then tag `main`:

```bash
git tag -a v1.0.1 -m "vcell-hy3s 1.0.1" origin/main && git push origin v1.0.1
```

`.github/workflows/release.yml` does the rest. It refuses a tag that is not
`vX.Y.Z` or not on `main`. Pull requests and pushes to `main` run the same
workflow — every build and every test below — and publish nothing.

## What a release contains

| artifact | contents |
| --- | --- |
| `linux64.tgz` | x86_64, built on manylinux_2_28: needs glibc ≥ 2.28 and nothing else |
| `linux64arm.tgz` | aarch64, the same |
| `mac64.tgz` | universal (arm64 + x86_64), ad-hoc signed |
| `win64.zip` | x86_64, Intel Fortran + MSVC, static runtimes |
| `SHA256SUMS` | a checksum for each of the four |
| `ghcr.io/virtualcell/vcell-hy3s:<X.Y.Z>`, `:latest` | amd64 + arm64 image |
| `oras://ghcr.io/virtualcell/vcell-hy3s_singularity:<X.Y.Z>`, `:latest` | amd64 SIF of that image |

Every archive has, at its root and nothing else:

- `Hybrid_EM_x64`, `Hybrid_MIL_x64`, `Hybrid_MIL_Adaptive_x64` (`.exe` on
  Windows) — VCell's names;
- the shared libraries they need that the OS does not provide;
- `LICENSE` — this repository's MIT licence followed by the third-party notices
  (`packaging/THIRD-PARTY-NOTICES.txt`: NetCDF, libcurl, the GCC and Intel
  runtimes);
- `VERSION` — `X.Y.Z`.

| platform | bundled libraries | how they are found |
| --- | --- | --- |
| Linux | `libstdc++.so.6` — whatever `ldd` finds outside glibc. libgfortran (with libquadmath on x86_64) is GCC 14's, linked statically, because the build image's shared one is GCC 8's, whose `RANDOM_NUMBER` is a different generator | `$ORIGIN` RUNPATH on every file; glibc (including `libmvec`) and `libgcc_s` come from the host. `packaging/package-linux.sh` fails the build if anything resolves outside the directory or needs a glibc symbol newer than 2.28. |
| macOS | `libgfortran.5.dylib`, `libquadmath.0.dylib`, `libgcc_s.1.1.dylib` (Homebrew GCC 13) | every reference rewritten to `@loader_path/`, absolute `LC_RPATH`s deleted; `packaging/package_macos.py` fails if any non-system reference remains |
| Windows | none | the MSVC and Intel Fortran runtimes are linked statically; `packaging/package-windows.ps1` walks every import and fails on a dynamic MSVC runtime |

## The container

`docker/Dockerfile` has one build stage and two outputs, so the image cannot
drift from the archive: `--target archive` *is* the `linux64` directory, and the
default `runtime` target copies that same directory to `/opt/vcell-hy3s` on top
of `debian:bookworm-slim` and puts it on `PATH`. No compiler, no build tree.

The entrypoint is `/usr/local/bin/vcell-solver-entrypoint`
(`docker/entrypoint.sh`, POSIX `sh`):

| invocation | behaviour |
| --- | --- |
| no arguments, `--help`, `-h` | prints `vcell-hy3s <version>`, the three executables and their usage; exit 0 |
| `Hybrid_EM_x64 …` (or either other executable) | `exec`s it, so its exit code and signals pass through |
| anything else | usage on stderr; exit 2 |

It writes nothing; the solver writes only the `.nc` file it is given (Hy3S
stores its solution in its input). So it runs from a read-only SIF as any uid,
with argv exactly as SlurmProxy writes it:

```
singularity run --containall <binds> <env> <sif> Hybrid_EM_x64 /simdata/<user>/SimID_<key>_0_.nc 100.0 10.0 0.01 0.1 -OV -tid 0
```

## Messaging

**The Linux archives and the image are built with messaging on**; macOS and
Windows are built without it.

- With `-tid <n>`, a messaging build reads `JMS_BROKER`, `VCELL_USER`,
  `SIMULATION_KEY` and `JOB_INDEX` from the `.nc` file and POSTs progress and
  completion to `http://<JMS_BROKER>/api/message/workerEvent?…`
  (`vcell-messaging`). Without `-tid` it prints `[[[progress:…]]]` to stdout, as
  the desktop expects. With `-tid` and no reachable broker the run still
  completes (curl errors go to stderr).
- libcurl is built in `docker/Dockerfile` from its release tarball (pinned by
  checksum) as a static, HTTP-only library — the REST bridge is plain HTTP —
  so messaging adds no library to the archive and no OpenSSL or zlib to the
  build. (Conan's Linux libcurl recipe goes through autotools, whose m4 does
  not run on manylinux.) That is what lets one Linux build serve both the
  desktop archive and the image.
- The desktop never passes `-tid`, and the Windows build cannot carry
  messaging (see README), so the macOS build leaves it out too: no curl, no
  C++ runtime to bundle.

**VCell side:** `cbit.vcell.solver.stoch.NetCDFWriter` still writes
`JMS_BROKER` as `failover:(tcp://<host>:<port>)`, the legacy AMQP form, with
the REST form commented out beside it ("USE THIS WHEN Hybrid Solvers are
compiled"). Until it writes `<jmsSimHostExternal>:<jmsSimRestPortExternal>`, as
`SolverFileWriter` does for the FV solver, a Hybrid job's status never reaches
the broker; the job itself is unaffected.

## Verification — on every run of the workflow

**Statistical reference** (`tests/statistical/`). There is no working legacy
Hy3S build to compare with — `vcell-solvers` never built it (README,
*Provenance*) — so the reference is an exact solution. The birth–death model
`0 → X (k = 1000)`, `X → 0 (γ = 1)`, `X(0) = 2000`, written exactly as VCell's
`NetCDFWriter` writes a model, has

```
X(t) = Binomial(x0, e^{-γt}) + Poisson(k/γ (1 - e^{-γt}))
mean = x0 p + (k/γ)(1 - p),   var = x0 p (1 - p) + (k/γ)(1 - p),   p = e^{-γt}
```

At VCell's default thresholds (ε = 100, λ = 10) both reactions are fast for
the whole run, so each binary integrates the chemical Langevin equation with
its own scheme — the code path the ctest suite never enters. Because the
propensities are linear, the Langevin equation has exactly these two moments.
Over 1000 trials, at each of 12 save points, the sample mean and variance must
lie within 4 standard errors of the exact values, for:

| run | arguments (VCell's format) |
| --- | --- |
| `Hybrid_EM_x64` | `100.0 10.0 0.01 0.001` |
| `Hybrid_MIL_x64` | `100.0 10.0 0.01 0.001` |
| `Hybrid_MIL_Adaptive_x64` | `100.0 10.0 0.01 1.0e-4 0.01` |
| SSA control, all reactions discrete | `1e9 1e9 0.01 0.001` |
| **negative control**, must be *rejected* | Euler–Maruyama at `dt = 0.25` |

The negative control is there so a pass means something: coarse
Euler–Maruyama biases the mean by 30–50 standard errors, and the check has to
see it. Local reference run (gfortran, macOS arm64), max |z| over all save
points: EM 2.36, Milstein 1.98, adaptive Milstein 2.60, SSA 1.90, negative
control 48.8 (rejected).

It runs against: `linux64`/`linux64arm` unpacked on Rocky Linux 8 (glibc 2.28);
the image, with `--read-only` and an arbitrary uid; the SIF under
`apptainer run --containall` as a non-root user, with `-tid 0` against a mock
JMS REST bridge that must receive progress (1001) and completed (1003) events;
`mac64` on macOS 14 (arm64 slice) and on Intel macOS (x86_64 slice); and
`win64` with the compilers' directories removed from `PATH`.

**Baseline** — the ctest suite runs inside every build (the Docker build
included), and the all-discrete enzyme case is re-run through the image and the
SIF — the latter with SlurmProxy's argv shape, `-tid` included — and must match
the committed per-compiler baseline bit for bit.

## Decisions taken here

| question | decision |
| --- | --- |
| Messaging in the archives | Linux: on (it is the image). macOS, Windows: off. |
| `latest` | moves with releases only; pushes to `main` publish nothing |
| arm64 | `linux64arm.tgz` and an arm64 image, built natively on `ubuntu-24.04-arm` |
| macOS minimum | the solvers are built for macOS 11; the bundled Homebrew GCC dylibs carry Homebrew's own minimum (printed by the `mac-universal` job). CI runs the archive on macOS 14. |
| Windows runtime | static MSVC and Intel runtimes, so no redistributable is needed |
| Version stamp | the banner prints the release version (`-DHY3S_VERSION_STAMP`), not a git hash |

## Defects fixed for the first release

The first statistical reference run is what found these — the ctest suite
keeps every reaction discrete, so no test had ever entered the SDE integrators.

- **Both Milstein binaries aborted (SIGABRT) on the first SDE step.**
  `Normal_Rand` (`randomgen.f90`) assigned `min(goodnums, rsize - start + 1)`
  values to a section holding `min(goodnums, rsize - start)`: a shape mismatch,
  undefined behaviour, which corrupted the heap whenever a step needed three or
  more normal deviates. Euler–Maruyama with two fast reactions never met it.
  The count now matches the section; every previously well-defined case is
  unchanged.
- **The command-line number parser was wrong in three ways** (`dataio.f90`):
  without a decimal point an exponent misplaced the mantissa (`1e9` → 0), a
  multi-digit exponent was read digit-reversed (`1.0e-10` → 0.1), and a minus
  sign cost the integer part a power of ten. VCell always sends a decimal point
  and rarely a two-digit exponent, so it seldom showed. It is now a
  list-directed `read`, and a non-numeric parameter stops with exit code 2
  instead of being read as garbage.
