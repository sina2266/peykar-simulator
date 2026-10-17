"""Tests for the pyvcell_mbsolver Python package.

These tests require the pyvcell_mbsolver package (the _core C extension plus
its wrapper) to be importable. The build system sets PYTHONPATH to the build
tree's bin/ directory when running via ctest; for an installed wheel it is
importable directly.
"""

import os
import pytest

_HERE = os.path.dirname(__file__)
_DATA_DIR = os.path.join(_HERE, "data")
_MINIMAL_XML = os.path.join(_DATA_DIR, "minimal.xml")

from pyvcell_mbsolver import _core as _mb
from pyvcell_mbsolver import (
    MovingBoundarySolver,
    SimulationObserver,
    TimeStepObserver,
    RedistributionMode,
    RedistributionVersion,
    ExtrapolationMethod,
    TimeStepTooBig,
)


# ===========================================================================
# Enum sanity checks — verify the enum values are accessible and have the
# expected integer backing values from MovingBoundaryTypes.h / fdecs.h.
# ===========================================================================

class TestEnums:
    def test_redistribution_mode_values(self):
        assert int(RedistributionMode.NO_REDIST)        == 0
        assert int(RedistributionMode.EXPANSION_REDIST) == 1
        assert int(RedistributionMode.FULL_REDIST)      == 2

    def test_redistribution_version_values(self):
        assert int(RedistributionVersion.ORDINARY_REDISTRIBUTE)  == 1
        assert int(RedistributionVersion.EQUI_BOND_REDISTRIBUTE) == 2

    def test_extrapolation_method_values(self):
        assert int(ExtrapolationMethod.NEAREST_NEIGHBOR) == 1

    def test_enums_are_distinct(self):
        assert RedistributionMode.NO_REDIST != RedistributionMode.FULL_REDIST


# ===========================================================================
# Low-level pybind11 type construction
# ===========================================================================

class TestLowLevelTypes:
    def test_index_vect_construction(self):
        v = _mb.IndexVect(3, 7)
        assert v.i == 3
        assert v.j == 7

    def test_index_vect_item_access(self):
        v = _mb.IndexVect(5, 9)
        assert v[0] == 5
        assert v[1] == 9

    def test_coord_vect_construction(self):
        v = _mb.CoordVect(1.5, -2.5)
        assert v.x == pytest.approx(1.5)
        assert v.y == pytest.approx(-2.5)

    def test_coord_vect_item_access(self):
        v = _mb.CoordVect(0.1, 0.2)
        assert v[0] == pytest.approx(0.1)
        assert v[1] == pytest.approx(0.2)

    def test_moving_boundary_setup_default_construction(self):
        s = _mb.MovingBoundarySetup()
        assert s.front_to_node_ratio == pytest.approx(5.0)
        assert s.max_time == pytest.approx(0.0)
        assert not s.hard_time

    def test_time_step_too_big_is_runtime_error(self):
        assert issubclass(TimeStepTooBig, RuntimeError)


# ===========================================================================
# XML parsing via the static factory method
# ===========================================================================

