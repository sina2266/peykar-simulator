from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class VCellHy3SRecipe(ConanFile):
    name = "vcell-hy3s"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "include_messaging": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        "include_messaging": True,
    }

    def layout(self):
        cmake_layout(self)
        # Keep the CI build tree flat when Ninja is selected, matching the other
        # solver repos; cmake_layout() would otherwise insert the build type.
        if self.conf.get("tools.cmake.cmaketoolchain:generator") == "Ninja":
            self.folders.build = "build"
            self.folders.generators = "build/generators"

    def validate(self):
        # Windows builds with Intel Fortran (ifx) driving MSVC, which is what
        # this code was originally built with -- MSVC compiles the C and C++,
        # ifx the Fortran, and ifx hands the link off to MSVC's link.exe. No
        # MSYS2 or MinGW is involved. Conan has no Fortran setting to check
        # here, so the compiler pairing is asserted by CMakeLists.txt instead.
        #
        # Messaging is off on Windows, which leaves no external dependency at
        # all: NetCDF is vendored and libcurl is only needed for messaging.
        if self.settings.os == "Windows" and self.options.include_messaging:
            raise ValueError(
                "Messaging is not built on Windows. Configure with "
                "-o include_messaging=False."
            )

    def requirements(self):
        # NetCDF is deliberately NOT a Conan requirement. Hy3S does `USE netcdf`,
        # and Conan Center ships the C library only -- no Fortran module, and no
        # netcdf-fortran recipe at all. The Fortran bindings are vendored under
        # netcdf/ instead; see README.md.
        #
        # That leaves libcurl as the only external dependency, and only when
        # messaging is on.
        if self.options.include_messaging:
            self.requires("libcurl/[<9.0]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.20]")
        self.tool_requires("ninja/[>=1.12.1]")

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
            "OPTION_TARGET_MESSAGING": "ON" if self.options.include_messaging else "OFF",
        })
        cmake.build()
