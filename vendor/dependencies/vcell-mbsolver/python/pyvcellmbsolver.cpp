#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <MovingBoundarySetup.h>
#include <MovingBoundaryParabolicProblem.h>
#include <MeshElementNode.h>
#include <CoordVect.h>
#include <IndexVect.h>
#include <ReportClient.h>
#include <World.h>
#include <Universe.h>
#include <tinyxml2.h>

namespace py = pybind11;
using namespace moving_boundary;
using namespace spatial;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Convert a world-coordinate TPoint<CoordinateType,2> to problem-domain (x, y)
static std::pair<double, double>
worldPointToPD(const TPoint<CoordinateType, 2>& pt)
{
    auto pd = World<CoordinateType, 2>::get().toProblemDomain(pt);
    return { pd.get(cX), pd.get(cY) };
}

// Convert the GeometryInfo boundary vector to a Python-friendly list of tuples
static std::vector<std::pair<double, double>>
boundaryAsPairs(const GeometryInfo<CoordinateType>& gi)
{
    std::vector<std::pair<double, double>> result;
    result.reserve(gi.boundary.size());
    for (const auto& pt : gi.boundary) {
        result.push_back(worldPointToPD(pt));
    }
    return result;
}

// Load an XML file and call MovingBoundarySetup::setupProblem
static MovingBoundarySetup
setupFromXml(const std::string& filename, int taskId, int nx)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load XML: " + filename
                                 + " — " + doc.GetErrorStr1());
    }
    const tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        throw std::runtime_error("XML file has no root element: " + filename);
    }
    return MovingBoundarySetup::setupProblem(*root, taskId, nx);
}

// Full end-to-end run from XML (mirrors CLI usage)
static void
runFromXml(const std::string& xmlFile, const std::string& outputFile,
           int taskId, int nx)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlFile.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load XML: " + xmlFile
                                 + " — " + doc.GetErrorStr1());
    }
    const tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        throw std::runtime_error("XML file has no root element: " + xmlFile);
    }
    auto setup = MovingBoundarySetup::setupProblem(*root, taskId, nx);
    MovingBoundaryParabolicProblem problem(setup);
    ReportClient::setup(*root, outputFile, problem);
    problem.run();
}

// ---------------------------------------------------------------------------
// Trampoline: MovingBoundaryTimeClient
// ---------------------------------------------------------------------------
class PyTimeClient : public MovingBoundaryTimeClient {
public:
    using MovingBoundaryTimeClient::MovingBoundaryTimeClient;

    std::string outputName() const override {
        PYBIND11_OVERRIDE_PURE(std::string, MovingBoundaryTimeClient, outputName);
    }

    void time(double t, unsigned int generationCount, bool last,
              const GeometryInfo<CoordinateType>& gi) override {
        PYBIND11_OVERRIDE_PURE(void, MovingBoundaryTimeClient, time,
                               t, generationCount, last, gi);
    }

    void simulationComplete() override {
        PYBIND11_OVERRIDE_PURE(void, MovingBoundaryTimeClient, simulationComplete);
    }
};

// ---------------------------------------------------------------------------
// Trampoline: MovingBoundaryElementClient
// ---------------------------------------------------------------------------
class PyElementClient : public MovingBoundaryElementClient {
public:
    using MovingBoundaryElementClient::MovingBoundaryElementClient;

    std::string outputName() const override {
        PYBIND11_OVERRIDE_PURE(std::string, MovingBoundaryElementClient, outputName);
    }

    void time(double t, unsigned int generationCount, bool last,
              const GeometryInfo<CoordinateType>& gi) override {
        PYBIND11_OVERRIDE_PURE(void, MovingBoundaryElementClient, time,
                               t, generationCount, last, gi);
    }

    void simulationComplete() override {
        PYBIND11_OVERRIDE_PURE(void, MovingBoundaryElementClient, simulationComplete);
    }

