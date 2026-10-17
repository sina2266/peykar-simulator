# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

`vcell-fvsolver` is the Virtual Cell finite-volume reaction–diffusion–advection PDE solver. It is a C++17/C/Fortran codebase that ships in two forms from a single CMake build:

- **Standalone executables** `FiniteVolume_x64` and `smoldyn_x64` (consumed by the VCell modeling app).
- **Python wheel `pyvcell_fvsolver`** (built via scikit-build-core / pybind11), which exposes `version()` and `solve(fvInputFilename, vcgInputFilename, outputDir)` from `src/main.cpp`. Inputs are VCell `.fvinput` + `.vcg` files; outputs are `.log`, `.zip`, `.mesh`, `.meshmetrics`, `.hdf5`.

Top-level `CMakeLists.txt` has a hard switch between these two modes via `OPTION_TARGET_PYTHON_BINDING`. When that option is ON it forces several other options (messaging, docs, libtiff, iconv) off and pulls the project name/version from the scikit-build environment — pure CMake invocations must pass `-DOPTION_TARGET_PYTHON_BINDING=OFF` to build the executables.

## Build & test

Both build modes need a Fortran compiler (flang ≥ 21 in CI), a C/C++ compiler (clang), CMake ≥ 3.22, and Conan 2 to provide hdf5, libaec, libzip, zlib (see `conanfile.txt`). On macOS, libzip/libaec come from Homebrew rather than Conan and CMake calls `brew --prefix` directly.

The submodules (`extern/pybind11`, `libzippp`) must be initialized: clone with `--recurse-submodules` or run `git submodule update --init --recursive`.

### Native executables (Linux/macOS)

```bash
mkdir build && cd build
conan install .. --output-folder . --build=missing       # Linux/Windows; macOS skips this
source conanbuild.sh                                     # Linux/Windows only
cmake -B . -S .. -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \         # omit on macOS
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBZIPPP_CMAKE_CONFIG_MODE=ON \                      # omit on macOS, see gotcha below
  -DOPTION_TARGET_PYTHON_BINDING=OFF \
  -DOPTION_TARGET_MESSAGING=OFF \
  -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
  -DOPTION_TARGET_FV_SOLVER=ON \
  -DOPTION_TARGET_TESTS=ON
cmake --build . --config Release
ctest -VV                                                # runs gtest suite + smoke test
./bin/FiniteVolume_x64                                   # built executable
```

macOS prerequisites: `brew install hdf5 libzip libaec flang`, plus an up-to-date Command Line Tools install whose SDKs include the SDK matching your macOS major version (Homebrew flang's driver looks for `/Library/Developer/CommandLineTools/SDKs/MacOSX<N>.sdk` and the linker fails with `ld: library 'System' not found` if it's missing — `sudo rm -rf /Library/Developer/CommandLineTools && xcode-select --install` to refresh).

`Dockerfile` is the Linux release build: manylinux_2_28 with gcc-toolset-14 (gcc/g++/gfortran), Conan 2 dependencies from `docker/conan/` plus a static HTTP-only libcurl for messaging, `OPTION_TARGET_MESSAGING=ON`, then a slim runtime image (see `SOLVER-RELEASE.md`). `.github/workflows/cd.yml` is the canonical reference for the macOS / Windows (LLVM clang + flang) and Ubuntu CI builds and for which CMake flags are passed in each environment; `.github/workflows/container.yml` builds the Linux archives, image and SIF.

### Python wheel / bindings

```bash
pip install -e .                                                   # editable dev install
python -m build --wheel -o ./wheelhouse                            # produce wheel
nox -s dev                                                         # writes a .venv with editable install + compile_commands.json
nox -s tests                                                       # pip install .[test] && pytest
pytest tests/test_basic.py::test_version_function                  # single test
```

`pyproject.toml` enables `filterwarnings = ["error"]`, so any Python warning fails tests.

### Running individual C++ tests

`OPTION_TARGET_TESTS=ON` builds `vcell_test` (googletest, fetched via `FetchContent` in `VCell/tests/CMakeLists.txt`) and registers it with CTest via `gtest_discover_tests`.

```bash
ctest -VV -R VCellTest                          # filter by test name regex
./VCell/tests/vcell_test --gtest_filter=VCellTest.ExcerciseFV
```

