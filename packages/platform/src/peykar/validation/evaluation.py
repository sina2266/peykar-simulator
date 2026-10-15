import math

from peykar_core.contracts import (
    DatasetSpec,
    Observation,
    Quantity,
    Reference,
    RunManifest,
    SimulationResult,
    ValidationReport,
)
from peykar_core.errors import InvalidRequest
from peykar_core.units import convert


def evaluate(
    manifest: RunManifest,
    result: SimulationResult,
    dataset: DatasetSpec,
    observation: Observation,
    tolerance: Quantity,
    calibration_groups: frozenset[str] = frozenset(),
) -> ValidationReport:
    if dataset.role == "calibration" or dataset.independence_group in calibration_groups:
        raise InvalidRequest(
            "Calibration data or its related group cannot be used as held-out data"
        )
    if dataset.context != manifest.plan.context:
        raise InvalidRequest("Dataset and model contexts differ")
    if observation.dataset != Reference(id=dataset.id, version=dataset.version):
        raise InvalidRequest("Observation references a different dataset")
    output = next((o for o in result.outputs if o.name == observation.endpoint), None)
    if output is None:
        raise InvalidRequest("No observation operator exists for this endpoint")
    observed_times = tuple(
        convert(Quantity(value=t, unit=observation.time.unit), result.time.unit)
        for t in observation.time.values
    )
    if len(observed_times) != len(result.time.values) or any(
        not math.isclose(a, b, rel_tol=1e-9, abs_tol=1e-9)
        for a, b in zip(observed_times, result.time.values, strict=True)
    ):
        raise InvalidRequest("Time grids differ; an explicit observation operator is required")
    if len(observation.values.values) != len(output.values):
        raise InvalidRequest("Observation lengths differ")
    measured = tuple(
        convert(Quantity(value=v, unit=observation.values.unit), output.unit)
        for v in observation.values.values
    )
    errors = tuple(
        predicted - actual for predicted, actual in zip(output.values, measured, strict=True)
    )
    threshold = convert(tolerance, output.unit)
    if threshold < 0:
        raise InvalidRequest("Tolerance must be nonnegative")
    rmse = math.sqrt(math.fsum(e * e for e in errors) / len(errors))
    independent = dataset.origin == "experimental" and dataset.role == "validation"
    return ValidationReport(
        model=manifest.plan.model,
        dataset=observation.dataset,
        endpoint=observation.endpoint,
        n=len(errors),
        rmse=rmse,
        max_absolute_error=max(abs(e) for e in errors),
        unit=output.unit,
        tolerance=threshold,
        passed=rmse <= threshold,
        context=dataset.context,
        evidence_class=(
            "independent_experimental_validation" if independent else "synthetic_verification"
        ),
        limitations=(
            "Dataset independence and provenance require external review.",
            "This report does not automatically promote model validity.",
        ),
    )
