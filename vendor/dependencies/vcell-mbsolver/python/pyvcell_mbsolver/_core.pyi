"""
VCell Moving Boundary Solver — Python bindings (pyvcell_mbsolver._core)
"""
from __future__ import annotations
import typing
__all__: list[str] = ['CoordVect', 'EQUI_BOND_REDISTRIBUTE', 'EXPANSION_REDIST', 'ElementClient', 'ExtrapolationMethod', 'FULL_REDIST', 'GeometryInfo', 'IndexVect', 'MeshElementNode', 'MovingBoundaryParabolicProblem', 'MovingBoundarySetup', 'NEAREST_NEIGHBOR', 'NO_REDIST', 'ORDINARY_REDISTRIBUTE', 'RedistributionMode', 'RedistributionVersion', 'TimeClient', 'TimeStepTooBig', 'run_from_xml', 'setup_from_xml', 'universe_destroy']
class CoordVect:
    """
    2-D floating-point coordinate vector.
    """
    x: float
    y: float
    def __getitem__(self, arg0: int) -> float:
        ...
    def __init__(self, x: float, y: float) -> None:
        ...
    def __repr__(self) -> str:
        ...
class ElementClient(TimeClient):
    """
    
    Abstract base class for per-element callbacks (extends TimeClient).
    
    Subclass this in Python to receive every mesh node at every time step.
    Register with MovingBoundaryParabolicProblem.add_element_client().
    
    Additional abstract methods
    ---------------------------
    element(node: MeshElementNode) -> None
    iteration_complete() -> None
    """
    def __init__(self) -> None:
        ...
    def element(self, node: MeshElementNode) -> None:
        ...
    def iteration_complete(self) -> None:
        ...
class ExtrapolationMethod:
    """
    Members:
    
      NEAREST_NEIGHBOR
    """
    NEAREST_NEIGHBOR: typing.ClassVar[ExtrapolationMethod]  # value = <ExtrapolationMethod.NEAREST_NEIGHBOR: 1>
    __members__: typing.ClassVar[dict[str, ExtrapolationMethod]]  # value = {'NEAREST_NEIGHBOR': <ExtrapolationMethod.NEAREST_NEIGHBOR: 1>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class GeometryInfo:
    """
    
    Snapshot of the moving boundary geometry at one time step.
    Passed to TimeClient.time() and ElementClient.time() callbacks.
    """
    @property
    def boundary(self) -> list[tuple[float, float]]:
        """
        Moving front as a list of (x, y) float tuples in problem-domain coordinates
        """
    @property
    def nodes_adjusted(self) -> bool:
        """
        True if mesh nodes were repositioned since the last time step
        """
class IndexVect:
    """
    2-D integer vector used for grid dimensions (Nx).
    """
    i: int
    j: int
    def __getitem__(self, arg0: int) -> int:
        ...
    def __init__(self, i: int, j: int) -> None:
        ...
    def __repr__(self) -> str:
        ...
class MeshElementNode:
    """
    
    One node of the moving-boundary mesh.
    Passed to ElementClient.element() during each time step.
    """
    def concentration(self, species_index: int) -> float:
        """
        Current concentration of species i at this node
        """
    def prior_concentration(self, species_index: int) -> float:
        """
        Concentration of species i at the start of this time cycle
        """
    @property
    def grid_i(self) -> int:
        """
        Grid index along X
        """
    @property
    def grid_j(self) -> int:
        """
        Grid index along Y
        """
    @property
    def is_inside(self) -> bool:
        """
        True if node is inside the moving boundary
        """
    @property
    def is_outside(self) -> bool:
        """
        True if node is outside the moving boundary
        """
    @property
    def x(self) -> float:
        """
        X coordinate in problem-domain units
        """
    @property
    def y(self) -> float:
        """
        Y coordinate in problem-domain units
        """
