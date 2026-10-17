# VCell Stochastic Solver - Standalone Project

This is a standalone version of the VCell Stochastic solver, extracted from the main vcell-solvers repository.

## Overview

The VCell Stochastic solver implements stochastic simulation algorithms (Gibson/Gillespie) for biochemical reaction networks.

## Solver release contract (SOLVER-RELEASE)

This repository follows VCell's solver release contract (VCell `docs/plan-solver-repos.md` §1), so
VCell's desktop client and its HPC cluster can consume it by version.

**Releases.** A tag `vX.Y.Z` on `main` runs `.github/workflows/release.yml`, which attaches:

| asset | contents |
|---|---|
| `linux64.tgz` | x86_64, built on manylinux_2_28 (runs on glibc >= 2.28); depends only on glibc, `libstdc++` and `libgcc_s` from the host |
| `linux64arm.tgz` | aarch64, the same |
| `mac64.tgz` | universal (x86_64 + arm64), macOS >= 13.3, ad-hoc signed; depends only on `/usr/lib` system libraries |
| `win64.zip` | x64, static MSVC runtime; depends only on Windows system DLLs |
| `SHA256SUMS` | sha256 of each archive above |

Each archive has, at its root, `VCellStoch_x64` (`VCellStoch_x64.exe` on Windows) — the name VCell
resolves — plus `LICENSE`, `COPYING-HDF5` and `VERSION`. HDF5 (1.14.6, C library only) is linked
**statically** into the executable (`ci/build-hdf5.sh`), so there are no HDF5 `.so`/`.dylib`/`.dll`
files to bundle and nothing refers to Homebrew or vcpkg paths. The archives are built with messaging
**off**: VCell's desktop client runs `VCellStoch_x64 gibson <in>.stochInput <out>` and reads the
`[[[progress:...]]]` markers from stdout. Pull requests and pushes to `main` run the same builds and
tests without publishing.

**Container image and SIF.** `.github/workflows/container.yml` builds `docker/Dockerfile` for
linux/amd64 and linux/arm64, with messaging **on** (`OPTION_TARGET_MESSAGING=ON`, libcurl), so a
trailing `-tid <n>` reports status to VCell's broker:

- `ghcr.io/virtualcell/vcell-stochastic:<X.Y.Z>` and `:latest` (multi-arch) on a `vX.Y.Z` tag;
  `:sha-<short>` on every push to `main`;
- `oras://ghcr.io/virtualcell/vcell-stochastic_singularity:<X.Y.Z>` (amd64), and `:latest` / `:sha-<short>`
  alongside.

The runtime stage is `almalinux:8-minimal` (the distro of the manylinux_2_28 build stage, so the
system libcurl matches) plus `VCellStoch_x64` in `/usr/local/bin`.

**Entrypoint** (`/usr/local/bin/vcell-solver-entrypoint`, `docker/entrypoint.sh`):

- no argument or `--help`: prints the version and the executables provided (`VCellStoch_x64`), exit 0;
- `VCellStoch_x64 ...`: `exec`s the solver, so exit codes and signals pass through;
- anything else: usage on stderr, exit 2.

It writes nothing, so it runs as any uid from a read-only SIF, e.g. as SlurmProxy writes it:

    singularity run --containall --bind <dir>:/simdata vcell-stochastic_singularity_<X.Y.Z>.sif \
        VCellStoch_x64 gibson /simdata/SimID_1_0_.stochInput /simdata/SimID_1_0_.ida -tid 0

**Verification.** `Tests/reference/` holds five Gibson inputs (single trajectory with an output
interval, "keep every" output, histogram, multi-trial statistics with its `_hdf5` file, and the
analytic `statstest`) and the outputs of the legacy `vcell-solvers` v0.0.44-dev4 `VCellStoch_x64`
(`legacy-v0.0.44-dev4-linux64/`). `Tests/reference/compare.py` runs a build on them and checks that
the output matches the legacy files number for number (`--exact`; the seeds are fixed and the
generator is `std::mt19937_64`), plus statistical checks against the legacy statistics and the
closed-form answers. Every release build (all four platforms) and every image run it, the images
through Docker as a non-root uid and through the SIF under `apptainer run --containall`, both with
`-tid`; a messaging run must also deliver its `JOB_COMPLETED` event to a stand-in broker
(`Tests/reference/fake_broker.py`).

Python wheels (`publish-python-package.yml`) are released separately, under `python-v<version>` tags,
so they never collide with the solver's `vX.Y.Z` tags.

## Project Structure

