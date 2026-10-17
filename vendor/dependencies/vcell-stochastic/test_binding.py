import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "python", "src"))

from vcellstochastic import GibsonSolver, TrialStats


def test_gibson_solver():
    print("Testing GibsonSolver...")
    solver = GibsonSolver.__new__(GibsonSolver)
    import vcellstochastic_py
    solver._solver = vcellstochastic_py.Gibson()
    assert solver.num_vars == 0
    assert solver.num_processes == 0
    r = solver.random_uniform()
    print(f"Random uniform value: {r}")
    assert 0.0 <= r <= 1.0
    print("GibsonSolver tests passed.")


def test_trial_stats():
    print("Testing TrialStats...")
    stats = TrialStats(2, 5)
    assert stats.num_vars == 2

    stats.start_trial()
    stats.add_sample(0, 0.0, [1.0, 2.0])

    assert stats.num_time_points == 1
    assert stats.time_point(0) == 0.0
    assert stats.mean(0, 0) == 1.0
    assert stats.mean(1, 0) == 2.0
    print(f"Mean at t=0, var=0: {stats.mean(0, 0)}")
    print("TrialStats tests passed.")


if __name__ == "__main__":
    test_gibson_solver()
    test_trial_stats()
    print("All tests passed!")