class MovingBoundaryParabolicProblem:
    """
    
    Moving-boundary parabolic PDE solver.
    
    Typical usage
    -------------
    ::
    
        setup = MovingBoundarySetup.from_xml("problem.xml")
        problem = MovingBoundaryParabolicProblem(setup)
        problem.report_progress(10)   # print progress every 10 %
        problem.run()
        print("output:", problem.get_output_files())
    """
    def __init__(self, setup: MovingBoundarySetup) -> None:
        ...
    def add_element_client(self, client: ElementClient) -> None:
        """
        Register an ElementClient; the client must stay alive for the duration of run().
        """
    def add_time_client(self, client: TimeClient) -> None:
        """
        Register a TimeClient; the client must stay alive for the duration of run().
        """
    def end_time(self) -> float:
        """
        Simulation end time.
        """
    def front_description(self) -> str:
        """
        Human-readable description of the front provider.
        """
    def front_time_step(self) -> float:
        """
        Time step used to propagate the moving front.
        """
    def get_output_files(self) -> str:
        """
        Return the paths of all output files written by registered report clients.
        """
    def mesh_interval(self) -> float:
        """
        Spatial mesh interval.
        """
    def no_reaction(self) -> bool:
        """
        True if all reaction terms are identically zero.
        """
    def number_time_steps(self) -> int:
        """
        Total number of time steps in the simulation.
        """
    def report_progress(self, percent: int, estimate_time: bool = False) -> None:
        """
        Print a progress message every ``percent`` percent (1–99).
        """
    def run(self) -> None:
        """
        Run the simulation to completion.
        """
    def solver_time_step(self) -> float:
        """
        Time step used by the implicit diffusion/advection solver.
        """
class MovingBoundarySetup:
    """
    
    Configuration for a moving-boundary simulation.
    
    Can be constructed programmatically (set fields then pass to
    MovingBoundaryParabolicProblem), or loaded from an XML file via
    ``MovingBoundarySetup.from_xml(filename)``.
    """
    @staticmethod
    def from_xml(filename: str, task_id: int = -1, nx: int = -1) -> MovingBoundarySetup:
        """
        Parse a MovingBoundarySetup XML configuration file.
        
        Parameters
        ----------
        filename : str
            Path to the XML input file.
        task_id : int, optional
            Task identifier for parameter studies (default -1).
        nx : int, optional
            Override the Nx grid resolution from the XML (default -1 = use XML value).
        """
    def __init__(self) -> None:
        ...
    @property
    def Nx(self) -> IndexVect:
        """
        Grid dimensions (IndexVect)
        """
    @Nx.setter
    def Nx(self, arg0: IndexVect) -> None:
        ...
    @property
    def diffusion_constant(self) -> float:
        """
        Global diffusion constant
        """
    @diffusion_constant.setter
    def diffusion_constant(self, arg0: float) -> None:
        ...
    @property
    def extent_x(self) -> CoordVect:
        """
        X extent of the domain (CoordVect: low, high)
        """
    @extent_x.setter
    def extent_x(self, arg0: CoordVect) -> None:
        ...
    @property
    def extent_y(self) -> CoordVect:
        """
        Y extent of the domain (CoordVect: low, high)
        """
    @extent_y.setter
    def extent_y(self, arg0: CoordVect) -> None:
        ...
    @property
    def extrapolation_method(self) -> ExtrapolationMethod:
        """
        ExtrapolationMethod enum
        """
    @extrapolation_method.setter
    def extrapolation_method(self, arg0: ExtrapolationMethod) -> None:
        ...
    @property
    def front_time_step(self) -> str:
        """
        Front propagation time step (expression string)
        """
    @front_time_step.setter
    def front_time_step(self, arg0: str) -> None:
        ...
    @property
    def front_to_node_ratio(self) -> float:
        """
        Ratio of front spacing to mesh node spacing (default 5)
        """
    @front_to_node_ratio.setter
    def front_to_node_ratio(self, arg0: float) -> None:
        ...
    @property
    def front_velocity_x(self) -> str:
        """
        Front velocity in X (expression string)
        """
    @front_velocity_x.setter
    def front_velocity_x(self, arg0: str) -> None:
        ...
    @property
    def front_velocity_y(self) -> str:
        """
        Front velocity in Y (expression string)
        """
    @front_velocity_y.setter
    def front_velocity_y(self, arg0: str) -> None:
        ...
    @property
    def hard_time(self) -> bool:
        """
        If True, abort when the time step would need adjustment
        """
    @hard_time.setter
    def hard_time(self, arg0: bool) -> None:
        ...
    @property
    def level_function(self) -> str:
        """
        Level-set function string (defines initial front position)
        """
    @level_function.setter
    def level_function(self, arg0: str) -> None:
        ...
    @property
    def max_time(self) -> float:
        """
        End time of the simulation
        """
    @max_time.setter
    def max_time(self, arg0: float) -> None:
        ...
    @property
    def output_time_step(self) -> str:
        """
        How often to write output (expression string)
        """
    @output_time_step.setter
    def output_time_step(self, arg0: str) -> None:
        ...
    @property
    def redistribution_frequency(self) -> int:
        """
        How often (in steps) to redistribute the front
        """
    @redistribution_frequency.setter
    def redistribution_frequency(self, arg0: int) -> None:
        ...
    @property
    def redistribution_mode(self) -> RedistributionMode:
        """
        RedistributionMode enum
        """
    @redistribution_mode.setter
    def redistribution_mode(self, arg0: RedistributionMode) -> None:
        ...
    @property
    def redistribution_version(self) -> RedistributionVersion:
        """
        RedistributionVersion enum
        """
    @redistribution_version.setter
    def redistribution_version(self, arg0: RedistributionVersion) -> None:
        ...
    @property
    def solver_time_step(self) -> str:
        """
        Diffusion/advection solver time step (expression string)
        """
    @solver_time_step.setter
    def solver_time_step(self, arg0: str) -> None:
        ...