class TestXmlParsing:
    def test_from_xml_returns_setup(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert isinstance(setup, _mb.MovingBoundarySetup)

    def test_from_xml_reads_max_time(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert setup.max_time == pytest.approx(0.1)

    def test_from_xml_reads_grid_dimensions(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert setup.Nx.i == 19
        assert setup.Nx.j == 19

    def test_from_xml_reads_redistribution_mode(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert setup.redistribution_mode == RedistributionMode.EXPANSION_REDIST

    def test_from_xml_reads_redistribution_version(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert setup.redistribution_version == RedistributionVersion.ORDINARY_REDISTRIBUTE

    def test_from_xml_reads_extrapolation_method(self):
        setup = _mb.MovingBoundarySetup.from_xml(_MINIMAL_XML)
        assert setup.extrapolation_method == ExtrapolationMethod.NEAREST_NEIGHBOR

    def test_from_xml_invalid_file_raises(self):
        with pytest.raises(RuntimeError):
            _mb.MovingBoundarySetup.from_xml("/nonexistent/path/missing.xml")

    def test_setup_from_xml_free_function(self):
        setup = _mb.setup_from_xml(_MINIMAL_XML)
        assert isinstance(setup, _mb.MovingBoundarySetup)
        assert setup.max_time == pytest.approx(0.1)


# ===========================================================================
# High-level MovingBoundarySolver facade
# ===========================================================================

class TestMovingBoundarySolverFacade:
    def test_from_xml_class_method(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        assert isinstance(solver, MovingBoundarySolver)
        assert solver.problem is None  # not run yet

    def test_output_files_before_run_is_empty(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        assert solver.output_files() == []

    def test_property_access_before_run_raises(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        with pytest.raises(RuntimeError):
            solver.front_time_step()
        with pytest.raises(RuntimeError):
            solver.solver_time_step()
        with pytest.raises(RuntimeError):
            solver.number_time_steps()

    def test_run_completes(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()
        assert solver.problem is not None

    def test_solver_time_step_after_run(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()
        dt = solver.solver_time_step()
        assert dt > 0.0

    def test_front_time_step_after_run(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()
        dt = solver.front_time_step()
        assert dt > 0.0

    def test_number_time_steps_after_run(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()
        n = solver.number_time_steps()
        assert n > 0

    def test_end_time_after_run(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()
        assert solver.end_time() == pytest.approx(0.1)

    def test_add_observer_after_run_raises(self):
        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.run()

        class _Obs(TimeStepObserver):
            def on_time(self, t, gen, last, geo): pass

        with pytest.raises(RuntimeError):
            solver.add_time_observer(_Obs())


# ===========================================================================
# TimeStepObserver subclassing
# ===========================================================================

class TestTimeStepObserver:
    def test_on_time_is_called(self):
        calls = []

        class _Recorder(TimeStepObserver):
            def on_time(self, t, generation, last, geometry):
                calls.append((t, generation, last))

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_time_observer(_Recorder(), name="recorder")
        solver.run()

        assert len(calls) > 0
        # The last call should have last=True
        assert calls[-1][2] is True

    def test_on_complete_is_called(self):
        completed = []

        class _CompletionTracker(TimeStepObserver):
            def on_time(self, t, gen, last, geo): pass
            def on_complete(self):
                completed.append(True)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_time_observer(_CompletionTracker(), name="tracker")
        solver.run()

        assert completed == [True]

    def test_time_values_are_monotonically_increasing(self):
        times = []

        class _TimeCollector(TimeStepObserver):
            def on_time(self, t, gen, last, geo):
                times.append(t)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_time_observer(_TimeCollector(), name="tc")
        solver.run()

        assert len(times) >= 2
        for a, b in zip(times, times[1:]):
            assert b >= a

    def test_geometry_boundary_is_list_of_tuples(self):
        boundaries = []

        class _GeoCollector(TimeStepObserver):
            def on_time(self, t, gen, last, geo):
                boundaries.append(geo.boundary)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_time_observer(_GeoCollector(), name="gc")
        solver.run()

        assert len(boundaries) > 0
        boundary = boundaries[0]
        assert isinstance(boundary, list)
        assert len(boundary) > 0
        x, y = boundary[0]
        assert isinstance(x, float)
        assert isinstance(y, float)

    def test_missing_abstract_method_raises(self):
        with pytest.raises(TypeError):
            class _Bad(TimeStepObserver):
                pass
            _Bad()


# ===========================================================================
# SimulationObserver subclassing
# ===========================================================================

class TestSimulationObserver:
    def test_on_element_is_called(self):
        elements = []

        class _Collector(SimulationObserver):
            def on_element(self, node):
                elements.append(node)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_Collector(), name="coll")
        solver.run()

        assert len(elements) > 0

    def test_mesh_node_has_coordinates(self):
        coords = []

        class _CoordCollector(SimulationObserver):
            def on_element(self, node):
                coords.append((node.x, node.y))

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_CoordCollector(), name="cc")
        solver.run()

        assert len(coords) > 0
        x, y = coords[0]
        assert isinstance(x, float)
        assert isinstance(y, float)

    def test_mesh_node_has_grid_indices(self):
        indices = []

        class _IdxCollector(SimulationObserver):
            def on_element(self, node):
                indices.append((node.grid_i, node.grid_j))

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_IdxCollector(), name="ic")
        solver.run()

        assert len(indices) > 0
        i, j = indices[0]
        assert isinstance(i, int)
        assert isinstance(j, int)

    def test_inside_outside_are_bool(self):
        flags = []

        class _FlagCollector(SimulationObserver):
            def on_element(self, node):
                flags.append((node.is_inside, node.is_outside))

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_FlagCollector(), name="fc")
        solver.run()

        assert len(flags) > 0
        for inside, outside in flags:
            assert isinstance(inside, bool)
            assert isinstance(outside, bool)

    def test_concentration_is_float(self):
        concentrations = []

        class _ConcCollector(SimulationObserver):
            def on_element(self, node):
                if node.is_inside:
                    concentrations.append(node.concentration(0))

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_ConcCollector(), name="conc")
        solver.run()

        assert len(concentrations) > 0
        assert isinstance(concentrations[0], float)

    def test_on_iteration_complete_is_called(self):
        completions = []

        class _IterTracker(SimulationObserver):
            def on_element(self, node): pass
            def on_iteration_complete(self):
                completions.append(True)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_element_observer(_IterTracker(), name="it")
        solver.run()

        assert len(completions) > 0

    def test_missing_abstract_method_raises(self):
        with pytest.raises(TypeError):
            class _Bad(SimulationObserver):
                pass
            _Bad()

    def test_multiple_observers(self):
        time_calls = []
        elem_calls = []

        class _TimeObs(TimeStepObserver):
            def on_time(self, t, gen, last, geo):
                time_calls.append(t)

        class _ElemObs(SimulationObserver):
            def on_element(self, node):
                elem_calls.append(True)

        solver = MovingBoundarySolver.from_xml(_MINIMAL_XML)
        solver.add_time_observer(_TimeObs(), name="to")
        solver.add_element_observer(_ElemObs(), name="eo")
        solver.run()

        assert len(time_calls) > 0
        assert len(elem_calls) > 0
