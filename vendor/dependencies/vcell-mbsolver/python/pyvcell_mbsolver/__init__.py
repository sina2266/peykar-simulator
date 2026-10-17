"""
pyvcell_mbsolver — high-level Python wrapper around the compiled ``_core``
moving-boundary solver extension.

The low-level pybind11 bindings (``pyvcell_mbsolver._core``) expose C++ types
directly. This package provides friendlier Python classes on top:

  MovingBoundarySolver   — facade that owns setup + problem lifetime
  SimulationObserver     — abstract base for per-element callbacks
  TimeStepObserver       — abstract base for per-timestep callbacks

The compiled module remains available as ``pyvcell_mbsolver._core`` for direct
access to the underlying C++ API.
"""

from __future__ import annotations

import abc
from typing import List, Tuple, Optional

from . import _core as _mb

# Re-export enums and exceptions so callers only need to import this package.
RedistributionMode    = _mb.RedistributionMode
RedistributionVersion = _mb.RedistributionVersion
ExtrapolationMethod   = _mb.ExtrapolationMethod
TimeStepTooBig        = _mb.TimeStepTooBig

__all__ = [
    "MovingBoundarySolver",
    "TimeStepObserver",
    "SimulationObserver",
    "RedistributionMode",
    "RedistributionVersion",
    "ExtrapolationMethod",
    "TimeStepTooBig",
]


class TimeStepObserver(abc.ABC):
    """Abstract base for observers that receive one callback per time step.

    Subclass this and override :meth:`on_time` (and optionally
    :meth:`on_complete`), then register with
    :meth:`MovingBoundarySolver.add_time_observer`.

    The *geometry* argument passed to :meth:`on_time` is a
    :class:`vcellmbsolver_py.GeometryInfo` instance that carries the current
    moving-front boundary as a list of ``(x, y)`` float tuples.
    """

    @abc.abstractmethod
    def on_time(self, t: float, generation: int, last: bool, geometry) -> None:
        """Called at each output time step.

        Parameters
        ----------
        t:
            Current simulation time.
        generation:
            Number of front-propagation steps completed so far.
        last:
            True on the final time step.
        geometry:
            :class:`vcellmbsolver_py.GeometryInfo` — boundary snapshot.
        """

    def on_complete(self) -> None:
        """Called once after the simulation finishes.  Override as needed."""

    # ------------------------------------------------------------------
    # Internal: adapts this class to the C++ MovingBoundaryTimeClient API
    # ------------------------------------------------------------------
    def _as_time_client(self, name: str) -> _mb.TimeClient:
        observer = self

        class _Adapter(_mb.TimeClient):
            def outputName(self) -> str:         # noqa: N802
                return name

            def time(self, t, gen, last, gi):
                observer.on_time(t, gen, last, gi)

            def simulationComplete(self):        # noqa: N802
                observer.on_complete()

        return _Adapter()


class SimulationObserver(abc.ABC):
    """Abstract base for observers that receive per-element callbacks.

    Subclass this and override :meth:`on_element` (and optionally
    :meth:`on_time`, :meth:`on_iteration_complete`, :meth:`on_complete`),
    then register with :meth:`MovingBoundarySolver.add_element_observer`.

    Each :class:`vcellmbsolver_py.MeshElementNode` passed to
    :meth:`on_element` exposes:

    * ``.x``, ``.y`` — problem-domain coordinates
    * ``.grid_i``, ``.grid_j`` — integer grid indices
    * ``.is_inside``, ``.is_outside`` — boundary membership
    * ``.concentration(i)`` — current concentration of species *i*
    * ``.prior_concentration(i)`` — concentration at start of step
    """

    @abc.abstractmethod
    def on_element(self, node) -> None:
        """Called for every mesh node at every time step.

        Parameters
        ----------
        node:
            :class:`vcellmbsolver_py.MeshElementNode`
        """

    def on_time(self, t: float, generation: int, last: bool, geometry) -> None:
        """Called at the start of each time step.  Override as needed."""

    def on_iteration_complete(self) -> None:
        """Called after all elements have been visited.  Override as needed."""

    def on_complete(self) -> None:
        """Called once after the simulation finishes.  Override as needed."""

    # ------------------------------------------------------------------
    # Internal: adapts this class to the C++ MovingBoundaryElementClient API
    # ------------------------------------------------------------------
    def _as_element_client(self, name: str) -> _mb.ElementClient:
        observer = self

        class _Adapter(_mb.ElementClient):
            def outputName(self) -> str:           # noqa: N802
                return name

            def time(self, t, gen, last, gi):
                observer.on_time(t, gen, last, gi)

            def element(self, node):
                observer.on_element(node)

            def iterationComplete(self):           # noqa: N802
                observer.on_iteration_complete()

            def simulationComplete(self):          # noqa: N802
                observer.on_complete()

        return _Adapter()