The smoke test (`VCell/tests/smoke/smoke.py`) runs `FiniteVolume_x64` against a templated `.fvinput.in` and diffs the resulting HDF5 structure against `*.hdf5.expected`. It needs `h5py` and `numpy` from whichever `python3` is on `PATH` when `ctest` runs. The repo has a `.venv/` for this; install the deps once and put it on PATH:

```bash
.venv/bin/python3 -m pip install h5py numpy
PATH="$(git rev-parse --show-toplevel)/.venv/bin:$PATH" ctest      # or: source .venv/bin/activate
```

`VCell/tests/CMakeLists.txt` registers the test with the literal command `python3 …/smoke.py`, so the test only resolves whichever `python3` is on `PATH` at ctest time — no rebuild is needed when you swap Python environments.

The `gtest` suite uses an auto-generated header `VCell/tests/testFiles/input/testResourceLocations.h` (configured from `.h.in`); regenerate it by re-running CMake configure if you add new test inputs.

## Architecture

### Module layout (build-order-meaningful)

The top-level CMake adds subdirectories in this order, which encodes the dependency graph:

1. **`VCellMessaging`**, **`VCellZipUtils`**, **`libzippp`** — thin utility libs (messaging is compiled out unless `OPTION_TARGET_MESSAGING=ON`, which also requires libcurl).
2. **`ExpressionParser`** — JJTree-generated expression parser (see `Parser.jjt`); produces an AST + `StackMachine` evaluator used everywhere by `*VarContextExpression` classes.
3. **`sundials`**, **`blas`** (skipped on Apple — uses the system Accelerate framework instead), **`PCGPack`**, **`qhull`** — vendored numerical libraries. PCG (preconditioned conjugate gradient) is wrapped through a Fortran shim (`VCell/src/pcgwrapper.f`) so the C++ sparse solver can call into it.
4. **`smoldyn-2.38`** — vendored Smoldyn particle simulator. Built with VCell-specific defines (`-DVCELL_HYBRID -DVCELL`).
5. **`bridgeVCellSmoldyn`** — adapters that let the FV solver and Smoldyn share a mesh/value source (`SimpleMesh`/`VCellMesh`, `SimpleValueProvider`/`VCellValueProvider`) and that emit Smoldyn output through the FV `DataGenerator`/HDF5 pipeline. Builds the `vcellsmoldynbridge` static lib and the `smoldyn_x64` executable.
6. **`VCell`** — the FV solver core. Builds the `vcell` static lib (everything else links against it) and the `FiniteVolume_x64` executable from `VCell/src/FiniteVolume.cpp`.

When the Python target is on, `src/main.cpp` is built into the `_core` pybind11 extension that links against `vcell` and is installed into `pyvcell_fvsolver/`.

### VCell solver core (`VCell/src` + `VCell/include/VCELL`)

Roughly 70 `.cpp` files; the conceptual layers are:

- **Model description**: `VCellModel` aggregates `Feature` (volume subdomains), `Membrane`, and `Structure`. Each carries `VarContext` subclasses (`VolumeVarContextExpression`, `MembraneVarContextExpression`, `*RegionVarContextExpression`, `MembraneRegionVarContextExpression`) that bind named expressions (initial conditions, diffusion, advection, reaction, boundary/jump) to symbol tables resolved at runtime by `ExpressionParser`.
- **Mesh & geometry**: `Mesh` / `CartesianMesh` hold `VolumeRegion`, `MembraneRegion`, and `MembraneElement` connectivity. `FieldData` and `RandomVariable` provide spatially varying inputs.
- **Variables**: `Variable` hierarchy → `VolumeVariable`, `MembraneVariable`, region variants, plus `*ParticleVariable` for hybrid Smoldyn coupling. `RegionSizeVariable` is computed, not solved.
- **Equation builders → solvers**: `EqnBuilder` subclasses (`SparseVolumeEqnBuilder`, `EllipticVolumeEqnBuilder`, `MembraneEqnBuilderDiffusion`, `*RegionEqnBuilder`, `EqnBuilderReactionForward`) populate `SparseMatrixPCG` matrices that the `PDESolver` (`SparseLinearSolver` via PCG, or `StructuredPDESolver` for ADI) drives. Stiff problems use `SundialsPdeScheduler` (CVODE); non-stiff use the operator-splitting `SplitScheduler`/`SerialScheduler`. ODE-only problems go through `ODESolver`. Algebraic constraints are handled by `FastSystem` / `FastSystemExpression` (Newton on a per-point reduced system, used for fast-equilibrium reactions).
- **Driver**: `SimTool` is the top-level orchestrator. `FVSolver` parses `.fvinput`/`.vcg`, builds the model, picks a scheduler, and drives stepping. `SimulationExpression` couples `Simulation` with the expression-based variable contexts. Per-timestep output flows through `PostProcessingBlock` → `PostProcessingHdf5Writer` and the `DataGenerator` family (`Gaussian`/`Projection`/`Roi`/`VariableStatistics`).
- **Entrypoints**: `SolverMain.h` exposes `version()` + `solve()` for both the CLI (`FiniteVolume.cpp`) and the Python module (`src/main.cpp`).