class RedistributionMode:
    """
    Members:
    
      NO_REDIST
    
      EXPANSION_REDIST
    
      FULL_REDIST
    """
    EXPANSION_REDIST: typing.ClassVar[RedistributionMode]  # value = <RedistributionMode.EXPANSION_REDIST: 1>
    FULL_REDIST: typing.ClassVar[RedistributionMode]  # value = <RedistributionMode.FULL_REDIST: 2>
    NO_REDIST: typing.ClassVar[RedistributionMode]  # value = <RedistributionMode.NO_REDIST: 0>
    __members__: typing.ClassVar[dict[str, RedistributionMode]]  # value = {'NO_REDIST': <RedistributionMode.NO_REDIST: 0>, 'EXPANSION_REDIST': <RedistributionMode.EXPANSION_REDIST: 1>, 'FULL_REDIST': <RedistributionMode.FULL_REDIST: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class RedistributionVersion:
    """
    Members:
    
      ORDINARY_REDISTRIBUTE
    
      EQUI_BOND_REDISTRIBUTE
    """
    EQUI_BOND_REDISTRIBUTE: typing.ClassVar[RedistributionVersion]  # value = <RedistributionVersion.EQUI_BOND_REDISTRIBUTE: 2>
    ORDINARY_REDISTRIBUTE: typing.ClassVar[RedistributionVersion]  # value = <RedistributionVersion.ORDINARY_REDISTRIBUTE: 1>
    __members__: typing.ClassVar[dict[str, RedistributionVersion]]  # value = {'ORDINARY_REDISTRIBUTE': <RedistributionVersion.ORDINARY_REDISTRIBUTE: 1>, 'EQUI_BOND_REDISTRIBUTE': <RedistributionVersion.EQUI_BOND_REDISTRIBUTE: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class TimeClient:
    """
    
    Abstract base class for time-step callbacks.
    
    Subclass this in Python and override all abstract methods, then register
    an instance with MovingBoundaryParabolicProblem.add_time_client().
    
    Abstract methods
    ----------------
    output_name() -> str
    time(t, generation_count, last, geometry) -> None
    simulation_complete() -> None
    """
    def __init__(self) -> None:
        ...
    def output_name(self) -> str:
        ...
    def simulation_complete(self) -> None:
        ...
    def time(self, t: float, generation_count: int, last: bool, geometry: GeometryInfo) -> None:
        ...
class TimeStepTooBig(RuntimeError):
    pass
def run_from_xml(xml_file: str, output_file: str, task_id: int = -1, nx: int = -1) -> None:
    """
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
    """
def setup_from_xml(filename: str, task_id: int = -1, nx: int = -1) -> MovingBoundarySetup:
    """
    Parse an XML configuration file and return a MovingBoundarySetup.
    
    Equivalent to ``MovingBoundarySetup.from_xml(filename)``.
    """
def universe_destroy() -> None:
    """
    Reset the Universe<2> singleton so it can be re-initialized.
    
    This is required between independent simulations in the same process (e.g.
    between pytest test cases).  Calling it when no simulation has been set up
    is a no-op.
    """
EQUI_BOND_REDISTRIBUTE: RedistributionVersion  # value = <RedistributionVersion.EQUI_BOND_REDISTRIBUTE: 2>
EXPANSION_REDIST: RedistributionMode  # value = <RedistributionMode.EXPANSION_REDIST: 1>
FULL_REDIST: RedistributionMode  # value = <RedistributionMode.FULL_REDIST: 2>
NEAREST_NEIGHBOR: ExtrapolationMethod  # value = <ExtrapolationMethod.NEAREST_NEIGHBOR: 1>
NO_REDIST: RedistributionMode  # value = <RedistributionMode.NO_REDIST: 0>
ORDINARY_REDISTRIBUTE: RedistributionVersion  # value = <RedistributionVersion.ORDINARY_REDISTRIBUTE: 1>
