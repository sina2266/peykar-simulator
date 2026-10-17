from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout


class VCellChomboRecipe(ConanFile):
    name = "vcell-chombo"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "include_messaging": [True, False],
        "with_2d": [True, False],
        "with_3d": [True, False],
        "parallel": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        "include_messaging": True,
        "with_2d": True,
        "with_3d": True,
        "parallel": False,
        # vcell-messaging only ever speaks plain http:// to the broker's REST
        # bridge (vcell-messaging/src/CurlProxyClasses.cpp), so libcurl needs
        # no TLS. Leaving it out drops OpenSSL from the dependency tree -- the
        # longest build on macOS, where everything compiles from source -- and
        # leaves the release binaries with no CA-bundle path to get wrong.
        "libcurl/*:with_ssl": False,
    }

    def layout(self):
        cmake_layout(self)
        # Keep the CI build tree flat when Ninja is selected. The workflow and
        # packaging steps use build/{bin,lib,generators}; cmake_layout() would
        # otherwise insert build_type (for example, build/Release) for Ninja.
        if self.conf.get("tools.cmake.cmaketoolchain:generator") == "Ninja":
            self.folders.build = "build"
            self.folders.generators = "build/generators"

    def validate(self):
        # vcell-expressionparser and vcell-messaging both use std::format.
        check_min_cppstd(self, "20")
        if self.settings.os == "Windows":
            raise ValueError(
                "vcell-chombo does not build on Windows: Chombo's build system needs "
                "GNU make, perl and a Unix shell."
            )

    def requirements(self):
        # Chombo writes its checkpoints through the HDF5 C API, and so does
        # VCellChombo's own post-processing writer. Neither uses the C++ API.
        self.requires("hdf5/[>=1.14 <2.0]")
        self.requires("zlib/[>=1.3 <2.0]")
        # SimTool rolls each timestep's .sim.hdf5 into a .hdf5.zip that VCell
        # serves to the client.
        self.requires("libzip/[>=1.10 <2.0]")
        if self.options.include_messaging:
            self.requires("libcurl/[<9.0]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.20]")
        self.tool_requires("ninja/[>=1.12.1]")

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
            "OPTION_TARGET_MESSAGING": "ON" if self.options.include_messaging else "OFF",
            "OPTION_TARGET_CHOMBO2D_SOLVER": "ON" if self.options.with_2d else "OFF",
            "OPTION_TARGET_CHOMBO3D_SOLVER": "ON" if self.options.with_3d else "OFF",
            "OPTION_TARGET_PARALLEL": "ON" if self.options.parallel else "OFF",
        })
        cmake.build()
