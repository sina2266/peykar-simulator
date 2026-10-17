from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout

class VCellODERecipe(ConanFile):
    name = "vcell-ode"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {"shared": [True, False],
               "fPIC": [True, False],
               "with_fmt": [True, False],
               "include_messaging": [True, False],
               "generate_docs": [True, False]}

    default_options = {"shared": False,
                       "fPIC": True,
                       "with_fmt": True,
                       "include_messaging": True,
                       "generate_docs": False}

    def layout(self):
        cmake_layout(self)
        # Keep the CI build tree flat when Ninja is selected. The workflow and
        # packaging steps use build/{bin,lib,generators}; cmake_layout() would
        # otherwise insert build_type (for example, build/Release) for Ninja.
        if self.conf.get("tools.cmake.cmaketoolchain:generator") == "Ninja":
            self.folders.build = "build"
            self.folders.generators = "build/generators"

    def validate(self):
        check_min_cppstd(self, "17")

    def build(self):
        cmake = CMake(self)
        messaging_enabled = (
            bool(self.options.include_messaging) and self.settings.os != "Windows"
        )
        cmake.configure(variables={
            "OPTION_TARGET_MESSAGING": "ON" if messaging_enabled else "OFF",
            "OPTION_TARGET_DOCS": "ON" if self.options.generate_docs else "OFF",
        })
        cmake.build()

    def requirements(self):
        self.requires("argparse/[>=3.2 <4.0]")
        self.requires("spdlog/[>=1.16.0 <2.0]")
        if self.options.include_messaging and self.settings.os != "Windows":
            self.requires("libcurl/[<9.0]")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.13]")
        self.tool_requires("ninja/[>=1.12.1]")
        # if self.settings.os == "Linux":
        #     self.tool_requires("llvm-core/19.1.7")

    def config_options(self):
        # if self.settings.os == "Linux":
        #     self.settings.compiler.libcxx = "libc++"
        if self.settings.os == "Windows":
            del self.options.fPIC


    def configure(self):
        if self.options.shared:
            # If os=Windows, fPIC will have been removed in config_options()
            # use rm_safe to avoid double delete errors
            self.options.rm_safe("fPIC")