    void element(const MeshElementNode& e) override {
        // Do NOT use PYBIND11_OVERRIDE_PURE here: the default py::cast for a
        // const-ref argument uses return_value_policy::copy, which invokes
        // Volume's copy-constructor.  That constructor *steals* the state
        // pointer from the original (setting original.vol.state = nullptr),
        // corrupting the live mesh node and causing a SIGSEGV in FronTier on
        // the next time step.  Pass a non-owning pointer instead.
        py::gil_scoped_acquire gil;
        auto fn = py::get_override(
            static_cast<const MovingBoundaryElementClient*>(this), "element");
        if (!fn) {
            py::pybind11_fail(
                "Tried to call pure virtual function "
                "\"MovingBoundaryElementClient::element\"");
        }
        fn(py::cast(&e, py::return_value_policy::reference));
    }

    void iterationComplete() override {
        PYBIND11_OVERRIDE_PURE(void, MovingBoundaryElementClient, iterationComplete);
    }
};

// ---------------------------------------------------------------------------
// Module definition
// ---------------------------------------------------------------------------
PYBIND11_MODULE(_core, m) {
    m.doc() = "VCell Moving Boundary Solver — Python bindings (pyvcell_mbsolver._core)";

    // -----------------------------------------------------------------------
    // Enums
    // -----------------------------------------------------------------------
    py::enum_<REDISTRIBUTION_MODE>(m, "RedistributionMode")
        .value("NO_REDIST",        NO_REDIST)
        .value("EXPANSION_REDIST", EXPANSION_REDIST)
        .value("FULL_REDIST",      FULL_REDIST)
        .export_values();

    py::enum_<REDISTRIBUTION_VERSION>(m, "RedistributionVersion")
        .value("ORDINARY_REDISTRIBUTE",  ORDINARY_REDISTRIBUTE)
        .value("EQUI_BOND_REDISTRIBUTE", EQUI_BOND_REDISTRIBUTE)
        .export_values();

    py::enum_<EXTRAPOLATION_METHOD>(m, "ExtrapolationMethod")
        .value("NEAREST_NEIGHBOR", NEAREST_NEIGHBOR)
        .export_values();

    // -----------------------------------------------------------------------
    // IndexVect  (2-D integer grid vector)
    // -----------------------------------------------------------------------
    py::class_<IndexVect>(m, "IndexVect",
        "2-D integer vector used for grid dimensions (Nx).")
        .def(py::init<int, int>(), py::arg("i"), py::arg("j"))
        .def("__getitem__",
             [](const IndexVect& v, int i) -> int { return v[i]; })
        .def_property("i",
            [](const IndexVect& v) { return v[0]; },
            [](IndexVect& v, int val) { v[0] = val; })
        .def_property("j",
            [](const IndexVect& v) { return v[1]; },
            [](IndexVect& v, int val) { v[1] = val; })
        .def("__repr__", [](const IndexVect& v) {
            return "IndexVect(" + std::to_string(v[0])
                   + ", " + std::to_string(v[1]) + ")";
        });

    // -----------------------------------------------------------------------
    // CoordVect  (2-D floating-point coordinate vector)
    // -----------------------------------------------------------------------
    py::class_<CoordVect>(m, "CoordVect",
        "2-D floating-point coordinate vector.")
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def("__getitem__",
             [](const CoordVect& v, int i) -> double { return v[i]; })
        .def_property("x",
            [](const CoordVect& v) { return v[0]; },
            [](CoordVect& v, double val) { v[0] = val; })
        .def_property("y",
            [](const CoordVect& v) { return v[1]; },
            [](CoordVect& v, double val) { v[1] = val; })
        .def("__repr__", [](const CoordVect& v) {
            return "CoordVect(" + std::to_string(v[0])
                   + ", " + std::to_string(v[1]) + ")";
        });

    // -----------------------------------------------------------------------
    // MovingBoundarySetup
    // -----------------------------------------------------------------------
    py::class_<MovingBoundarySetup>(m, "MovingBoundarySetup", R"(
Configuration for a moving-boundary simulation.

Can be constructed programmatically (set fields then pass to
MovingBoundaryParabolicProblem), or loaded from an XML file via
``MovingBoundarySetup.from_xml(filename)``.
)")
        .def(py::init<>())
        .def_readwrite("Nx",                       &MovingBoundarySetup::Nx,
            "Grid dimensions (IndexVect)")
        .def_readwrite("extent_x",                 &MovingBoundarySetup::extentX,
            "X extent of the domain (CoordVect: low, high)")
        .def_readwrite("extent_y",                 &MovingBoundarySetup::extentY,
            "Y extent of the domain (CoordVect: low, high)")
        .def_readwrite("front_to_node_ratio",      &MovingBoundarySetup::frontToNodeRatio,
            "Ratio of front spacing to mesh node spacing (default 5)")
        .def_readwrite("redistribution_mode",      &MovingBoundarySetup::redistributionMode,
            "RedistributionMode enum")
        .def_readwrite("redistribution_version",   &MovingBoundarySetup::redistributionVersion,
            "RedistributionVersion enum")
        .def_readwrite("redistribution_frequency", &MovingBoundarySetup::redistributionFrequency,
            "How often (in steps) to redistribute the front")
        .def_readwrite("extrapolation_method",     &MovingBoundarySetup::extrapolationMethod,
            "ExtrapolationMethod enum")
        .def_readwrite("max_time",                 &MovingBoundarySetup::maxTime,
            "End time of the simulation")
        .def_readwrite("front_time_step",          &MovingBoundarySetup::frontTimeStep,
            "Front propagation time step (expression string)")
        .def_readwrite("solver_time_step",         &MovingBoundarySetup::solverTimeStep,
            "Diffusion/advection solver time step (expression string)")
        .def_readwrite("output_time_step",         &MovingBoundarySetup::outputTimeStep,
            "How often to write output (expression string)")
        .def_readwrite("hard_time",                &MovingBoundarySetup::hardTime,
            "If True, abort when the time step would need adjustment")
        .def_readwrite("level_function",           &MovingBoundarySetup::levelFunctionStr,
            "Level-set function string (defines initial front position)")
        .def_readwrite("front_velocity_x",         &MovingBoundarySetup::frontVelocityFunctionStrX,
            "Front velocity in X (expression string)")
        .def_readwrite("front_velocity_y",         &MovingBoundarySetup::frontVelocityFunctionStrY,
            "Front velocity in Y (expression string)")
        .def_readwrite("diffusion_constant",       &MovingBoundarySetup::diffusionConstant,
            "Global diffusion constant")
        .def_static("from_xml",
            &setupFromXml,
            py::arg("filename"), py::arg("task_id") = -1, py::arg("nx") = -1,
            R"(
Parse a MovingBoundarySetup XML configuration file.

Parameters
----------
filename : str
    Path to the XML input file.
task_id : int, optional
    Task identifier for parameter studies (default -1).
nx : int, optional
    Override the Nx grid resolution from the XML (default -1 = use XML value).
)");

    // -----------------------------------------------------------------------
    // GeometryInfo  (read-only; passed to callback methods)
    // -----------------------------------------------------------------------
    py::class_<GeometryInfo<CoordinateType>>(m, "GeometryInfo", R"(
Snapshot of the moving boundary geometry at one time step.
Passed to TimeClient.time() and ElementClient.time() callbacks.
)")
        .def_property_readonly("nodes_adjusted",
            [](const GeometryInfo<CoordinateType>& gi) {
                return gi.nodesAdjusted;
            },
            "True if mesh nodes were repositioned since the last time step")
        .def_property_readonly("boundary",
            &boundaryAsPairs,
            "Moving front as a list of (x, y) float tuples in problem-domain coordinates");

    // -----------------------------------------------------------------------
    // MeshElementNode  (read-only; passed to ElementClient.element())
    // -----------------------------------------------------------------------
    py::class_<MeshElementNode>(m, "MeshElementNode", R"(
One node of the moving-boundary mesh.
Passed to ElementClient.element() during each time step.
)")
        .def_property_readonly("x",
            [](const MeshElementNode& e) -> double {
                return CoordVect(e).toProblemDomain()[0];
            },
            "X coordinate in problem-domain units")
        .def_property_readonly("y",
            [](const MeshElementNode& e) -> double {
                return CoordVect(e).toProblemDomain()[1];
            },
            "Y coordinate in problem-domain units")
        .def_property_readonly("grid_i",
            [](const MeshElementNode& e) { return static_cast<int>(e.indexOf(0)); },
            "Grid index along X")
        .def_property_readonly("grid_j",
            [](const MeshElementNode& e) { return static_cast<int>(e.indexOf(1)); },
            "Grid index along Y")
        .def_property_readonly("is_inside",
            [](const MeshElementNode& e) { return e.isInside(); },
            "True if node is inside the moving boundary")
        .def_property_readonly("is_outside",
            [](const MeshElementNode& e) { return e.isOutside(); },
            "True if node is outside the moving boundary")
        .def("concentration",
            [](const MeshElementNode& e, size_t i) -> double {
                return e.concentration(i);
            },
            py::arg("species_index"),
            "Current concentration of species i at this node")
        .def("prior_concentration",
            [](const MeshElementNode& e, size_t i) -> double {
                return e.priorConcentration(i);
            },
            py::arg("species_index"),
            "Concentration of species i at the start of this time cycle");

    // -----------------------------------------------------------------------
    // Abstract callback base: TimeClient
    // -----------------------------------------------------------------------
    py::class_<MovingBoundaryTimeClient, PyTimeClient>(m, "TimeClient", R"(
Abstract base class for time-step callbacks.

Subclass this in Python and override all abstract methods, then register
an instance with MovingBoundaryParabolicProblem.add_time_client().

Abstract methods
----------------
output_name() -> str
time(t, generation_count, last, geometry) -> None
simulation_complete() -> None
)")
        .def(py::init<>())
        .def("output_name",         &MovingBoundaryTimeClient::outputName)
        .def("time",                &MovingBoundaryTimeClient::time,
             py::arg("t"), py::arg("generation_count"),
             py::arg("last"), py::arg("geometry"))
        .def("simulation_complete", &MovingBoundaryTimeClient::simulationComplete);

    // -----------------------------------------------------------------------
    // Abstract callback base: ElementClient
    // -----------------------------------------------------------------------
    py::class_<MovingBoundaryElementClient, PyElementClient,
               MovingBoundaryTimeClient>(m, "ElementClient", R"(
Abstract base class for per-element callbacks (extends TimeClient).

Subclass this in Python to receive every mesh node at every time step.
Register with MovingBoundaryParabolicProblem.add_element_client().

Additional abstract methods
---------------------------
element(node: MeshElementNode) -> None
iteration_complete() -> None
)")
        .def(py::init<>())
        .def("element",            &MovingBoundaryElementClient::element,
             py::arg("node"))
        .def("iteration_complete", &MovingBoundaryElementClient::iterationComplete);

    // -----------------------------------------------------------------------
    // TimeStepTooBig exception
    // -----------------------------------------------------------------------
    py::register_exception<TimeStepTooBig>(m, "TimeStepTooBig", PyExc_RuntimeError);

    // -----------------------------------------------------------------------
    // MovingBoundaryParabolicProblem  (the solver)
    // -----------------------------------------------------------------------
    py::class_<MovingBoundaryParabolicProblem>(m, "MovingBoundaryParabolicProblem", R"(
Moving-boundary parabolic PDE solver.

Typical usage
-------------
::

    setup = MovingBoundarySetup.from_xml("problem.xml")
    problem = MovingBoundaryParabolicProblem(setup)
    problem.report_progress(10)   # print progress every 10 %
    problem.run()
    print("output:", problem.get_output_files())
)")
        .def(py::init<const MovingBoundarySetup&>(), py::arg("setup"))
        .def("run",
             &MovingBoundaryParabolicProblem::run,
             "Run the simulation to completion.")
        .def("report_progress",
             &MovingBoundaryParabolicProblem::reportProgress,
             py::arg("percent"), py::arg("estimate_time") = false,
             "Print a progress message every ``percent`` percent (1–99).")
        .def("add_time_client",
             [](MovingBoundaryParabolicProblem& p, MovingBoundaryTimeClient& c) {
                 p.add(c);
             },
             py::arg("client"), py::keep_alive<1, 2>(),
             "Register a TimeClient; the client must stay alive for the duration of run().")
        .def("add_element_client",
             [](MovingBoundaryParabolicProblem& p, MovingBoundaryElementClient& c) {
                 p.add(c);
             },
             py::arg("client"), py::keep_alive<1, 2>(),
             "Register an ElementClient; the client must stay alive for the duration of run().")
        .def("get_output_files",  &MovingBoundaryParabolicProblem::getOutputFiles,
             "Return the paths of all output files written by registered report clients.")
        .def("front_time_step",   &MovingBoundaryParabolicProblem::frontTimeStep,
             "Time step used to propagate the moving front.")
        .def("solver_time_step",  &MovingBoundaryParabolicProblem::solverTimeStep,
             "Time step used by the implicit diffusion/advection solver.")
        .def("mesh_interval",     &MovingBoundaryParabolicProblem::meshInterval,
             "Spatial mesh interval.")
        .def("number_time_steps", &MovingBoundaryParabolicProblem::numberTimeSteps,
             "Total number of time steps in the simulation.")
        .def("end_time",          &MovingBoundaryParabolicProblem::endTime,
             "Simulation end time.")
        .def("no_reaction",       &MovingBoundaryParabolicProblem::noReaction,
             "True if all reaction terms are identically zero.")
        .def("front_description", &MovingBoundaryParabolicProblem::frontDescription,
             "Human-readable description of the front provider.");

    // -----------------------------------------------------------------------
    // Free functions
    // -----------------------------------------------------------------------
    m.def("setup_from_xml",
          &setupFromXml,
          py::arg("filename"), py::arg("task_id") = -1, py::arg("nx") = -1,
          R"(
Parse an XML configuration file and return a MovingBoundarySetup.

Equivalent to ``MovingBoundarySetup.from_xml(filename)``.
)");

    m.def("universe_destroy",
          []() { moving_boundary::Universe<2>::get().destroy(); },
          R"(
Reset the Universe<2> singleton so it can be re-initialized.

This is required between independent simulations in the same process (e.g.
between pytest test cases).  Calling it when no simulation has been set up
is a no-op.
)");

    m.def("run_from_xml",
          &runFromXml,
          py::arg("xml_file"), py::arg("output_file"),
          py::arg("task_id") = -1, py::arg("nx") = -1,
          R"(
Run a complete simulation from an XML input file.

This is the Python equivalent of the MovingBoundarySolver command-line binary.
HDF5 output is written to ``output_file`` (extension added automatically by
the solver).

Parameters
----------
xml_file : str
    Path to the MovingBoundarySetup XML input file.
output_file : str
    Base path for HDF5 output.
task_id : int, optional
    Task identifier for parameter studies (default -1).
nx : int, optional
    Override the Nx grid resolution from the XML (default -1 = use XML value).

Raises
------
RuntimeError
    On XML parse failure or simulation error.
TimeStepTooBig
    If the time step is numerically unstable and ``hard_time`` is True.
)");
}
