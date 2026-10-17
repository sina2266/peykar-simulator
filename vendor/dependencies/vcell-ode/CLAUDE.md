# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

`vcell-ode` is the ODE/DAE numerical solver used by the Virtual Cell framework (https://github.com/virtualcell/vcell). It produces two artifacts from one source tree:

- **A standalone CLI** (`SundialsSolverStandalone_x64`) that VCell launches as a subprocess against a `.cvodeInput` file.
- **A Python wheel** (`pyvcell_odesolver`) exposing the same `solve()` entry by `ctypes`-loading the shared `IDAWin` library out of `pyvcell_odesolver/lib/`.

The Python side is **pure Python + ctypes** — there is no compiled extension module. `IDAWin/SundialsSolverInterface.h` exports `extern "C"` shims (`version_ctypes`, `solve_ctypes`) alongside the C++ `version()`/`solve()`, and `pyvcell_odesolver/_internal/native_utils.py` binds them at import time. The vendored `extern/pybind11/` tree was removed once nothing referenced it.

The numerical core is CVODE/IDA from a vendored copy of `sundials/`, wrapped by `IDAWin/`, with user rate laws evaluated via the `vcell-expressionparser` submodule. Optional progress messaging (libcurl → ActiveMQ over the JMS REST bridge) is provided by the `vcell-messaging` submodule.

## Build

CMake-driven. Conan 2.x supplies `spdlog` (and `libcurl` when messaging is on), plus `cmake`/`ninja` as `tool_requires`. Note two dependencies come from CMake `FetchContent`, **not** Conan, so configure needs network access to GitHub:

- `argparse` — root `CMakeLists.txt`, pinned to `v3.2` (matching the `>=3.2 <4.0` range in `conanfile.py`). Conan also resolves argparse, but the FetchContent target is what actually gets linked.
- `googletest` — `tests/CMakeLists.txt`, pinned to `v1.17.0`.

### Prerequisites

Conan resolves the libraries, but you supply the toolchain. `conan-profiles/CI-CD/Linux-*_profile.txt` pins `compiler=clang`, `compiler.version=19`, `compiler.libcxx=libc++`, `compiler.cppstd=20`, and sets `-fuse-ld=mold` for exe and shared link flags — so a matching clang/libc++/mold must be on the box or Conan's ABI hash won't match the binaries it built. On Debian/Ubuntu:

```bash
sudo apt install clang-19 libc++-19-dev libc++abi-19-dev mold
sudo ln -sf /usr/bin/clang-19 /usr/local/bin/clang
sudo ln -sf /usr/bin/clang++-19 /usr/local/bin/clang++
```

The symlinks matter because CI drives the build as `CC=clang CXX=clang++`, and `compiler.version=19` in the profile must match what bare `clang` resolves to.

Conan itself needs a Python with the `_sqlite3` extension; if the system interpreter lacks it, `uv tool install --python-preference only-managed conan` sidesteps it with a uv-managed CPython.

`vcell-expressionparser` and `vcell-messaging` are git submodules — a plain `git clone` leaves both directories empty and CMake fails at `add_subdirectory`. Clone with `--recursive`, or in an existing checkout:

```bash
git submodule update --init --recursive
```

Canonical native build (matches `.github/workflows/cd.yml`):

```bash
CC=clang CXX=clang++ conan install . --build=missing
source build/generators/conanbuild.sh
CC=clang CXX=clang++ cmake -B build -S . -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/build/generators/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cd build && ctest -VV
```

**Where the generator files land is decided by `layout()` in `conanfile.py`, not by `--output-folder`.** `layout()` calls `cmake_layout()` and then flattens the tree to `build/` + `build/generators/` *only when* `tools.cmake.cmaketoolchain:generator=Ninja` is set in the active profile. Without that conf, `cmake_layout()` inserts the build type and you get `build/Release/generators/` instead — which is exactly how the Linux and macOS CI jobs broke once they stopped passing `--output-folder`. Every CI profile therefore sets that conf; if you build with a hand-rolled `conan profile detect` profile, either add it or adjust your paths to `build/Release/generators/`.

`source build/generators/conanbuild.sh` matters: it puts Conan's `cmake`/`ninja` on `PATH`. The `tool_requires` floor is `ninja>=1.12.1`, above what Ubuntu 24.04 ships, so the system `ninja` will not satisfy it.

Wheel build. The backend is **`uv_build`** (`pyproject.toml`), *not* scikit-build-core — it does not invoke CMake at all. The native library is built first by the CMake run above (with `-DOPTION_TARGET_PYTHON_BINDING=ON`, which sends shared libs to `build/lib/`), then hand-copied into the package before `uv` packages it. This is the sequence from `cd.yml`:

```bash
cmake -B build -S . -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/build/generators/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release -DOPTION_TARGET_PYTHON_BINDING=ON
cmake --build build --config Release
cp -r build/lib/* pyvcell_odesolver/lib/     # <-- ctypes finds the .so here at runtime
uv sync --extra build --extra test
uv run pytest
uv build --wheel -o wheelhouse
```

The wheel is then re-tagged for the manylinux platform with `uv run python -m wheel tags --remove --platform-tag manylinux_2_28_x86_64 ...`. `pyvcell_odesolver/lib/` is committed with only a `README.md` placeholder; forgetting the `cp` yields a wheel that imports but raises `OSError: Could not find the shared library` from `native_utils.py`.

The complete option set in the root `CMakeLists.txt`:

- `OPTION_TARGET_PYTHON_BINDING` (default OFF) — targets the wheel. Combining it with `OPTION_TARGET_MESSAGING=ON` is a **hard `FATAL_ERROR`** at configure time (it does *not* silently force messaging off). It leaves `CMAKE_LIBRARY_OUTPUT_DIRECTORY` at `build/lib/` instead of redirecting it to `build/bin/`, skips the install-prefix/`OPTION_EXE_DIRECTORY` setup, and adds `install(TARGETS IDAWin DESTINATION pyvcell_odesolver)`. The standalone exe **is still built** in this mode — `IDAWin/CMakeLists.txt` adds that target unconditionally and it lands in `build/bin/` either way.
- `OPTION_TARGET_MESSAGING` (default OFF) — adds `-DUSE_MESSAGING`, links libcurl, and changes the install layout (`OPTION_EXE_DIRECTORY` → `../bin`). Without it, `vcell-messaging` still builds, but the curl path is replaced by `NullCurlProxy` (the `AbstractCurlProxy` polymorphism — not `#ifdef` forests).
- `OPTION_TEST_WITH_LOCALHOST` — adds `-DTEST_WITH_LOCALHOST`; flips `tests/unit/smoke_test.cpp` to the localhost-keyed expected-output baseline.
- `OPTION_STATICALLY_LINK`, `OPTION_TARGET_DOCS`, `OPTION_EXTRA_CONFIG_INFO` — niche flags; rarely flipped.

Conan profiles for CI live under `conan-profiles/CI-CD/`, one per `(platform, arch)`; CI copies the matching one to `~/.conan2/profiles/default`. They pin `compiler.cppstd=20`, `compiler.libcxx=libc++`, add `mold` for Linux linking, and set `tools.cmake.cmaketoolchain:generator=Ninja` (load-bearing — see the `layout()` note above). For local dev, `conan profile detect --force` also works if you bump `cppstd` to `20` — but it will pick up gcc/libstdc++ and the system linker, which is a *different ABI configuration* than CI ships. Don't compare binaries across the two.

Platform notes (full recipes in `cd.yml`):
- **Linux**: CI builds inside `ghcr.io/virtualcell/fvsolver_manylinux_2_28_{x86_64,aarch64}` containers, which already carry clang/libc++/mold/cmake/ninja; the job only `pip install conan`s on top. On a bare host, install the toolchain yourself (see Prerequisites).
- **macOS** (arm64 + x86_64): Homebrew `conan` + `spdlog`. arm64 and x86_64 wheels build separately; a `MacOS-Universal` job `lipo`s the resulting `_x64`/`.dylib` files.
- **Windows**: LLVM toolchain via `llvm/actions/setup-windows`, conan + chocolatey. `OPTION_TARGET_MESSAGING` is OFF on Windows, and `requirements()` in `conanfile.py` drops the `libcurl` requirement there outright, so the workflow needs no `-o include_messaging=False`. Builds with `compiler=clang` + `clang-cl`, for which Conan derives the Visual Studio version from **`compiler.runtime_version`** (`v143`/`v144` → VS 17, `v145` → VS 18) — `tools.microsoft.msbuild:vs_version` is consulted only when `compiler=msvc`. The x86_64 and ARM64 runner images carry different VS versions, so the two profiles pin different values; see the comments in `conan-profiles/CI-CD/Windows-*_profile.txt`.
- **Docker** (`Dockerfile`): Debian trixie-slim + clang-19 + libc++-19 + mold + ninja + cmake + spdlog from apt; builds with `OPTION_TARGET_MESSAGING=ON` and `OPTION_TEST_WITH_LOCALHOST=ON`. This is the image published to ghcr.io.

## Tests

`ctest -VV` from `build/` runs four C++ tests built into `build/bin/unit_tests`, plus optionally a Python test:

- `SmokeTest.UserProvidesFilesWithoutJMS` / `SmokeTest.UserProvidesFilesWithJMS` (`tests/unit/smoke_test.cpp`) — runs CVODE against `tests/unit/resources/SimID_*_.cvodeInput` and diffs the output against `*.ida.expected` with relative tolerance `1e-7`. The "WithJMS" variant uses `taskID=2025` and selects the `886118677` (localhost) or `256118677` (vcell.cam.uchc.edu) baseline based on `TEST_WITH_LOCALHOST`. Both variants pass with messaging off — the solver no-ops the curl path under `NullCurlProxy`.
- `HelloTest.BasicAssertions` (`tests/unit/hello_test.cpp`) — gtest sanity.
- `MessageProcessingTest.MessagesAreProcessed` (`tests/unit/message_processing_test.cpp`) — exercises `MessageEventManager` directly (worker thread + lock-ordered queue), independent of `OPTION_TARGET_MESSAGING`.
- `python_smoke_test` (`tests/unit/smoke_test.py`) — pytest, exercises `pyvcell_odesolver.solve()` against the same `1489333437` resource the C++ smoke uses. Registration is **skipped at configure time** unless `${PYTHON_TEST_INTERPRETER}` can `import pyvcell_odesolver`. To enable it locally: build the wheel, install it into a venv, then re-run `cmake -DPYTHON_TEST_INTERPRETER=<venv>/bin/python .`. CI's native ctest job uses the system Python so the entry skips silently; the wheel is exercised separately by the `uv run pytest` step in `cd.yml`, after `build/lib/*` has been copied into `pyvcell_odesolver/lib/`. `pyproject.toml` sets `testpaths = ["tests"]` and `filterwarnings = ["error"]`, so any warning under pytest is a failure.

Run a single test: `ctest -VV -R <regex>`. Smoke-test inputs live next to the script and are baselined — if you intentionally change numerical output, regenerate the `.expected` files.

`ExpressionParserTest/ExpressionParserTest.cpp` exists but is not currently wired into ctest.

## Produced binaries

In native builds, `build/bin/` contains:
- `SundialsSolverStandalone_x64` — the CLI VCell invokes; `argparse`-driven, takes `<input> <output> [-tid <jobId>]`.
- `unit_tests` — gtest binary with the C++ tests above.

In Python-binding builds, `build/lib/` holds `libIDAWin.so` — the only shared object, since `vcell*`/`sundials*` build as static archives and get linked into it — alongside those `.a` files and googletest's. `cd.yml` copies the whole directory into `pyvcell_odesolver/lib/`; the archives are inert baggage, because `check_arch.py`/`native_utils.py` only consider files matching the platform's shared-library extension. There is no per-interpreter extension module, so the wheel is ABI-independent of the Python version — only the platform tag matters.

The Python API (`pyvcell_odesolver/sundials.py`, re-exported from `__init__.py`):
- `version() -> str` — the solver version string, unwrapped from the `ReturnValue` model's `.message`.
- `solve(input_file_path, output_file_path, task_id=-1) -> int` — `task_id=-1` skips messaging; positive values are JMS job IDs. Returns **0 on success** (it inverts `ReturnValue.success` to mimic a process exit code).

`pyvcell_odesolver/_internal/` holds the ctypes layer: `native_utils.py` (`VCellNativeLibraryLoader` — scans `pyvcell_odesolver/lib/`, loads each candidate and keeps the first whose `version_ctypes()` contains `"VCell ODE solver (CVODE/IDA) v"`), `native_calls.py` (`ODENativeCalls`, returning pydantic `ReturnValue` models — hence the `pydantic>=2.13.4` runtime dep), and `check_arch.py` (filters candidate libs by architecture).

## Architecture

Top-level subdirectories and how they fit:

- **`vcell-expressionparser/`** (submodule, target `vcellexpressionparser`) — math expression parser (AST `Node`s, `Expression`, `SymbolTable`, `StackMachine`). Used by `IDAWin` to evaluate user-supplied rate laws, initial conditions, and event/discontinuity expressions. Headers live under `include/`, sources under `src/`. Requires C++20 — the AST nodes build error messages with `std::format`.
- **`sundials/`** (vendored, modified) — CVODE + IDA + nvec_ser. A thin top-level `add_subdirectory` aggregates `sundials_cvode`/`sundials_ida`/`sundials_nvecserial`/`sundials_lib` into the `sundials` interface target.
- **`IDAWin/`** — the solver layer. Decomposed (since the stabilization branch) into:
  - `VCellSolver.h` — abstract base (`configureFromInput`, `solve`).
  - `VCellSolverInput.{h,cpp}` — `VCellSolverInputBreakdown` struct-of-structs holding `ModelSettings`, `TimeCourseSettings`, `SteadyStateSettings`, `DiscontinuitiesSettings`, `EventSettings`.
  - `VCellSolverFactory.{h,cpp}` — parses the `.cvodeInput` file into the breakdown and produces the right `VCellSolver` subclass.
  - `VCellSundialsSolver.{h,cpp}` — common sundials machinery (events, discontinuities, output, stop-checking).
  - `VCellCVodeSolver` / `VCellIDASolver` — concrete subclasses for ODE and DAE.
  - `SundialsSolverInterface.{h,cpp}` — the `solve(input, output, tid)` entry shared by the CLI and Python. Also declares the `IDAWIN_API` visibility macro and the `extern "C"` ctypes shims `version_ctypes()` / `solve_ctypes()` that the wheel binds to.
  - `SundialsSolverStandalone.cpp` — `argparse`-based `main` that calls `solve()`.

  `IDAWin` builds SHARED by default; `OPTION_STATICALLY_LINK` switches it to STATIC (or to a SHARED lib with statically-linked deps when combined with `OPTION_TARGET_PYTHON_BINDING`).
- **`vcell-messaging/`** (submodule, target `vcellmessaging`) — progress messaging. Decomposed into:
  - `SimulationMessaging` — the singleton facade.
  - `MessageEventManager` — owns the worker `std::thread` and the event queue; lock ordering documented in the header (**queue mutex *before* the `stopRequested` mutex** — this is the reverse of the pre-submodule in-tree copy, which deadlocked). Shutdown joins the worker rather than waiting for the queue to drain: an empty queue is not the same as "all work finished", since the worker pops under the lock but sends after releasing it.
  - `CurlProxyClasses` — `AbstractCurlProxy` / `NullCurlProxy` (always built) / `CurlProxy` (only under `USE_MESSAGING`); polymorphism replaces the old `#ifdef` forests.
  - `JobEventStatus` — `JobEvent::Status` namespaced enum + `toString`.
  - `WorkerEvent` — the queue payload.
- **`pyvcell_odesolver/`** — the Python package (pure Python; see "Produced binaries" above). `lib/` is the drop point for the native shared libraries and ships with only a `README.md` placeholder in git.
- **`tests/`** — gtest C++ tests, pytest Python tests, and resource files. `RESOURCE_DIR` is passed to the C++ tests via `target_compile_definitions`; the Python test resolves resources via `Path(__file__).parent / "resources"`.
- **`conan-profiles/CI-CD/`** — per-platform Conan profiles; CI copies the matching one to `~/.conan2/profiles/default`.
- **`cmake/modules/`** — `GetGitRevisionDescription.cmake` (used to stamp `g_GIT_DESCRIBE` into `vcell-messaging/src/GitDescribe.cpp`, which the version string surfaces).

Public headers for shared libraries live in their own dir and are pulled in via `target_include_directories(... PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})` (or `include/` for the two submodules and `sundials`) — there are no installed include dirs at configure time.

## Repo conventions worth knowing

- Executables always go to `build/bin/`. Shared libraries go to `build/bin/` too in native builds, but stay in `build/lib/` when `OPTION_TARGET_PYTHON_BINDING=ON` — which is why both the native release step and the wheel step in `cd.yml` start with a `cp -r build/lib/* <dest>`.
- `cmake-build-debug/` is the CLion out-of-source build dir; `build/` is the canonical CI/Docker dir. Both are gitignored.
- `sundials/` is vendored; prefer surgical edits over upstream re-syncs.
- Shared-library path fixup for releases is done post-build by `.github/scripts/install_name_tool_macos.sh` (macOS) and the `ldd | cp` loops in `cd.yml` (Linux/Windows). Don't introduce absolute `rpath`s into CMake.
- `OPTION_TARGET_PYTHON_BINDING=ON` and `OPTION_TARGET_MESSAGING=ON` are mutually exclusive — configure aborts with `FATAL_ERROR`. The Python wheel never carries a curl dependency.
- `pyproject.toml` is the source of truth for the Python package: build backend `uv_build`, `requires-python = ">=3.10"`, runtime dep `pydantic`, extras `test` (pytest) and `build` (build, wheel). Ruff lint config lives here too.
- Setting `-DOPTION_EXTRA_CONFIG_INFO=ON` dumps every CMake variable at the end of configure — handy for debugging Conan-generated targets.
