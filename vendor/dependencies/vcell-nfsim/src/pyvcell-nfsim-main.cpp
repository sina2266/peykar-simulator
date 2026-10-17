#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <NFsim.hh>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

// Wrapper function that handles the full simulation lifecycle
void runSimulation(const std::map<std::string, std::string>& argMap, bool verbose) {
    System* s = initSystemFromFlags(argMap, verbose);
    if (s != nullptr) {
        runFromArgs(s, argMap, verbose);
        delete s;
    } else {
        throw std::runtime_error("Failed to initialize system from arguments");
    }
}

PYBIND11_MODULE(_core, m) {
    m.doc() = R"pbdoc(
        VCell NFsim solver Python bindings
        -----------------------------------

        .. currentmodule:: pyvcell_nfsim

        .. autosummary::
           :toctree: _generate

           run_simulation
    )pbdoc";

    m.def("run_simulation", &runSimulation, R"pbdoc(
        Run an NFsim simulation from start to finish

        Args:
            argMap: Dictionary of command-line arguments for NFsim
            verbose: Enable verbose output

        Example:
            >>> import pyvcell_nfsim
            >>> args = {
            ...     "xml": "input.xml",
            ...     "o": "output.gdat",
            ...     "sim": "1.0"
            ... }
            >>> pyvcell_nfsim.run_simulation(args, False)
    )pbdoc",
        py::arg("argMap"), py::arg("verbose"));

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif

}//
// Created by cbontempi on 3/27/26.
//