class MovingBoundarySolver:
    """High-level facade for running a moving-boundary simulation.

    Usage — run from an XML file::

        solver = MovingBoundarySolver.from_xml("problem.xml")
        solver.run()

    Usage — programmatic setup::

        setup = vcellmbsolver_py.MovingBoundarySetup()
        setup.max_time = 1.0
        ...
        solver = MovingBoundarySolver(setup)
        solver.add_element_observer(MyObserver(), name="collector")
        solver.run()

    Parameters
    ----------
    setup:
        A :class:`vcellmbsolver_py.MovingBoundarySetup` instance.
    progress_percent:
        If > 0, print a progress line every this many percent (1–99).
    """

    def __init__(
        self,
        setup: _mb.MovingBoundarySetup,
        *,
        progress_percent: int = 0,
    ) -> None:
        self._setup = setup
        self._progress_percent = progress_percent
        self._problem: Optional[_mb.MovingBoundaryParabolicProblem] = None
        # Keep adapter objects alive for the duration of run()
        self._time_clients: List[_mb.TimeClient] = []
        self._element_clients: List[_mb.ElementClient] = []

    # ------------------------------------------------------------------
    # Construction helpers
    # ------------------------------------------------------------------

    @classmethod
    def from_xml(
        cls,
        filename: str,
        *,
        task_id: int = -1,
        nx: int = -1,
        progress_percent: int = 0,
    ) -> "MovingBoundarySolver":
        """Create a solver by loading a MovingBoundarySetup XML file.

        Parameters
        ----------
        filename:
            Path to the XML input file.
        task_id:
            Task identifier for parameter studies (passed through to C++).
        nx:
            Override the grid resolution from the XML (-1 = use XML value).
        progress_percent:
            Print progress every this many percent (0 = silent).
        """
        setup = _mb.MovingBoundarySetup.from_xml(filename, task_id, nx)
        return cls(setup, progress_percent=progress_percent)

    # ------------------------------------------------------------------
    # Observer registration
    # ------------------------------------------------------------------

    def add_time_observer(
        self, observer: TimeStepObserver, *, name: str = "python_time_observer"
    ) -> None:
        """Register a :class:`TimeStepObserver`.

        Must be called *before* :meth:`run`.
        """
        if self._problem is not None:
            raise RuntimeError("Cannot add observers after run() has been called.")
        client = observer._as_time_client(name)
        self._time_clients.append(client)

    def add_element_observer(
        self, observer: SimulationObserver, *, name: str = "python_element_observer"
    ) -> None:
        """Register a :class:`SimulationObserver`.

        Must be called *before* :meth:`run`.
        """
        if self._problem is not None:
            raise RuntimeError("Cannot add observers after run() has been called.")
        client = observer._as_element_client(name)
        self._element_clients.append(client)

    # ------------------------------------------------------------------
    # Run
    # ------------------------------------------------------------------

    def run(self) -> None:
        """Build the C++ problem object and run the simulation."""
        self._problem = _mb.MovingBoundaryParabolicProblem(self._setup)
        if self._progress_percent > 0:
            self._problem.report_progress(self._progress_percent)
        for c in self._time_clients:
            self._problem.add_time_client(c)
        for c in self._element_clients:
            self._problem.add_element_client(c)
        self._problem.run()

    # ------------------------------------------------------------------
    # Post-run inspection
    # ------------------------------------------------------------------

    @property
    def setup(self) -> _mb.MovingBoundarySetup:
        """The :class:`vcellmbsolver_py.MovingBoundarySetup` used by this solver."""
        return self._setup

    @property
    def problem(self) -> Optional[_mb.MovingBoundaryParabolicProblem]:
        """The underlying C++ problem object (available after :meth:`run`)."""
        return self._problem

    def output_files(self) -> List[str]:
        """Return HDF5 output file paths (available after :meth:`run`)."""
        if self._problem is None:
            return []
        return self._problem.get_output_files()

    def front_time_step(self) -> float:
        """Front propagation time step (available after :meth:`run`)."""
        self._require_run("front_time_step")
        return self._problem.front_time_step()

    def solver_time_step(self) -> float:
        """Diffusion/advection solver time step (available after :meth:`run`)."""
        self._require_run("solver_time_step")
        return self._problem.solver_time_step()

    def number_time_steps(self) -> int:
        """Total number of time steps (available after :meth:`run`)."""
        self._require_run("number_time_steps")
        return self._problem.number_time_steps()

    def end_time(self) -> float:
        """Simulation end time (available after :meth:`run`)."""
        self._require_run("end_time")
        return self._problem.end_time()

    def _require_run(self, name: str) -> None:
        if self._problem is None:
            raise RuntimeError(f"{name}() is only available after run().")
