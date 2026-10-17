# Tooling

## C++ Compiler

### C++ Standard

The minimal C++ standard officially supported at BBP is C++17, to be reviewed annually based on compiler support.
An open source project may support a standard as low as C++14 if this is a requirement for its community.

### Compiler Versions Supporting C++17

* GCC 7.4
* CLang 8
* Intel 19.1.0

### Compilation flags

Here is the list of recommended compilation flags you may use during the development phase in order to spot as much errors as possible. For convenience, you may use the CMake function `bob_begin_cxx_flags` provided by this CMake project to set the compilation flags accordingly.

* Clang: `-Werror -Weverything`. And then disable those you dislike:
  * `-Wno-disabled-macro-expansion`
  * `-Wno-documentation-unknown-command`
  * `-Wno-padded`
  * `-Wno-unused-member-function`
* GCC:
  * `-Wall`
  * `-Wcast-align`
  * `-Wconversion`
  * `-Wdouble-promotion`
  * `-Werror`
  * `-Wextra`
  * `-Wformat=2`
  * `-Wnon-virtual-dtor`
  * `-Wold-style-cast`
  * `-Woverloaded-virtual`
  * `-Wshadow`
  * `-Wsign-conversion`
  * `-Wunused`
  * `-Wuseless-cast`
* GCC 6 and greater:
  * `-Wduplicated-cond`
  * `-Wmisleading-indentation`
  * `-Wnull-dereference`
* GCC 7 and greater:
  * `-Wlogical-op`
  * `-Wduplicated-branches`
  * `-Wrestrict`
* GCC 8 and greater:
  * `-Wclass-memaccess`
  * `-Wstringop-truncation`

Continuous integration should compile your code with as many compilers as possible to get best feedback.

## Code formatting

C++ code can be formatted to meet the formatting conventions with
[ClangFormat](https://releases.llvm.org/9.0.0/tools/clang/docs/ClangFormat.html) utility.
The ClangFormat configuration file to comply to these conventions can be found [here](./.clang-format).
ClangFormat 9 is recommended but this coding conventions also support versions 7 and 8.

It is recommended to use the CMake project provided by this repository to automate the source code formatting.

## Static Analysis

You may use C++ linter tools to identify well known design mistakes like ClangTidy. A generic
configuration file can be found
[here](./.clang-tidy)
Only ClangTidy 7 is supported, the LLVM stable
version by the time of the writing of this document.

It is recommended to use the CMake project provided by this repository to automate the static analysis of the source code.

## GitHook

[pre-commit](https://pre-commit.com/) allows you to identify common mistakes before committing
changes.

## External C++ Libraries

### As git modules or directly bundled

The libraries below can be used directly in your C++ project, either as git modules or directly bundled as most of them provides single-header releases.

* [CLI11](https://github.com/CLIUtils/CLI11): a header-only command line parser for C++11
* [cereal](https://github.com/USCiLab/cereal)
  A C++ header-only library for serialization
* [{fmt}](https://github.com/fmtlib/fmt) a Python like formatting library.
* [google-benchmark](https://github.com/google/benchmark) A microbenchmark support library
* [spdlog](https://github.com/gabime/spdlog)
  Fast C++ logging library
* [pybind11](https://github.com/pybind/pybind11)
  Lightweight header-only library to create Python bindings of existing C++ code.
* [nlohmann/json](https://github.com/nlohmann/json) JSON for Modern C++
* [gsl-lite](https://github.com/martinmoene/gsl-lite) A single-file header-only version
  of ISO C++ Guideline Support Library (GSL)

### As external dependencies

* [pugixml](https://pugixml.org): Light-weight, simple and fast XML parser for C++ with XPath support. A good replacement for libxml2.
