# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

A C++14 solver library for **moving-boundary parabolic PDE problems** in biological systems (part of VCell). One CMake build produces three artifacts: a CLI binary (`MovingBoundarySolver`), a linkable library (`libMovingBoundaryLib`), and a pybind11 Python extension (`vcellmbsolver_py`).

## Submodules

Two of the bundled libraries are git submodules (see `.gitmodules`) — after cloning, run `git submodule update --init --recursive` or the build fails at `add_subdirectory`. `FronTierLib`, `vcommons`, and `sqlite` are vendored directly in-tree.

- **`vcell-expressionparser/`** → static library `vcellexpressionparser`. The math expression engine (`Expression`, `SymbolTable`, AST node classes, `StackMachine`) that the solver uses to evaluate reaction/diffusion/velocity formulas. The parser is **JavaCC-generated**: `Parser.jjt` is the grammar source, and the `ExpressionParser*.h`, `AST*.h`, `Token*.h`, `*CharStream.h` files are generated output — edit the grammar, not the generated files.
- **`vcell-messaging/`** → library `vcellmessaging` (`SimulationMessaging.h`, `GitDescribe.h`). Job-status/progress messaging. It only links CURL and defines `USE_MESSAGING=1` when CURL is present, which is gated by the top-level `OPTION_TARGET_MESSAGING` option (OFF locally, ON in CI).

## Build & Test