The project is organized as follows:
- CMakeLists.txt: Root build configuration
- CMakePresets.json: Named build presets for common configurations
- VCellStoch: Main solver library and executable with include and src subdirectories
- vcell-expressionparser: Expression parsing library dependency (git submodule)
- vcommons: Common utilities dependency
- vcell-messaging: Messaging support dependency (git submodule)
- Tests: C++ unit tests
- python: Optional Python bindings (pybind11) and Python wrapper classes
- cmake: CMake modules

`vcell-expressionparser` and `vcell-messaging` are git submodules. Clone with
`git clone --recurse-submodules`, or after cloning run
`git submodule update --init --recursive`.

## Building

### Prerequisites

- CMake 3.15 or higher (3.18 or higher to build the Python package)
- C++14 compatible compiler
- HDF5 library with C and C++ components
- libcurl (optional, for messaging support)
- Python 3 with development headers and pybind11 (optional, for Python bindings)

### Build Instructions

#### Linux (with preset)

    cmake --preset linux-ninja
    cmake --build --preset linux-ninja

#### macOS (with preset)

    cmake --preset macos-ninja
    cmake --build --preset macos-ninja

Preset builds place output in `build/<preset-name>/bin/`.

#### Linux/macOS (without preset)

    cmake -S . -B build
    cmake --build build --config Release

Output is placed in `build/bin/`.

#### Windows

    cmake --preset windows-msvc-hdf5
    cmake --build --preset windows-msvc-hdf5

If you are not using presets, configure manually and provide the HDF5 installation path:

    cmake -S . -B build -DHDF5_ROOT="C:\Program Files\HDF_Group\HDF5\2.1.0"
    cmake --build build --config Release

If CMake still cannot find HDF5 on Windows, set one or both of these variables before configuring:

- `HDF5_ROOT` — HDF5 installation prefix
- `HDF5_DIR` — directory containing HDF5 CMake package files, if available

The build produces:
- Static library: build/bin/libVCellStochLib.a (Linux/macOS) or build/bin/VCellStochLib.lib (Windows)
- Executable: build/bin/VCellStoch (Linux/macOS) or build\bin\VCellStoch.exe (Windows)

### Build Options

- `BUILD_SHARED_LIBS`: Build shared libraries instead of static (default: OFF)
- `BUILD_TESTING`: Enable tests (default: ON)
- `OPTION_BUILD_PYTHON_BINDINGS`: Build Python bindings via pybind11 (default: OFF)
- `OPTION_TARGET_MESSAGING`: Enable messaging support via libcurl (default: OFF)

### Python Bindings

Use the dedicated presets to build with Python bindings enabled:

#### Linux

    cmake --preset linux-ninja-pybind
    cmake --build --preset linux-ninja-pybind

#### macOS

    cmake --preset macos-ninja-pybind
    cmake --build --preset macos-ninja-pybind

#### Windows

    cmake --preset windows-msvc-pybind
    cmake --build --preset windows-msvc-pybind

Or add `-DOPTION_BUILD_PYTHON_BINDINGS=ON` to any manual configure command.

The compiled extension (`vcellstochastic_py`) is written to `build/<preset-name>/bin/`. A higher-level Python wrapper is provided in `python/src/vcellstochastic.py`:

```python
from vcellstochastic import GibsonSolver, TrialStats

solver = GibsonSolver("model.txt", "output.h5")
solver.run()
```

## Testing

C++ tests are in the `Tests/` directory and use a custom test framework (no external test library required). When Python bindings are built, `test_binding.py` is also registered as a CTest test (`TestPythonBindings`).

### Running Tests

#### Linux/macOS (C++ tests only)

    cmake -S . -B build -DBUILD_TESTING=ON
    cmake --build build
    ctest --test-dir build --verbose

#### Linux/macOS (C++ tests + Python binding test)

    cmake -S . -B build -DBUILD_TESTING=ON -DOPTION_BUILD_PYTHON_BINDINGS=ON
    cmake --build build
    ctest --test-dir build --verbose

Or use the pybind preset (which enables `OPTION_BUILD_PYTHON_BINDINGS` automatically):

    cmake --preset linux-ninja-pybind
    cmake --build --preset linux-ninja-pybind
    ctest --test-dir build/linux-ninja-pybind --verbose

##### if macOS has HDF5 library issues:

    % sudo xattr -d com.apple.quarantine /Applications/HDF_Group/HDF5/2.1.0/lib/libhdf5_cpp.320.1.0.dylib
    % sudo xattr -rd com.apple.quarantine /Applications/HDF_Group/HDF5/2.1.0/lib/

