# NEURON Team Development Guidelines

**IMPORTANT NOTE**: this is a fork of the archived [Blue Brain Project HPC Team coding conventions](https://github.com/BlueBrain/hpc-coding-conventions), adapted for use in NEURON.

This document describes both C++ development guidelines adopted by
the NEURON team, and the tools and processes required to
ensure they are properly followed over time.

## Documentation

Development Guidelines are split in the following sections:
* [Tooling](./cpp/Tooling.md)
* [Development Lifecycle](./cpp/DevelopmentLifecycle.md)
* [Code Formatting](./cpp/formatting/README.md)
* [Naming Conventions](./cpp/NamingConventions.md)
* [Code Documentation](./cpp/Documentation.md)
* Best Practices
* Python bindings

## Status

This project in currently under development, and shall not provide the features
its documentation pretends. Here is a raw summary of the status:

| Feature               | Definition         | Documentation      | Integration         | Adoption (>10 proj) |
| --------------------- | ------------------ | ------------------ | ------------------  | ------------------- |
| ClangFormat           | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:  | WIP                 |
| ClangTidy             | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:  | WIP                 |
| Naming Conventions    | :heavy_check_mark: | :heavy_check_mark: | :no_entry_sign: N/A | :no_entry_sign: N/A |
| Writing Documentation | :heavy_check_mark: | :heavy_check_mark: |                     |                     |
| Project template      | WIP                |                    |                     |                     |
| Good Practices        |                    |                    |                     |                     |
| Memory Check          |                    |                    |                     |                     |
| UT Code Coverage      |                    |                    |                     |                     |

## CMake Project

This directory provides a CMake project that allows one to use the tools and the processes
described in this document. This project requires CMake version 3.10 or higher.

### Requirements

This CMake project expects for the following utilities to be available:
* [Python 3.5 or higher](https://python.org)
* [ClangFormat 9](https://releases.llvm.org/9.0.0/tools/clang/docs/ClangFormat.html)
* [ClangTidy 7](https://releases.llvm.org/7.0.0/tools/clang/tools/extra/docs/clang-tidy/index.html)
* [cmake-format](https://github.com/cheshirekow/cmake_format) Python package version 0.6
* [pre-commit](https://pre-commit.com/) Python package
* [git](https://git-scm.com/) version control system 2.17 or higher.

Optionally, it will also look for:
* [valgrind](http://valgrind.org/) memory checker
* code coverage utilities like gcov, lcov, or gcov

### Installation

You can import this CMake project into your Git repository using a git submodule:
```
git submodule add https://github.com/neuronsimulator/coding-conventions.git
git submodule update --init --recursive
```

Note: if your project already has a `cmake` sub-directory, it is recommended to create the
submodule in this directory instead of top-level.

Then simply add the following line in the top `CMakeLists.txt`, after your project
declaration:
```
project(mylib CXX)
# [...]
add_subdirectory(hpc-coding-conventions/cpp)
```

After cloning or updating this git submodule, run CMake to take benefits of the latest changes.
This will setup or update git [pre-commit](https://pre-commit.com) hooks of this repository.

CMake variables defined by this project are prefixed by `${PROJECT_NAME}` by default.
The CMake variable `CODING_CONV_PREFIX` allows to specify another prefix. It must be defined
before including this CMake project, for instance:
```cmake
project(mylib CXX)
# [...]
set(CODING_CONV_PREFIX Foo)
add_subdirectory(hpc-coding-conventions/cpp)
```

### Usage

#### Code Formatting

Create file `.bbp-project.yaml` at the root of your project and enable the formatting tools,
you want to enable, for instance:

```yaml
tools:
  ClangFormat:
    enable: True
  CMakeFormat:
    enable: True
```

You can then use the `bin/format` utility.

#### Version deduction

`bin/format` relies on `PATH` environment variable to locate the proper tools.
You can override the default required versions specified in `bbp-project.yaml`:

```yaml
tools:
  ClangFormat:
    enable: True
    version: ~=15.0.0
```

It is also possible to override the path to a tool in the YAML config file:
```yaml
tools:
  CMakeFormat:
    path: /path/to/bin/cmake-format
```

##### Usage

* To format the entire codebase: `/path/to/hpc-coding-conventions/bin/format`
* To check the formatting: `/path/to/hpc-coding-conventions/bin/format -n`
* To format the CMake files only: `/path/to/hpc-coding-conventions/bin/format --lang CMake`
* To check the formatting of C++ files in a specific locations:
    `/path/to/hpc-coding-conventions/bin/format -n --lang c++ src/path1/ /src/path2/foo.cpp`


##### Advanced configuration

You can override the default file filters of a tool, for instance:

```yaml
tools:
  ClangFormat:
    include:
      match: &cpp_patterns
      - src/steps/.*\.((h)|(cpp)|(hpp))
      - test/unit/.*\.((h)|(cpp)|(hpp))
    path: /foo/bin/clang-format
  ClangTidy:
    include:
      match: *cpp_patterns
```

##### Custom ClangFormat configuration

These coding conventions come with a predefined configuration of ClangFormat that
is by default copied in the top directory of the project.

It is recommended to not add this `.clang-format` to git so that it is fully
driven by this project. It will get updated along with the evolution of these
guidelines. Thus it is recommended to inform git to ignore this file by adding
it to the top `.gitignore`.

A project can however override the predefined configuration of ClangFormat
in two ways:
1. Create a `.clang-format.changes` containing only the required modifications.
1. Add `.clang-format` to the git repository. This project will never try
to modify it.

##### Custom CMakeFormat configuration

Like ClangFormat, these coding conventions already provide a CMakeFormat
configuration that the user can customize with a file named
`.cmake-format.changes.yaml` placed at the project's root directory.
This file can be used to specify the signature of owned CMake functions
and macros, for instance:

```yaml
additional_commands:
  add_mpi_test:
    kwargs:
      NAME: 1
      NUM_PROCS: 1
      COMMAND: '*'
```

will allow CMakeFormat to properly format functions calls like below:

```cmake
add_mpi_test(
  NAME OpSplitOmega_h_2D
  NUM_PROCS 2
  COMMAND $<TARGET_FILE:OpSplitOmega_h> --num-iterations 100 square.msh)
```

##### Continuous Integration

Define `${PROJECT}_TEST_FORMATTING:BOOL` CMake variable to enforce formatting during
the `test` make target.

#### Static Analysis

To activate static analysis of C++ files with clang-tidy within CMake, enable
the CMake variable `${PROJECT}_STATIC_ANALYSIS` where `${PROJECT}` is the name
given to the CMake `project` function.
For instance, given a project `foo`:
`cmake -DFoo_STATIC_ANALYSIS:BOOL=ON <path>`
Whenever a C++ file is compiled by CMake, clang-tidy will be called.

You can also use utility: `bin/clang-tidy`

##### Usage

This will provide a `clang-tidy` *make* target that will perform static analysis
of all C++ files. Target fails as soon as one defect is detected among the files.

It will also activate static analysis report during the compilation phase.

##### Advanced configuration

The following CMake cache variables can be used to customize the static analysis
of the code:

* `${PROJECT}_ClangTidy_DEPENDENCIES`: list of CMake targets to build before
  check C/C++ code. Default value is `""`

These variables are meant to be overridden inside your CMake project.
They are CMake _CACHE_ variables whose value must be forced
**before including this CMake project**.

##### Custom ClangTidy configuration

These coding conventions come with a `.clang-tidy` file providing a predefined
list of ClangTidy checks. By default, this file is copied in the top directory
of the project.

It is recommended to not add this `.clang-tidy` to git so that it is fully
driven by this project. It will get updated along with the evolution of these
guidelines. Thus it is recommended to inform git to ignore this file by adding
it to the top `.gitignore`.

A project can however override the predefined configuration of ClangFormat
in two ways:
1. Create a `.clang-tidy.changes` containing only the required modifications.
1. Add `.clang-tidy` to the git repository. This project will never try
to modify it.

#### Pre-Commit utility

Enable CMake option `${PROJECT}_GIT_HOOKS` to enable automatic checks
before committing or pushing with git. the git operation will fail if
one of the registered checks fails.

The following checks are available:
* `check-clang-format`: check C++ formatting
* `check-cmake-format`: check CMake formatting
* `clang-tidy`: execute static-analysis
* `courtesy-msg`: print a courtesy message to the console.
  This check never fails. The default message is a reminder to test and
  format the changes when pushing a contribution with git.
  A project can overwrite the message displayed by adding the CMake template
  named `.git-push-message.cmake.in` at the root of the project directory.

To enable these checks, use CMake variables `${PROJECT}_GIT_COMMIT_HOOKS` and
`${PROJECT}_GIT_PUSH_HOOKS` to specify which checks should be executed for
each specific git operation. For instance:

`cmake -Dfoo_GIT_COMMIT_HOOKS=clang-tidy \
       -Dfoo_GIT_PUSH_HOOKS=check-clang-format,courtesy-msg <path>`

This feature requires the `pre-commit` utility.

#### Bob

`bob.cmake` is a CMake utility file part of hpc-coding-conventions that provides
a set of convenient macros and functions used to:
* specify your project options and dependencies
* specify the proper compilation flags
* install the proper CMake export flags so that your project can be
  loaded by another project with the `find_package` CMake function.

##### Compilation flags

By default, CMake relies on the `CMAKE_BUILD_TYPE` variable to set the proper
compilation flags. Because _bob_ is now taking care of it, you must configure
your project with `CMAKE_BUILD_TYPE` empty.

_bob_ sets the compilation flags according to a set of CMake variables:
* `${PROJECT_NAME}_CXX_OPTIMIZE:BOOL`: Compile C++ with optimization (default is ON)
* `${PROJECT_NAME}_CXX_SYMBOLS:BOOL`: Compile C++ with debug symbols (default is ON)
* `${PROJECT_NAME}_CXX_WARNINGS:BOOL`: Compile C++ with warnings" (default is ON)
* `${PROJECT_NAME}_EXTRA_CXX_FLAGS:STRING`: Additional C++ compilation flags
* `${PROJECT_NAME}_CXX_FLAGS:STRING`: bypass variables above and use the specified
  compilation flags. `CMAKE_BUILD_TYPE` is ignored.
* `${PROJECT_NAME}_NORMAL_CXX_FLAGS:BOOL`: Allow `CMAKE_CXX_FLAGS` to follow _normal_ CMake behavior
  and bypass all variables above.

Default `CMAKE_CXX_FLAGS` variable value is taken into account.

##### Integration

The top-level CMakelists.txt of your project may look like:

```cmake
cmake_minimum_required(VERSION 3.10)
project(HelloWorld VERSION 1.0.0 LANGUAGES CXX)
add_subdirectory(hpc-coding-conventions/cpp)

bob_begin_package()
bob_begin_cxx_flags()
bob_cxx17_flags()
# specify custom compilation flags
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_FLAGS "${CMAKE_FLAGS} ${OpenMP_FLAGS}")
else()
  message(WARNING "OpenMP support is disabled because it could not be found.")
endif()
bob_end_cxx_flags()

# specify your targets:
add_library(...)
add_executable(...)

bob_end_package()
```

#### Embedded third parties

External libraries required to build or test your C++ project can be either
directly added to the git repository or as a git submodule. The standard
root location for this kind of files is the `3rdparty/` directory but can be
overriden with the `${PROJECT_NAME}_3RDPARTY_DIR` CMake variable.

Adding single-file/header-only C++ libraries directly to the git repository
of your project is acceptable in general, like catch2 or the JSON library
of Niels Lohmann for instance.

More significant dependencies should be considered as pure external
dependencies. But it can also be very convenient to have them as git
submodules, and be able to switch between the two.

This project provides helper functions to deal with these dependencies:

###### cpp_cc_git_submodule

````cmake
#
# cpp_cc_git_submodule(source_dir
#                      [DISABLED]
#                      SUBDIR path
#                      [BUILD] [<arguments>]
#                      [PACKAGE] [<arguments>]
#                      GIT_ARGS [<arguments>])
#
# Add a CMake option in the cache to control whether the
# submodule is used or not (default ON). The option is named after the source
# directory passed in first argument, for instance:
#   cpp_cc_git_submodule(src/eigen)
# adds the following CMake cached option:
#  ${PROJECT_NAME}_3RDPARTY_USE_SRC_EIGEN:BOOL=ON
#
# If enabled, then the submodule is fetched if missing in the working copy.
#
# If the DISABLED argument is provided, then the default value for the CMake
# option is OFF.
#
# If the BUILD argument is provided then the directory is added to the build
# through the add_subdirectory CMake function. Arguments following the BUILD
# arguments are passed to the add_subdirectory function call.
#
# The optional SUBDIR argument is used by the BUILD argument to determine
# the path to the directory added to the build. The path specified is relative
# to the path to the git submodule.
#
# If the PACKAGE argument is provided and the CMake option to determine whether
# the git submodule should be used or not is FALSE, then a call to the find_package
# function is made with the arguments specified to the PACKAGE option.
#
# Default options passed to the `git submodule update` command are
# `--init --recursive --depth 1` to perform a shallow clone of the submodule.
# If the GIT_ARGS argument is provided, then its value supersedes the default options.
#
````


## Contributing

Should you want to contribute to the naming conventions,
please refer to the dedicated [contributing document](./cpp/formatting/CONTRIBUTING.md) first.

## Funding

The development of this software was supported by funding to the Laboratory of Neural Microcircuitry (LNMC), a research center of the École polytechnique fédérale de Lausanne (EPFL), with funding from European Union's Horizon Europe Grant no. 101147319 (EBRAINS 2.0: A Research Infrastructure to Advance Neuroscience and Brain Health).

## Past Funding

The development of this software was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government's ETH Board of the Swiss Federal Institutes of Technology.

Copyright © 2019-2024 Blue Brain Project/EPFL
Copyright © 2025-2026 EPFL
