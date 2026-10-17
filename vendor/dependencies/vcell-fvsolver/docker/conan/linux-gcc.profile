# Conan profile for the Linux release build: the manylinux_2_28 image's gcc-toolset-14
# (gcc/g++/gfortran 14 against glibc 2.28), static dependencies.
[settings]
os=Linux
arch={{ {"x86_64": "x86_64", "aarch64": "armv8"}.get(platform.machine()) }}
build_type=Release
compiler=gcc
compiler.version=14
compiler.libcxx=libstdc++11
compiler.cppstd=17

[buildenv]
CC=gcc
CXX=g++
FC=gfortran

[conf]
tools.build:compiler_executables={"c": "gcc", "cpp": "g++", "fortran": "gfortran"}
