# vcell-chombo

The Chombo finite volume solver used by the [Virtual Cell](https://github.com/virtualcell/vcell)
framework: an embedded-boundary, adaptive-mesh PDE solver for reaction-diffusion
on curved geometry. It builds two executables from one source tree,
`VCellChombo2D_x64` and `VCellChombo3D_x64`, which VCell launches as a
subprocess against a `.fvinput` file.

This repository was split out of
[virtualcell/vcell-solvers](https://github.com/virtualcell/vcell-solvers), where
the solver had stopped being built (see [Provenance](#provenance)).

## Layout

| path | what |
| --- | --- |
| `VCellChombo/` | the solver: input parsing, VCell's variable/structure model, the Chombo scheduler, HDF5 output |
| `chombo/` | vendored [Chombo 3.x](https://commons.lbl.gov/display/chombo) with VCell's local fixes |
| `vcell-expressionparser/` | submodule — evaluates the user's rate laws, initial conditions and geometry expressions |
| `vcell-messaging/` | submodule — progress messaging to VCell's server over the JMS REST bridge |
| `cmake/` | the Chombo build driver and `GetGitRevisionDescription` |
| `tests/` | ctest smoke coverage |

## Releases

Tagged releases (`vX.Y.Z` on `main`) publish portable archives for Linux x86_64,
Linux aarch64 and universal macOS, a multi-arch image
`ghcr.io/virtualcell/vcell-chombo:<X.Y.Z>`, and an Apptainer SIF
`ghcr.io/virtualcell/vcell-chombo_singularity:<X.Y.Z>`. They follow VCell's
solver release contract. See [SOLVER-RELEASE.md](SOLVER-RELEASE.md) for what
each one contains, the container entry point, and how the releases are checked.

## Build

CMake-driven; Conan 2.x supplies HDF5, zlib, libzip and (with messaging on)
libcurl, plus `cmake`/`ninja` as `tool_requires`.

### Prerequisites

- **GCC with gfortran.** Chombo is roughly half Fortran, and gfortran's runtime
  is built against libstdc++, so the whole link has to be a libstdc++ world.
  That is why `conan-profiles/CI-CD/Linux-AMD64_profile.txt` pins GCC rather
  than the Clang/libc++/mold toolchain the other VCell solver repos use.
- **GNU make and perl**, for the vendored Chombo build (see
  [How Chombo gets built](#how-chombo-gets-built)).
- The two submodules. A plain `git clone` leaves them empty and CMake stops at
  configure time:

  ```bash
  git submodule update --init --recursive
  ```

### Canonical build

```bash
CC=gcc CXX=g++ FC=gfortran conan install . --build=missing \
      -pr:a=conan-profiles/CI-CD/Linux-AMD64_profile.txt
source build/generators/conanbuild.sh
CC=gcc CXX=g++ FC=gfortran cmake -B build -S . -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/build/generators/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      -DOPTION_TARGET_MESSAGING=ON
cmake --build build
ctest --test-dir build
```

Expect the first build to spend several minutes in Chombo — it compiles ~300
sources twice, once per dimension. `source build/generators/conanbuild.sh`
matters: it puts Conan's `cmake`/`ninja` on `PATH`, and the `ninja>=1.12.1` floor
is above what Ubuntu 24.04 ships.

Where the generator files land is decided by `layout()` in `conanfile.py`. It
flattens the tree to `build/` + `build/generators/` only when
`tools.cmake.cmaketoolchain:generator=Ninja` is set in the active profile;
without it `cmake_layout()` inserts the build type and you get
`build/Release/generators/` instead.

### Options

| option | default | effect |
| --- | --- | --- |
| `OPTION_TARGET_CHOMBO2D_SOLVER` | `ON` | build `VCellChombo2D_x64` |
| `OPTION_TARGET_CHOMBO3D_SOLVER` | `ON` | build `VCellChombo3D_x64` |
| `OPTION_TARGET_MESSAGING` | `OFF` | link libcurl and report progress to VCell's server; without it `vcell-messaging` still builds, but the curl path is replaced by `NullCurlProxy` |
| `OPTION_TARGET_PARALLEL` | `OFF` | MPI build. Carried over from the in-tree version and **not currently exercised** — configure warns |
| `CHOMBO_BUILD_JOBS` | `nproc` | parallelism for the Chombo GNU make step |
| `OPTION_EXTRA_CONFIG_INFO` | `OFF` | dump every CMake variable at the end of configure |

Turning off a dimension halves the build time, which is worth it while
iterating: `-DOPTION_TARGET_CHOMBO3D_SOLVER=OFF`.

## How Chombo gets built

Chombo ships a large, self-contained GNU make system that also runs the
ChomboFortran preprocessor (perl) over its `.ChF` sources. Reimplementing that in
CMake would mean reimplementing ChomboFortran, so `cmake/BuildChomboLibs.cmake`
drives Chombo's own `make lib` instead — but passes every configuration variable
(`DIM`, `OPT`, `FC`, `HDFINCFLAGS`, …) on the command line, where make gives it
precedence over anything in a makefile. Nothing is written into `chombo/lib/mk`,
so one checkout can serve several build trees.

Two details are worth knowing before you touch it:

- Chombo bakes its configuration into each archive's name
  (`libboxtools2d.Linux.64.g++.gfortran.OPT.a`). Predicting that string in CMake
  is what made the old in-tree build so fragile. `cmake/CollectChomboLibs.cmake`
  globs for the archives after the fact and copies them to stable paths under
  `build/chombo/<dim>d/lib/`.
- Everything under `chombo/lib` is shared between dimensions, including the
  generated `*_F.H` headers and `lib/include`, and those *are* dimension
  dependent. The 2D and 3D builds are therefore serialized, and each one's
  headers are snapshotted to `build/chombo/<dim>d/include/` before the next runs.

`chombo/lib` picks up build products in place. They are gitignored; `make
realclean` in that directory (or deleting `*.a`, `include/`, `src/*/{o,d,f,p}`
and the generated `*_F.H`) resets it.

## Tests

`ctest --test-dir build` runs, per dimension:

- `VCellChombo<N>D_x64_usage` — the solver prints its usage and exits non-zero
  when given no input file.
- `VCellChombo<N>D_x64_smoke` — runs `tests/resources/smoke<N>d.fvinput` end to
  end and checks that the `.log`, `.mesh.hdf5` and `.hdf5.zip` VCell expects were
  written, and that the archive holds the `.sim.hdf5` files the log names.

The smoke inputs are hand-written (VCell normally generates `.fvinput` files) and
deliberately minimal: a disc/sphere of `cyt` inside a box of `ec`, one volume PDE
initialised to 1.0 with no flux across the membrane and no reaction, so the
solution stays 1.0. That covers the whole pipeline — implicit-function geometry,
EB mesh generation, the semi-implicit solve, HDF5 output, zip archiving — but it
is **not** a numerical regression suite. Nothing here compares against baseline
values; those baselines live with VCell's own integration tests.

## Licensing

The root `LICENSE` (MIT) covers the code in this repository. It does **not**
cover `chombo/`, which is vendored third-party source: Chombo is
Copyright (c) 2000-2012 The Regents of the University of California through
Lawrence Berkeley National Laboratory, under a BSD-3-Clause-style licence
reproduced verbatim in [`chombo/Copyright.txt`](chombo/Copyright.txt). That
licence requires the copyright notice to be retained in both source and binary
redistributions, so keep that file with the tree and carry it into any packaged
build.

The two submodules carry their own licences.

## Provenance

VCellChombo had not been compiled in years. Its `CMakeLists.txt` guarded
everything on `OPTION_TARGET_CHOMBO_SOLVER`, a variable no caller ever set, and
listed sources that had been deleted when messaging was centralized; every CI
recipe in vcell-solvers passes `-DOPTION_TARGET_CHOMBO{2,3}D_SOLVER=OFF`. Getting
it building again needed changes on both sides of the split:

**Chombo** — four classes of code modern compilers reject: default arguments on
friend declarations (`ProblemDomain`, `IntVectSet`, `DisjointBoxLayout`), an
`istream`-to-`bool` conversion, comparison functors that are not const-invocable
(C++17 tightened `std::map`), and the BSD `HUGE` constant glibc no longer
defines. Two build-system fixes as well: `mk/reverse` was a csh script (csh is
not installed by default on modern Linux or macOS), and GCC's `-dumpversion` has
printed only the major number since GCC 7, which left Chombo's minor-version
tests failing with "unexpected operator".

**VCellChombo** — `SimTool` called `zip32()`/`unzip32()`, which no longer exist
anywhere in vcell-solvers (the FV solver moved to libzip years ago); they are now
a small libzip wrapper. The `vcell-messaging` API has since been rewritten
(`JobEvent::Status`, `setWorkerEvent` overloads instead of `new WorkerEvent`,
lazy singleton with `cleanupInstanceVar`), and the JMS REST bridge no longer
needs the queue/topic/password fields the input file still carries.

**vcell-expressionparser** — `ASTFloatNode::infixString` formatted every float as
the literal string `":.20g"`, so any `infix()` round-trip failed to re-parse.
Chombo hits this on every `newImplicitFunction()` call, i.e. on every geometry.