The `README.md` has full per-platform instructions. The common flow:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel        # outputs land in build/bin/
cd build && ctest --output-on-failure  # runs all three suites
```

Run a subset of tests:
```bash
ctest --output-on-failure -R "algo"              # gtest cases matching a pattern
ctest -R "ExpressionParser"                      # expression parser smoke test
ctest -R "PyVcellMbSolver"                       # python wrapper tests
ctest -N                                         # list tests without running
```
`cmake -S . -B build` recreates an isolated venv at `build/python_venv/` and installs `pytest` into it; ctest sets `PYTHONPATH` so `vcellmbsolver_py` and `vcellmbsolver` import from the build tree.

Key CMake options: `BUILD_SHARED_LIBS` (default OFF), `BUILD_TESTING` (default ON), `VARIABLE_SPECIES_STORAGE` (defines `MB_VARY_MASS`), `OPTION_TARGET_MESSAGING` (job-status messaging via CURL, OFF locally / ON in CI).

Run the binary: `./build/bin/MovingBoundarySolver --config <input_mb.xml> [-tid <n>]` (output goes to the input's `<report><outputFilePrefix>`). Input XML is validated against `Solver/MovingBoundarySetup.xsd`.

**Releases for VCell** (archives `linux64.tgz`/`linux64arm.tgz`/`mac64.tgz`/`win64.zip` with the executable renamed `MovingBoundary_x64`, the `ghcr.io/virtualcell/vcell-mbsolver` image and `_singularity` SIF, the `smoke/` reference comparison) are built by `.github/workflows/build-and-release.yml` from `packaging/` and `docker/`; see the README's *Releases for VCell (SOLVER-RELEASE)* section. Keep `project(... VERSION ...)` in `CMakeLists.txt` equal to the release tag.

## Architecture

The solver core lives in `Solver/` (`include/` headers, `src/` implementation). Almost everything is in the `moving_boundary` and `spatial` namespaces (utilities in `vcell_util`).

- **`MovingBoundaryParabolicProblem`** (`MovingBoundaryParabolicProblem.h`) is the public facade. It is a thin handle holding a shared pointer to **`MovingBoundaryParabolicProblemImpl`** (pimpl, defined in the `.cpp`), where the simulation actually lives. `Impl::run()` is the main time-stepping loop.

- **Front tracking**: the moving boundary is represented by `VCellFront` (`VCellFront.h`), a C++ wrapper over the bundled **FronTier** library (`FronTierLib/`). Each step calls `vcFront->propagateTo(time)` to advance the front, then re-meshes.

- **Mesh & geometry**: `VoronoiMesh`/`Voronoi*` build a Voronoi mesh of the interior; `MeshElementNode` is the per-cell unknown carrying concentrations (`concentration(i)` / `prior_concentration(i)`). `World`/`Universe` (`World.h`, `Universe.h`) define the global coordinate system — note the distinction between integer **world coordinates** (`CoordinateType`) used internally and floating-point **problem-domain** coordinates exposed to users; convert via `World<>::toProblemDomain(...)`.

- **Physiology / species** (`Physiology.h`): a problem has volume species (`VolumeVariable`) and point species (`PointVariable`) grouped into `VolumeSubdomain`/`PointSubdomain`. Reaction/diffusion/velocity expressions are parsed by the expression parser (`MTExpression`, `SExpression`).

- **Setup**: `MovingBoundarySetup::setupProblem(xmlRoot, taskId, nx)` parses the input XML (tinyxml2) into the setup struct that constructs a problem.

- **Output / clients**: results are pushed to registered observers. `MovingBoundaryTimeClient` (per-timestep) and `MovingBoundaryElementClient` (per-element) are the client interfaces; concrete clients include `Hdf5OutputWriter` (the `.h5` output), `TextReportClient`, `ReportClient`, and `StateClient` (checkpoint/restore via `persist`). `MBridge/` generates MATLAB debug/visualization output.

### Python bindings

The bindings ship as the `pyvcell_mbsolver` package (mirroring the sibling `vcell-fvsolver` / `pyvcell_fvsolver` convention): `python/pyvcellmbsolver.cpp` is the compiled pybind11 module installed as the private submodule `pyvcell_mbsolver._core` (exposing C++ types directly), and `python/pyvcell_mbsolver/__init__.py` is the high-level wrapper — `MovingBoundarySolver` facade plus `TimeStepObserver` / `SimulationObserver` abstract bases whose subclasses adapt to the C++ `*Client` interfaces. The wrapper imports the extension via `from . import _core as _mb`. Python tests are in `python/tests/`.

### Python packaging (wheels)

`pyproject.toml` uses the `scikit-build-core` backend to drive the existing CMake build into a wheel (`pip install .` / `python -m build`). It builds only the `_core` target and installs only the `python` install component (the `pyvcell_mbsolver` package) — the C++ lib/binary/headers are excluded; tests and messaging are forced off. `python/CMakeLists.txt` assembles the package under `build/bin/pyvcell_mbsolver/` (extension + a copied `__init__.py`) so it imports from the build tree with `PYTHONPATH=build/bin`; the install destination is `pyvcell_mbsolver/` (relative) when `SKBUILD` is defined (wheel build) vs. `${Python3_SITEARCH}/pyvcell_mbsolver` for a plain `cmake --install`. The wheel version is scraped from `project(VCellMovingBoundary VERSION ...)` in the root `CMakeLists.txt`, so that line is the single source of truth. `.github/workflows/wheels.yml` runs cibuildwheel across Linux (x86_64 + arm64) and macOS (x86_64 + arm64); a `publish` job does PyPI trusted-publishing on `v*` tags only. **Windows wheels are not built** — the bundled FronTier library does not compile under MSVC (Unix/GCC code, `POINT`/`boolean` clashes with the Windows SDK, GCC anonymous-struct tags).

### Tests

`Tests/` holds the GoogleTest suite (`TestMovingBoundary`, one `*test.cpp` per area, registered via `gtest_discover_tests`). GTest is found via `find_package` or fetched with FetchContent (v1.14.0). `ExpressionParserTest/` is a separate standalone-main smoke test.

## Conventions

The code predates C++14 idioms in places (e.g. `std::auto_ptr`, raw owning pointers) and is compiled with `-fpermissive` on Unix/macOS. Match the surrounding style when editing; don't modernize broadly unless asked.
