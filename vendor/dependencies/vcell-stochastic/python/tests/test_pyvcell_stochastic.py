"""Smoke tests run by cibuildwheel against each built wheel."""

from __future__ import annotations

import vcellstochastic
import vcellstochastic_py


def test_extension_classes_are_importable():
    assert vcellstochastic_py.StochModel().getNumOfVars() == 0
    assert vcellstochastic_py.MultiTrialStats(2, 5).getNumVars() == 2


def test_gibson_rng():
    gibson = vcellstochastic_py.Gibson()
    assert gibson.getNumOfVars() == 0
    assert 0.0 <= gibson.getRandomUniform() <= 1.0


def test_trial_stats_wrapper():
    stats = vcellstochastic.TrialStats(2, 5)
    assert stats.num_vars == 2
    # num_time_points counts the samples recorded so far, not the capacity.
    assert stats.num_time_points == 0

    stats.start_trial()
    stats.add_sample(0, 0.0, [1.0, 2.0])

    assert stats.num_time_points == 1
    assert stats.time_point(0) == 0.0
    assert stats.mean(0, 0) == 1.0
    assert stats.mean(1, 0) == 2.0
    assert stats.min(0, 0) == 1.0
    assert stats.max(1, 0) == 2.0


def test_gibson_solver_wrapper_delegates():
    solver = vcellstochastic.GibsonSolver.__new__(vcellstochastic.GibsonSolver)
    solver._solver = vcellstochastic_py.Gibson()
    assert solver.num_vars == 0
    assert solver.num_processes == 0
    assert 0.0 <= solver.random_uniform() <= 1.0
