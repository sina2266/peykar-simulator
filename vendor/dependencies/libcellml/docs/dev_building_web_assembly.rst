
===========================================
Building *libCellML* for use in the Browser
===========================================

To build *libCellML* for the browser, we use the emscripten toolchain to create web assembly binaries.
Here, we will go through the process of building web assembly binaries on macOS.
For other operating systems, modifications to these instructions will have to be made.
Any modifications required are expected to be self evident – with this in mind these instructions should be universally applicable.

Preparation
===========

To build *libCellML* for the browser, we will need the following tools ready for use.

- Emscripten
- Compiler toolchain
- CMake (at least version 3.14)
- Git

Building
========

*libCellML* requires two dependencies to be built before *libCellML* itself can be built.
We will clone and build those first and then move on to building *libCellML*.

Dependencies
------------

Start by creating a directory under which all our files will live and make it our current directory for subsequent commands::

  mkdir emscripten
  cd emscripten

Now, we can clone our dependencies (we suggest using these exact repositories other repositories may work but these repositories are known to work)::

  git clone https://github.com/CMLibs-Dependencies/zlib.git -b v1.2.12
  git clone https://github.com/CMLibs-Dependencies/libxml2.git -b v2.9.10

We will create two directories to hold the build files::

  mkdir build-zlib-release
  mkdir build-libxml2-release

Then, we will configure, build, and install *zlib* as this is required by *libxml2*.
We will install *zlib* under our emscripten working directory and **not** into the system directories::

  emcmake cmake -S zlib -B build-zlib-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=usr/local -DBUILD_SHARED_LIBS=OFF

Here, we have configured the project using the emscripten wrapper script :code:`emcmake`.
This script sets up environment variables for us to make building with the emscripten compilers easy.
We also configure the *zlib* library to be built in **Release** mode and set the install prefix relative to the **current** directory.
However, this is not an issue for us as the **source** and **build** directories are sibling directories.

To build and install the *zlib* library, we simply issue these commands::

  cd build-zlib-release
  make
  make install

Next, we follow a similar procedure for building *libxml2*.
Start by making the current directory the parent of the zlib build directory::

  cd ..

Then, as before, configure the library::

  emcmake cmake -S libxml2 -B build-libxml2-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=usr/local -DZLIB_DIR=../usr/local/lib/cmake/ZLIB-1.2.12/ -DBUILD_SHARED_LIBS=OFF -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF

There are seven additions to the configure command this time:

1. We set :code:`ZLIB_DIR` to enable the configuration to find our *zlib* library we just installed, :code:`-DZLIB_DIR=../usr/local/lib/cmake/ZLIB-1.2.12/`.
2. We turn off building a shared *libxml2* library, -DBUILD_SHARED_LIBS=OFF.
3. We turn off building *libxml2* with iconv, -DLIBXML2_WITH_ICONV=OFF.
4. We turn off building *libxml2* with lzma compression, -DLIBXML2_WITH_LZMA=OFF.
5. We turn off building *libxml2* with Python, -DLIBXML2_WITH_PYTHON.
6. We turn off building *libxml2* with tests, -DLIBXML2_WITH_TESTS=OFF; and
7. We turn off building *libxml2* with programs, -DLIBXML2_WITH_PROGRAMS=OFF.

As a general note, it is better to specify the variable :code:`ZLIB_DIR` as an absolute path and not a relative path as we have done here.
Feel free to use the absolute path when you are configuring *libxml2*.

Then, as before, we simply issue the build and install commands::

  cd build-libxml2-release
  make
  make install

That is all that is required for preparing the dependencies ready for *libCellML*.

libCellML
---------

Following a similar approach to building the dependencies, we can now configure and build *libCellML*.
We don't install *libCellML* when building for web assembly, so there is no install command in the instructions.

To get started, we will make our current directory the :code:`emscripten` directory we made at the start.
Assuming we are in the directory where we left off from preparing the dependencies, we simply change to the parent directory::

  cd ..

If your current directory is not where the dependency instructions left off then modify the command as appropriate for your situation.

Now, let's get the source code using :code:`git clone`::

  git clone https://github.com/cellml/libcellml.git

Now, we will create a build directory::

  mkdir build-libcellml-release

We can now configure the library::

  emcmake cmake -S libcellml -B build-libcellml-release -DLibXml2_DIR=../usr/local/lib/cmake/libxml2-2.9.10/ -DBUILD_TYPE=Release

As with the earlier note, it is best if :code:`LibXml2_DIR` is an absolute path and not a relative path as shown here.

The last thing we have to do is build the library::

  cd build-libcellml-release
  make

That should be everything done.

Outcome
=======

Given that all of the above worked you should have the following four files :code:`libcellml.js`, :code:`libcellml.wasm`, :code:`libcellml.common.js`, and :code:`libcellml.common.wasm` in the :code:`src/bindings/javascript/` relative to the *libCellML* build directory where we left off above.

Test
====

We can test the *libCellML* WebAssembly to make sure everything works.
From the *libCellML* build directory :code:`build-libcellml-release`, we can run the test command::

  ctest -V

The first time the tests are run the node modules required for running the tests are installed.
This can take a while to complete, please be patient while the node modules install.
Subsequent running of the tests will not require node modules to be installed and the tests will be run in a much more timely manner.
If all the tests passed, you should see::

  100% tests passed, 0 tests failed out of 1

A few lines up from the bottom of the output from the above command.
