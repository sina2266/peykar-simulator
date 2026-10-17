#include <pybind11/pybind11.h>

#include <VCELL/SolverMain.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
    m.doc() = R"pbdoc(
        VCell FiniteVolume solver
        -------------------------

        .. currentmodule:: pyvcell_fvsolver

        .. autosummary::
           :toctree: _generate

           version
           solve
    )pbdoc";

    m.def("version", &version, R"pbdoc(
        version of build

        version string of build using git hash
    )pbdoc");

    m.def("solve", &solve, R"pbdoc(
        solve the ODE

        The inputFilename expects a .fvinput file, the outputDir will be created as needed.
    )pbdoc",
        py::arg("fvInputFilename"), py::arg("vcgInputFilename"), py::arg("outputDir"));

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
