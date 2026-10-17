# vcell-hy3s

Hy3S — Hybrid Stochastic Simulation for Supercomputers — as used by the
[Virtual Cell](https://github.com/virtualcell/vcell) framework. It simulates
chemical kinetics by partitioning reactions into a fast subset integrated as a
stochastic differential equation and a slow subset handled by discrete jumps.

Three executables come from one Fortran source set, differing only in the SDE
integrator compiled in:

| binary | integrator |
| --- | --- |
| `Hybrid_EM_x64` | Euler–Maruyama |
| `Hybrid_MIL_x64` | Milstein, fixed step |
| `Hybrid_MIL_Adaptive_x64` | Milstein, adaptive step |

Split out of
[virtualcell/vcell-solvers](https://github.com/virtualcell/vcell-solvers), where
it was switched off in every build recipe — and, it turns out, could not have
been built at all (see [Provenance](#provenance)).

## Layout

| path | what |
| --- | --- |
| `Hy3S/` | the solver: 13 Fortran 90 sources plus a C++ shim for messaging |
| `netcdf/` | vendored NetCDF 3.6.2 — C, F77 and F90 layers |
| `vcell-messaging/` | submodule — progress messaging over the JMS REST bridge |
| `cmake/` | `GetGitRevisionDescription` |
| `docker/` | the release image: manylinux build, slim runtime, standard entrypoint |
| `packaging/` | assembles the release archives (bundled libraries, `LICENSE`, `VERSION`) |
| `tests/` | ctest suite; `statistical/` and `smoke/` for the release checks |

There is no expression parser here; Hy3S does not use one.

## Releases

Tagged releases publish `linux64.tgz`, `linux64arm.tgz`, `mac64.tgz`
(universal), `win64.zip` and `SHA256SUMS`, the image
`ghcr.io/virtualcell/vcell-hy3s:<X.Y.Z>` and the Apptainer SIF
`oras://ghcr.io/virtualcell/vcell-hy3s_singularity:<X.Y.Z>`. How they are
built, what is in them, how they are verified (including a statistical check of
every integrator against an exact solution), and how to cut one:
[SOLVER-RELEASE.md](SOLVER-RELEASE.md).

```bash
docker run --rm ghcr.io/virtualcell/vcell-hy3s:1.0.0                 # version and executables
docker run --rm -v "$PWD:/simdata" ghcr.io/virtualcell/vcell-hy3s:1.0.0 \
    Hybrid_EM_x64 /simdata/model.nc 100.0 10.0 0.01 0.001 -OV
```

## Build

```bash
git submodule update --init --recursive     # vcell-messaging
CC=gcc CXX=g++ FC=gfortran conan install . --build=missing \
      -pr:a=conan-profiles/CI-CD/Linux-AMD64_profile.txt
source build/generators/conanbuild.sh
CC=gcc CXX=g++ FC=gfortran cmake -B build -S . -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/build/generators/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      -DOPTION_TARGET_MESSAGING=ON
cmake --build build
```

Prerequisites are a GCC with **gfortran** — Clang has no Fortran compiler, and
gfortran's runtime is built against libstdc++, so the libc++ toolchain the ODE
repo uses is not an option either.

Conan carries very little here: **libcurl is the only dependency, and only when
messaging is enabled.** With `-DOPTION_TARGET_MESSAGING=OFF` the build needs
nothing external at all.

### Why NetCDF is vendored rather than a Conan package

Hy3S does `USE netcdf`, so it needs the Fortran 90 bindings. Conan Center
publishes `netcdf` as the **C library only** — no `.mod` files — and has no
`netcdf-fortran` recipe. Unidata's netcdf-fortran cannot be pulled in with
`FetchContent` either: it refers to `CMAKE_SOURCE_DIR` internally and so insists
on being the top-level project.

The vendored 3.6.2 tree carries its own C, F77 and F90 layers, is what this code
was written against, and builds cleanly with modern gcc/gfortran. It needed only
two changes: declaring C alongside Fortran, and dropping a hardcoded path to the
Intel compiler.

Three committed 2007-era MSVC binaries — `netcdf_f90.lib`, `netcdf_for.lib` and
`netcdf.lib` — have been deleted. Nothing in the CMake build referenced them,
and the Windows job builds all three libraries from the vendored sources. The
only things that named them were the superseded `Makefile.win32` files, which
still point at a `netcdf-3.6.2/` directory this repo does not have.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

Five cases in about six seconds, on all three platforms. The substantive checks
are exact rather than tolerance-based: the enzyme-kinetics model conserves total
enzyme and total substrate at every timepoint by stoichiometry, independent of
the trajectory.

The one trajectory-dependent check, against a committed baseline, needs a
baseline per **compiler** rather than per platform: Fortran's `RANDOM_NUMBER`
has no specified algorithm, so gfortran and Intel draw different streams from
the same seed. Linux and both macOS architectures match each other to the bit;
Windows has its own baseline.

See [tests/README.md](tests/README.md), which also explains why one case runs
the solver twenty times.

## Platforms

All three build in CI: Linux and macOS (arm64 and x86_64) with GCC, Windows with
Intel Fortran and MSVC.

Linux and macOS use GCC because Clang has no Fortran compiler, and gfortran's
runtime links against libstdc++, so the libc++ toolchain the ODE repo uses is
not an option either.

Windows pairs **Intel Fortran (`ifx`) with MSVC**, which is what this code was
originally built with — the evidence is still in the tree:
`-DPowerStationFortran` and a `netcdf/win32/config.h` for the C layer, Intel's
`/iface:mixed_str_len_arg` flags for the F90 layer, and the uppercase
`__cdecl LOAD_JMS_INFO` entry points in `msgwrapper.cpp` that match Intel
Fortran's Windows name mangling. MSVC compiles the C and C++, `ifx` compiles the
Fortran and hands the link to MSVC's `link.exe`.

**No MSYS2 or Cygwin is involved.** Unlike Chombo, Hy3S has no build system of
its own — no GNU make, no perl, no custom preprocessor, just CMake over plain
Fortran. Messaging is off on Windows, as everywhere in VCell, which leaves the
Windows build with **no external dependencies whatsoever**.

Two things are worth knowing if you touch the Windows build:

- `ifx` must be invoked with the MSVC developer environment active. Without it
  it fails with `error #10037: could not find 'link'`, and CMake reports the
  Fortran compiler as `unknown` — compiler identification links a test program.
- The build uses the Ninja generator, not the Visual Studio one, which needs
  oneAPI's IDE integration to see Fortran at all.

Both compilers were checked against the two things this repo changed in the
Fortran. `ifx` accepts the `#if` directives converted from `!DEC$ IF`, and it
reads source lines past column 132 — `ratelaws.f90` has arithmetic reaching 150
columns, which gfortran takes via `-ffree-line-length-none` and for which Intel
has no unlimited equivalent. Neither was assumed; a probe measured both.

## Provenance

Hy3S was an **Intel Fortran** project, and it had not been built in a long time.
Every recipe in vcell-solvers passes `-DOPTION_TARGET_HY3S_SOLVERS=OFF`, and it
would not have built if they had not: `mainprogram-HyJCMSS.f90` contains

```c
#ifndef SVNVERSION
#error "SVNVERSION required"
#endif
```

and nothing in that repository ever defined `SVNVERSION`. It is now stamped from
the git describe, which is what it was asking for.

Getting it building with gfortran turned up four more issues, three of them real
defects rather than configuration:

- **114 Intel `!DEC$ IF` directives.** gfortran does not implement these — it
  treats them as comments and compiles the guarded code regardless, so a
  messaging-free build would still try to link the messaging calls. All were
  converted to standard `#if`/`#else`/`#endif`. Intel's `Defined()` matched
  case-insensitively, so the sources said `Milstein` where the build defined
  `MILSTEIN`; the conversion normalises to upper case, since the C preprocessor
  draws no such equivalence. The result works under both compilers.
- **The version banner could not compile.** It stringified an unquoted macro
  with `#x`, and gfortran's `-cpp` preprocesses in *traditional* mode, which
  predates ANSI stringification. It is now supplied already quoted and
  concatenated with `//`.
- **A type bug in the messaging path.** `Call send_progress(percentile, i)`
  passed the `Integer` trial counter where the C side dereferences a `double*` —
  four bytes read as eight. Invisible until now, since Hy3S was never built and
  Intel's implicit interfaces would not have flagged it.
- **`f2kcli` declared `IARGC` `EXTERNAL`**, which defeats gfortran's intrinsic
  and leaves `iargc_` unresolved at link. Intel Fortran turned out to provide
  `IARGC` as an intrinsic too, and failed the same way. The file's own comment
  says those declarations "should not really be necessary" and were added for
  PGI, so they are now made for PGI alone.
- **An uninitialised error flag stopped about one optimised run in five.**
  `CheckAndDefineVariables` declares `integer, intent(out) :: error` and assigns
  it only on its ten failure paths, never on success. The caller does set it to
  zero first — but `intent(out)` promises the callee will define it, so at `-O1`
  and above gfortran deletes that store as dead code, exactly as the standard
  permits, and the subsequent `if (error == 1)` read an undefined stack slot.
  Affected runs printed the equally undefined 256-character `errormsg` as binary
  garbage and stopped without simulating — **while still exiting 0 and leaving
  the output file untouched.** At `-O0` the store survives, which is why the
  solver looked fine there. This is why the test suite asserts that a run
  actually happened, and why one case repeats twenty times.

Two more surfaced only once a test entered the SDE integrators, which the
ctest suite deliberately never does (see [SOLVER-RELEASE.md](SOLVER-RELEASE.md#defects-fixed-for-the-first-release)):

- **Both Milstein binaries aborted on their first SDE step.** `Normal_Rand`
  assigned one more normal deviate than the array section it filled held — a
  shape mismatch that corrupted the heap whenever a step needed three or more.
- **The command-line number parser was wrong** for exponents without a decimal
  point (`1e9` read as 0), for multi-digit exponents (read digit-reversed) and
  for negative numbers. It is now a standard list-directed `read`.

`msgwrapper` also drove the old in-tree `SimulationMessaging` — `create()`, an
explicit `start()`, and `new WorkerEvent(JOB_PROGRESS, …)`. None of that exists
in the current `vcell-messaging`, so it was ported to the lazy singleton and
`JobEvent` statuses, the same migration vcell-chombo needed.