### Hybrid FV/Smoldyn coupling

`bridgeVCellSmoldyn/vcellhybrid.{h,cpp}` flips Smoldyn into a "hybrid" mode (`vcellhybrid::setHybrid()` is called at the top of `FiniteVolume.cpp` `main()` to put the linked Smoldyn library in the correct state before any FV setup runs). The bridge layer lets Smoldyn read concentrations from the FV solver via `VCellValueProvider` and write particle output through the FV HDF5 writer (`SmoldynHdf5Writer`).

### Static linking convention

`CMAKE_FIND_LIBRARY_SUFFIXES` is forced to `.a` and `BUILD_SHARED_LIBS=OFF` at the top of the top-level `CMakeLists.txt` — both the standalone executables and the Python extension bundle dependencies statically. CI then runs platform-specific scripts (`.github/scripts/install_name_tool_macos.sh`, `ldd`-walking on Linux, `lipo` for universal Mac binaries) to pull system libs into the artifact. If you add a dependency, plan on a static build.

## Conventions / gotchas

- macOS skips Conan entirely and uses Homebrew for `libzip`, `libaec`, `hdf5`, `boost`, `flang`. Do not assume `conan_toolchain.cmake` exists in macOS builds. macOS also skips the bundled `blas` directory and uses the system Accelerate framework.
- **`-DLIBZIPPP_CMAKE_CONFIG_MODE=ON` breaks local macOS builds** even though CI passes it. The top-level `find_package(LIBZIP)` uses the bundled `libzippp/cmake/FindLIBZIP.cmake` (module mode) which defines `libzip::zip`; with that flag set, `libzippp/CMakeLists.txt` *additionally* calls `find_package(libzip CONFIG)`, which loads Homebrew's `libzip-config.cmake` and tries to import `libzip::zipcmp`/`zipmerge`/`ziptool`, hitting a "Some (but not all) targets in this export set were already defined" error. Newer Homebrew libzip exports more targets than older ones, so the flag works in some environments and breaks in others — **omit it on macOS**, keep it on Linux/Windows where Conan provides libzip.
- The default `OPTION_TARGET_PYTHON_BINDING=ON` only works when invoked through `pip` / `scikit-build-core`, because the `project(${SKBUILD_PROJECT_NAME} ...)` call needs scikit-build to inject those vars. A bare `cmake ..` with the default silently configures with project name `"VERSION"` and `OPTION_TARGET_FV_SOLVER=OFF` — always pass `-DOPTION_TARGET_PYTHON_BINDING=OFF` for native builds.
- `OPTION_VCELL` and `VCELL_HYBRID` defines are required for the Smoldyn build to behave as the hybrid FV/particle solver rather than vanilla Smoldyn.
- `FORTRAN_UNDERSCORE` is added globally; the Fortran ↔ C boundary in `pcgwrapper.f` depends on it.
- `Parser.jjt` is the source of the JavaCC/JJTree-generated parser files in `ExpressionParser/`. The generated `.cpp`/`.h` files are checked in — regenerate them deliberately rather than editing the generated files by hand.
- The `cmake-build-debug-without-binding-with-msg/` directory is a CLion build tree, not source — don't edit files inside it.
