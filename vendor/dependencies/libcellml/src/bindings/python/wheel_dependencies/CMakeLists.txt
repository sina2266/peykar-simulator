cmake_minimum_required(VERSION 3.21)

project(prepare_dependencies)

#include(FetchContent)
include(ExternalProject)

ExternalProject_Add(zlib_library
  GIT_REPOSITORY "https://github.com/CMLibs-Dependencies/zlib"
  GIT_TAG "e4594ed"
  CMAKE_GENERATOR Ninja
  CMAKE_ARGS -DBUILD_SHARED_LIBS=FALSE -DCMAKE_BUILD_TYPE=Release
)

set(ZLIB_DIR_DEFINE "-DZLIB_DIR=C:/Program Files (x86)/ZLIB/ZLIB-1.2.12/CMake/")
ExternalProject_Add(libxml2_library
  GIT_REPOSITORY "https://github.com/CMLibs-Dependencies/libxml2"
  GIT_TAG "1f98b29"
  CMAKE_GENERATOR Ninja
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release \\"-DZLIB_DIR=C:/Program Files (x86)/ZLIB/ZLIB-1.2.12/CMake/\\" -DBUILD_SHARED_LIBS=OFF -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF
  DEPENDS zlib_library
)
