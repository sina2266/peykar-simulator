"""
Python wrapper for the vcellstochastic_py bindings.
"""
from vcellstochastic_py import Gibson, MultiTrialStats, StochModel


class GibsonSolver:
    """
    Wraps the Gibson (Next Reaction Method) stochastic solver.

    Parameters
    ----------
    input_file : str
        Path to the model input file.
    output_file : str
        Path where simulation output will be written.
    """

    def __init__(self, input_file: str, output_file: str):
        self._solver = Gibson(input_file, output_file)

    def run(self) -> int:
        """Run the full simulation. Returns the Gibson core() exit code."""
        return self._solver.core()

    def march(self):
        """Advance the simulation by one step."""
        self._solver.march()

    def random_uniform(self) -> float:
        """Return a uniform random sample from the solver's RNG."""
        return self._solver.getRandomUniform()

    @property
    def num_vars(self) -> int:
        return self._solver.getNumOfVars()

    @property
    def num_processes(self) -> int:
        return self._solver.getNumOfProcesses()

    def var_index(self, name: str) -> int:
        return self._solver.getVarIndex(name)

    def process_index(self, name: str) -> int:
        return self._solver.getProcessIndex(name)


class TrialStats:
    """
    Wraps MultiTrialStats for accumulating statistics across simulation trials.

    Parameters
    ----------
    num_vars : int
        Number of state variables tracked.
    num_time_points : int
        Number of time points per trial.
    """

    def __init__(self, num_vars: int, num_time_points: int):
        self._stats = MultiTrialStats(num_vars, num_time_points)

    def start_trial(self):
        """Signal the start of a new trial."""
        self._stats.startNewTrial()

    def add_sample(self, time_index: int, time_value: float, var_vals: list):
        """
        Record one sample.

        Parameters
        ----------
        time_index : int
        time_value : float
        var_vals : list of float
            Values for each variable at this time point.
        """
        self._stats.addSample(time_index, time_value, var_vals)

    def mean(self, var_index: int, time_index: int) -> float:
        return self._stats.getMean(var_index, time_index)

    def variance(self, var_index: int, time_index: int) -> float:
        return self._stats.getVariance(var_index, time_index)

    def min(self, var_index: int, time_index: int) -> float:
        return self._stats.getMin(var_index, time_index)

    def max(self, var_index: int, time_index: int) -> float:
        return self._stats.getMax(var_index, time_index)

    @property
    def num_vars(self) -> int:
        return self._stats.getNumVars()

    @property
    def num_time_points(self) -> int:
        return self._stats.getNumTimePoints()

    def time_point(self, time_index: int) -> float:
        return self._stats.getTimePoint(time_index)

    def write_output(self, output_file: str, var_names: list):
        """
        Write accumulated statistics to an HDF5 file.

        Parameters
        ----------
        output_file : str
        var_names : list of str
        """
        self._stats.writeOutput(output_file, var_names)