#### Windows (with presets)

    cmake --preset windows-msvc-hdf5 -DBUILD_TESTING=ON
    cmake --build --preset windows-msvc-hdf5
    ctest --test-dir build/windows-msvc-hdf5 --verbose

#### Windows (without presets)

    cmake -S . -B build -DBUILD_TESTING=ON -DHDF5_ROOT="C:\Program Files\HDF_Group\HDF5\2.1.0"
    cmake --build build --config Release
    ctest --test-dir build --verbose

Alternatively, run the test executable directly:

    .\build\windows-msvc-hdf5\bin\TestVCellStoch.exe

## Python Package

The bindings are also packaged as a Python wheel named `pyvcell_stochastic`,
built with [scikit-build-core](https://scikit-build-core.readthedocs.io/) and
[cibuildwheel](https://cibuildwheel.pypa.io/). Installing it gives you the
`vcellstochastic` wrapper module and the `vcellstochastic_py` extension without
needing to set `PYTHONPATH` at all.

### Installing from a checkout

Make sure the submodules are present, and point `CMAKE_PREFIX_PATH` at HDF5 if
it is not in a default location:

    git submodule update --init --recursive
    CMAKE_PREFIX_PATH=/opt/hdf5 pip install .

Then:

    from vcellstochastic import GibsonSolver, TrialStats

    stats = TrialStats(num_vars=2, num_time_points=5)

`pip install .` turns on `OPTION_BUILD_PYTHON_BINDINGS` by itself, and only the
extension and its wrapper module go into the wheel — the static libraries,
headers and the `VCellStoch` executable are not included.

### Building a wheel locally

    CMAKE_PREFIX_PATH=/opt/hdf5 python -m build

This produces an unrepaired wheel in `dist/` that still links against the HDF5
installed on the build machine. To make it self-contained, run the platform's
repair tool (`delocate-wheel` on macOS, `auditwheel repair` on Linux,
`delvewheel repair` on Windows) — which is what cibuildwheel does automatically.

### Building release wheels

The `Build and Publish Python Package` workflow builds wheels for CPython
3.9–3.12 on Linux (manylinux_2_28 x86_64), Windows (x64) and macOS (arm64),
plus a source distribution. Run it from the Actions tab; supplying a `version`
input also creates the matching GitHub release. Publishing to PyPI is wired up
but commented out.

Each platform gets its HDF5 differently: the manylinux container installs
`hdf5-devel` from EPEL, macOS uses Homebrew, and Windows uses vcpkg. The repair
step then vendors HDF5 into each wheel.

### Running the Python tests

`python/tests/` holds the pytest suite that cibuildwheel runs against every
wheel it builds. Against an installed wheel:

    pip install .[test]
    pytest python/tests

This is separate from `test_binding.py`, which CTest runs against the in-tree
build via `PYTHONPATH`.

## Usage

### Running the Standalone Executable

    ./build/bin/VCellStoch {gibson|gillespie} input_filename output_filename

Options:
- gibson: Use Gibson's algorithm (Next Reaction Method)
- gillespie: Use Gillespie's algorithm (Direct Method)
- input_filename: Path to the input simulation file
- output_filename: Path for the output results

With messaging support (if built with OPTION_TARGET_MESSAGING):

    ./build/bin/VCellStoch {gibson|gillespie} input_filename output_filename [-tid 0]

### Using the Static Library

Link against libVCellStochLib.a (Linux/macOS) or VCellStochLib.lib (Windows) and include the headers from VCellStoch/include/.

Key classes:
- `Gibson`: Main Gibson (Next Reaction Method) algorithm implementation
- `StochModel`: Stochastic model representation (base class)
- `MultiTrialStats`: Accumulates mean, variance, min, and max statistics across multiple simulation trials; writes results to HDF5
- `Jump`: Reaction jump representation
- `StochVar`: Stochastic variable representation

## Cleanup

To remove build artifacts:

    cmake --build build --target clean

To completely remove the build directory:

    rm -rf build

## Installation

To install the built artifacts:

    cmake --install build --prefix /path/to/install

This will install:
- Library to <prefix>/lib/libVCellStochLib.a (Linux/macOS) or <prefix>/lib/VCellStochLib.lib (Windows)
- Executable to <prefix>/bin/VCellStoch
- Headers to <prefix>/include/VCellStoch/

## Platform Support

- Linux (tested on Ubuntu/Debian)
- macOS (Intel and Apple Silicon)
- Windows (with MSVC via presets)

## License

See the main VCell project for licensing information.
