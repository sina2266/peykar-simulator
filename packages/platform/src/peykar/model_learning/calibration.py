import math

from peykar_core.contracts import DatasetSpec, Observation, Quantity
from peykar_core.errors import InvalidRequest
from peykar_core.units import convert


def estimate_decay_rate(dataset: DatasetSpec, observation: Observation) -> Quantity:
    """Unweighted log-linear estimate; no uncertainty or heteroscedastic correction.

    A calibration proposal, not a validated biological parameter estimate.
    """
    if dataset.role != "calibration":
        raise InvalidRequest("Calibration may only read a calibration dataset")
    if observation.endpoint != "concentration":
        raise InvalidRequest("Decay calibration needs a concentration endpoint")
    t = [
        convert(Quantity(value=v, unit=observation.time.unit), "second")
        for v in observation.time.values
    ]
    y = [
        convert(Quantity(value=v, unit=observation.values.unit), "mole/liter")
        for v in observation.values.values
    ]
    if len(t) < 3 or len(t) != len(y) or any(v <= 0 for v in y):
        raise InvalidRequest("Need at least three aligned positive concentrations")
    mean_t, mean_y = math.fsum(t) / len(t), math.fsum(map(math.log, y)) / len(y)
    denominator = math.fsum((v - mean_t) ** 2 for v in t)
    if denominator <= 0:
        raise InvalidRequest("Calibration times have no variation")
    slope = (
        math.fsum((a - mean_t) * (math.log(b) - mean_y) for a, b in zip(t, y, strict=True))
        / denominator
    )
    if slope > 0:
        raise InvalidRequest("Data support growth, not nonnegative decay")
    return Quantity(value=-slope, unit="1/second")
