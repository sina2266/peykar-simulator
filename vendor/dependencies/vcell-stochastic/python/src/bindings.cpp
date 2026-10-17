#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Gibson.h"
#include "MultiTrialStats.h"

namespace py = pybind11;

PYBIND11_MODULE(vcellstochastic_py, m) {
    m.doc() = "VCell Stochastic Solver Python Bindings";

    py::class_<StochModel>(m, "StochModel")
        .def(py::init<>())
        .def("getNumOfVars", &StochModel::getNumOfVars)
        .def("getNumOfProcesses", &StochModel::getNumOfProcesses)
        .def("getVarIndex", &StochModel::getVarIndex)
        .def("getProcessIndex", &StochModel::getProcessIndex);

    py::class_<Gibson, StochModel>(m, "Gibson")
        .def(py::init<>())
        .def(py::init<const char*, const char*>())
        .def("core", &Gibson::core)
        .def("march", &Gibson::march)
        .def("getRandomUniform", &Gibson::getRandomUniform);

    py::class_<MultiTrialStats>(m, "MultiTrialStats")
        .def(py::init<int, int>())
        .def("startNewTrial", &MultiTrialStats::startNewTrial)
        .def("addSample", [](MultiTrialStats &self, int timeIndex, double timeValue, std::vector<double> varVals) {
            self.addSample(timeIndex, timeValue, varVals.data());
        })
        .def("getMean", &MultiTrialStats::getMean)
        .def("getVariance", &MultiTrialStats::getVariance)
        .def("getMin", &MultiTrialStats::getMin)
        .def("getMax", &MultiTrialStats::getMax)
        .def("getNumVars", &MultiTrialStats::getNumVars)
        .def("getNumTimePoints", &MultiTrialStats::getNumTimePoints)
        .def("getTimePoint", &MultiTrialStats::getTimePoint)
        .def("writeOutput", &MultiTrialStats::writeOutput);
}
