# CLAUDE.md

Guidance for Claude Code (claude.ai/code) working in this repository. Read
`README.md` first — it covers what the project is, how to build it, and the
provenance of the split. This file records the things that are easy to get wrong.

## Orientation

Two executables, one source tree: `VCellChombo2D_x64` and `VCellChombo3D_x64`,
identical sources compiled with `CH_SPACEDIM=2` or `3` against that dimension's
Chombo libraries. VCell launches them as subprocesses against a `.fvinput` file.

`VCellChombo/src/` splits roughly into:

- `FiniteVolume.cpp` — `main`, argument handling, `vcellExit`.
- `FVSolver.cpp` — the `.fvinput` parser. One `load*` method per block
  (`SIMULATION_PARAM_BEGIN`, `MODEL_BEGIN`, `CHOMBO_SPEC_BEGIN`,
  `VARIABLE_BEGIN`, `COMPARTMENT_BEGIN`, `MEMBRANE_BEGIN`, …). The block-comment
  above each method is the format documentation; there is no schema elsewhere.
- `ChomboScheduler.cpp` / `ChomboSemiImplicitScheduler.cpp` — geometry, EB mesh
  generation and the time loop. This is where most of the volume is.
- `ChomboGeometry.cpp` / `ChomboIF.cpp` — the implicit-function geometry.
  `ChomboIF` is a Chombo `BaseIF` backed by two `vcell-expressionparser`
  expressions per subdomain.
- `SimTool.cpp` — the run driver: output files, the `.log`, `.hdf5.zip`
  archiving, progress messaging, the `.tid` lock.
- `DataSet.cpp` / `PostProcessingHdf5Writer.cpp` — HDF5 output (C API only; the
  C++ API is not used anywhere, and `hdf5_cpp` is deliberately not linked).
- `ZipUtils.cpp` — libzip wrapper, added during the split.

## Things that will bite you

**Chombo compile definitions must match the libraries.** `CH_USE_64`,
`CH_USE_COMPLEX`, `CH_USE_DOUBLE`, `CH_USE_HDF5`, `CH_USE_SETVAL`,
`CH_USE_MEMORY_TRACKING` and friends change struct layouts and inline function
bodies in Chombo's headers. `VCellChombo/CMakeLists.txt` sets them to match what
`cmake/BuildChomboLibs.cmake` passes to Chombo's make. Changing one side without
the other is an ODR violation that links cleanly and then misbehaves at runtime.

**The Fortran compiler must preprocess.** ChomboFortran generates `.f` via
`g++ -E -P -C`, and `-C` is deliberate: without it the preprocessor would treat
Fortran's `//` string-concatenation operator as a comment. The cost is that C
comment blocks survive into the `.f` — starting with gcc's implicitly included
`stdc-predef.h` — and only the Fortran compiler's own `-cpp` pass removes them.
`BuildChomboLibs.cmake` appends `-cpp` to `FC` for exactly this reason. Drop it
and every ChomboFortran file fails with "Non-numeric character in statement
label".

**2D and 3D share `chombo/lib`.** Generated `*_F.H` headers and `lib/include` are
dimension-dependent and get overwritten. The builds are serialized
(`add_chombo_dimension(3 DEPENDS chombo_2d)`) and each snapshots its headers into
`build/chombo/<dim>d/include/`. Never point a solver target at
`chombo/lib/include` directly.

**Chombo's build leaks into the source tree.** `chombo/lib/*.a`,
`chombo/lib/include/`, `chombo/lib/src/*/{o,d,f,p}/` and generated `*_F.H` are
all gitignored build products. If `CollectChomboLibs.cmake` reports finding two
candidate archives for one library, that is stale output from a different
configuration — clean the tree.

**GCC, not Clang.** gfortran's runtime links against libstdc++, so the whole
build has to be a libstdc++ world. Do not copy vcell-ode's Clang/libc++/mold
profile here.

## Modifying Chombo

`chombo/` is vendored and locally patched. Prefer surgical, well-commented edits
over an upstream re-sync — upstream Chombo has moved on and VCell's copy carries
its own fixes going back years ("Terry's fix for our version of chombo"). Every
change made during the split is commented in place explaining why.

## Submodules

`vcell-expressionparser` and `vcell-messaging` are shared with `vcell-ode`. A fix
made here has to go upstream to `virtualcell/<name>` before this repo can be
cloned by anyone else — bumping the gitlink to a commit that only exists locally
produces a checkout nobody else can resolve.

Both require C++20 (`std::format`), which is why the whole project is C++20 even
though Chombo itself is much older code.

## Input files

`tests/resources/smoke{2,3}d.fvinput` are hand-written and were built by reading
`FVSolver.cpp`; VCell normally generates these. Two non-obvious points if you
write another one:

- Each subdomain carries **two** expressions. `IF` is a level set — negative
  inside — and `USER` is a boolean predicate, non-zero inside. `ChomboIF::value`
  cross-checks them and aborts the run on disagreement, so they have to describe
  the same region in their two different conventions.
- `TIME_STEP` is not a recognized token in this solver (unlike the FV solver's
  input format). Use `TIME_INTERVALS`.

## Releases

`SOLVER-RELEASE.md` is the source of truth for what a release contains and how
it is checked. `.github/workflows/release.yml` builds it. Things that bite:

- The Linux release builds in `manylinux_2_28` with gcc-toolset-13, and
  compiles the whole Conan tree from source (`--build="*"`), because Conan
  Center's Linux binaries need a newer glibc. `packaging/bundle-linux.sh`
  fails the build if any file needs a glibc newer than 2.28.
- The macOS dylibs are bundled, not static. After changing link flags, check
  that `packaging/bundle_macos.py` still finds everything; it fails on any
  reference outside the archive.
- `tests/release/check_release.py` is the release-level test. ctest is the
  build-level one. Keep the two in step when you add an input.

## What is missing

- **MPI.** `OPTION_TARGET_PARALLEL` carries the old plumbing and warns at
  configure time. It is untested, and the release is serial only.
- **Windows.** `conanfile.py` rejects it. The MinGW-w64 attempt on the
  `windows-ci` branch stops in Chombo's `AMRTools`.
- **VCell-generated inputs.** Every input in `tests/resources/` is
  hand-written. None has yet come out of VCell's `FiniteVolumeFileWriter`.
