"""Bounded offline learning for a dose-conditioned corneal projected front.

This constructs a new empirical candidate from explicitly supplied calibration
animals. It never registers a model, runs remote code, reads a holdout implicitly
or grants biological validity. Initial-state prediction uses radius, time and
reported SDP concentration; observed post-injury area is never a predictor.
"""

import math
import re
from collections.abc import Callable, Mapping, Sequence
from dataclasses import asdict, dataclass
from hashlib import sha256
from typing import Any, cast

from peykar_core.errors import InvalidRequest

TRAINING_GROUPS = ("Control", "2% SDP")
REPLICATES = ("1", "2", "3")
HOURS = tuple(range(0, 55, 6))
MAX_EVALUATIONS = 45_000
REFINEMENT_STEPS = ((0.1, 0.025), (0.01, 0.0025), (0.001, 0.00025))


def _number(value: object, name: str, lower: float, upper: float) -> float:
    if type(value) not in (int, float):
        raise InvalidRequest(f"{name} requires a finite number in [{lower},{upper}]")
    number = float(cast(float, value))
    if not math.isfinite(number) or not lower <= number <= upper:
        raise InvalidRequest(f"{name} requires a finite number in [{lower},{upper}]")
    return number


@dataclass(frozen=True)
class CornealFrontParameters:
    rate0_um_per_h: float
    rate2_um_per_h: float
    lag0_h: float
    lag2_h: float

    def __post_init__(self) -> None:
        for name in ("rate0_um_per_h", "rate2_um_per_h"):
            _number(getattr(self, name), name, 0, 200)
        for name in ("lag0_h", "lag2_h"):
            _number(getattr(self, name), name, 0, 24)


def _area(radius: float, hour: float, rate: float, lag: float) -> float:
    return (max(0.0, radius - rate * max(0.0, hour - lag)) / radius) ** 2


def predict_open_area_fraction(
    parameters: CornealFrontParameters,
    *,
    initial_radius_um: float,
    hour: float,
    concentration_wv_percent: float,
) -> float:
    """Predict from the declared initial condition, not from a later observation.

    Dose interpolation is an explicit empirical assumption within [0,2] wt/vol%;
    neither a monotone drug effect nor extrapolation is silently imposed.
    """
    radius = _number(initial_radius_um, "initial_radius_um", 0, 10_000)
    if radius == 0:
        raise InvalidRequest("initial_radius_um must be positive")
    time = _number(hour, "hour", 0, 72)
    dose = _number(concentration_wv_percent, "concentration_wv_percent", 0, 2) / 2
    rate = parameters.rate0_um_per_h + dose * (
        parameters.rate2_um_per_h - parameters.rate0_um_per_h
    )
    lag = parameters.lag0_h + dose * (parameters.lag2_h - parameters.lag0_h)
    return _area(radius, time, rate, lag)


def corneal_learning_algorithm() -> dict[str, Any]:
    """Exact algorithm identity for a protocol frozen before any actual fit."""
    return {
        "id": "cornea.dose_conditioned_front.grid_refinement",
        "version": "1.0.0",
        "objective": "Mean of per-animal means of squared normalized AREA residuals",
        "baseline_in_fit": False,
        "observation_hours": list(HOURS[1:]),
        "endpoint_fits": "Independent Control and2%SDP endpoint fits; equal animal weights",
        "coarse_grid": {
            "rate_um_per_h": {"start": 0, "stop": 200, "step": 1},
            "lag_h": {"start": 0, "stop": 24, "step": 0.25},
            "evaluations_per_group": 19_497,
        },
        "refinements": [
            {"rate_step_um_per_h": rate, "lag_step_h": lag, "offsets": list(range(-10, 11))}
            for rate, lag in REFINEMENT_STEPS
        ],
        "refinement_center": "Previous stage best; fixed throughout the next complete stage",
        "outside_bounds": "Skip outside inclusive bounds; never clip or rescale observations",
        "order": "Rate outer ascending; lag inner ascending, at every stage",
        "selection": "Lexicographic minimum of (objective,rate,lag), no tolerance or early stop",
        "counting": "Count every actual objective evaluation, including repeated grid centers",
        "maximum_algorithm_evaluations": 41_640,
        "operator_evaluation_cap": MAX_EVALUATIONS,
        "bounds": {"rate_um_per_h": [0, 200], "lag_h": [0, 24]},
        "parameter_interpolation": "Linear in reported concentration from0to2wt/volpercent",
        "no_hyperparameter_search": True,
    }


@dataclass(frozen=True)
class CornealFrontFit:
    parameters: CornealFrontParameters
    initial_radius_um: float
    evaluation_count: int
    endpoint_fits: tuple[dict[str, Any], ...]
    calibration_source_sha256: str
    protocol_sha256: str
    normalized_calibration_sha256: str
    provenance: dict[str, str]

    def as_artifact(self) -> dict[str, Any]:
        """Return a portable calibration receipt, never a validation approval."""
        return {
            "schema_version": "1.0.0",
            "class": "constrained_empirical_calibration_not_validation",
            "parameters": asdict(self.parameters),
            "initial_radius_um": self.initial_radius_um,
            "algorithm": corneal_learning_algorithm(),
            "evaluation_count": self.evaluation_count,
            "endpoint_fits": list(self.endpoint_fits),
            "objective_mean_square_area_fraction": math.fsum(
                f["objective_mean_square_area_fraction"] for f in self.endpoint_fits
            )
            / len(self.endpoint_fits),
            "training_group_ids": list(TRAINING_GROUPS),
            "training_animals": [
                [group, replicate] for group in TRAINING_GROUPS for replicate in REPLICATES
            ],
            "sample_counts": {
                "actual_animals": 6,
                "rows_including_baseline": 60,
                "fitted_nonbaseline_values": 54,
                "fitted_times_per_animal": 9,
                "groups": 2,
            },
            "calibration_source_sha256": self.calibration_source_sha256,
            "protocol_sha256": self.protocol_sha256,
            "normalized_calibration_sha256": self.normalized_calibration_sha256,
            "provenance": dict(self.provenance),
            "biological_validation": False,
            "limitations": [
                "Six calibration animals with repeated observations are not54independent animals.",
                "Four cohort parameters; no identified causal drug/substrate mechanism.",
                "Dose interpolation is empirical; treatment effect need not be monotone.",
                "No cell-layer, chemical injury, barrier or clinical validation.",
                "Heldout animals and independent studies require separate frozen evaluation.",
            ],
        }


def _validated_rows(
    observations: Sequence[Mapping[str, object]],
) -> tuple[dict[str, tuple[tuple[float, ...], ...]], str]:
    import json

    if len(observations) != 60:
        raise InvalidRequest(
            "Calibration requires exactly60 complete rows; never pass heldout rows"
        )
    values: dict[tuple[str, str, int], float] = {}
    for row in observations:
        group, replicate = row.get("group"), row.get("replicate")
        if group not in TRAINING_GROUPS:
            raise InvalidRequest(
                "Only Control/2%SDP training groups allowed; heldout leakage rejected"
            )
        if not isinstance(group, str) or replicate not in REPLICATES:
            raise InvalidRequest("Calibration requires exact actual rabbit identifiers1/2/3")
        if not isinstance(replicate, str):
            raise InvalidRequest("Calibration rabbit identifiers must be strings")
        time = _number(row.get("hour"), "hour", 0, 54)
        if time not in HOURS:
            raise InvalidRequest("Calibration requires the complete0..54h grid in6h steps")
        area = _number(row.get("wound_area_fraction"), "wound_area_fraction", 0, 4)
        key = (group, replicate, int(time))
        if key in values:
            raise InvalidRequest("Duplicate calibration group/rabbit/time observation")
        if time == 0 and area != 1:
            raise InvalidRequest("Each actual rabbit requires a normalized baseline exactly1")
        values[key] = area
    expected = {(g, r, h) for g in TRAINING_GROUPS for r in REPLICATES for h in HOURS}
    if set(values) != expected:
        raise InvalidRequest("Missing calibration group/rabbit/time observation")
    normalized = [
        {"group": g, "replicate": r, "hour": h, "wound_area_fraction": values[g, r, h]}
        for g in TRAINING_GROUPS
        for r in REPLICATES
        for h in HOURS
    ]
    digest = sha256(
        json.dumps(normalized, sort_keys=True, separators=(",", ":"), allow_nan=False).encode()
    ).hexdigest()
    grouped = {
        g: tuple(tuple(values[g, r, h] for h in HOURS[1:]) for r in REPLICATES)
        for g in TRAINING_GROUPS
    }
    return grouped, digest


def fit_corneal_front(
    observations: Sequence[Mapping[str, object]],
    *,
    initial_radius_um: float,
    calibration_source_sha256: str,
    protocol_sha256: str,
    provenance: Mapping[str, str],
    checkpoint: Callable[[], None],
    max_evaluations: int = MAX_EVALUATIONS,
) -> CornealFrontFit:
    """Fit exactly the declared endpoint groups, rejecting any holdout row.

    The caller must review and freeze the protocol, isolate calibration rows and
    bound wall time. Checkpoint/cancellation failures propagate without returning
    a partial or successful fit. No experimental default or automatic execution
    is provided.
    """
    checkpoint()
    for digest in (calibration_source_sha256, protocol_sha256):
        if not isinstance(digest, str) or re.fullmatch(r"[0-9a-f]{64}", digest) is None:
            raise InvalidRequest(
                "Calibration source and frozen protocol need exact SHA256 identities"
            )
    if not provenance or any(
        not isinstance(k, str) or not isinstance(v, str) or not k.strip() or not v.strip()
        for k, v in provenance.items()
    ):
        raise InvalidRequest("Calibration requires explicit nonempty source/context provenance")
    if type(max_evaluations) is not int or not 1 <= max_evaluations <= MAX_EVALUATIONS:
        raise InvalidRequest("Calibration objective evaluation cap must be within1..45000")
    radius = _number(initial_radius_um, "initial_radius_um", 0, 10_000)
    if radius == 0:
        raise InvalidRequest("initial_radius_um must be positive")
    grouped, calibration_digest = _validated_rows(observations)
    count = 0
    endpoint_fits: list[dict[str, Any]] = []
    for group in TRAINING_GROUPS:
        start_count = count
        observed = grouped[group]

        def evaluate(
            rate: float, lag: float, observed: tuple[tuple[float, ...], ...] = observed
        ) -> tuple[float, float, float]:
            nonlocal count
            checkpoint()
            if count >= max_evaluations:
                raise InvalidRequest(
                    "Calibration objective evaluation cap reached; no fit produced"
                )
            predicted = tuple(_area(radius, h, rate, lag) for h in HOURS[1:])
            objective = (
                math.fsum(
                    math.fsum((a - b) ** 2 for a, b in zip(predicted, rabbit, strict=True)) / 9
                    for rabbit in observed
                )
                / 3
            )
            count += 1
            return objective, rate, lag

        best = (math.inf, 0.0, 0.0)
        for rate_index in range(201):
            for lag_index in range(97):
                best = min(best, evaluate(float(rate_index), lag_index / 4))
        for rate_step, lag_step in REFINEMENT_STEPS:
            center_rate, center_lag = best[1:]
            for rate_index in range(-10, 11):
                rate = center_rate + rate_index * rate_step
                if not 0 <= rate <= 200:
                    continue
                for lag_index in range(-10, 11):
                    lag = center_lag + lag_index * lag_step
                    if 0 <= lag <= 24:
                        best = min(best, evaluate(rate, lag))
        endpoint_fits.append(
            {
                "group": group,
                "rate_um_per_h": best[1],
                "lag_h": best[2],
                "objective_mean_square_area_fraction": best[0],
                "evaluation_count": count - start_count,
                "actual_rabbit_ids": list(REPLICATES),
                "fitted_nonbaseline_values": 27,
            }
        )
    checkpoint()
    return CornealFrontFit(
        parameters=CornealFrontParameters(
            rate0_um_per_h=endpoint_fits[0]["rate_um_per_h"],
            rate2_um_per_h=endpoint_fits[1]["rate_um_per_h"],
            lag0_h=endpoint_fits[0]["lag_h"],
            lag2_h=endpoint_fits[1]["lag_h"],
        ),
        initial_radius_um=radius,
        evaluation_count=count,
        endpoint_fits=tuple(endpoint_fits),
        calibration_source_sha256=calibration_source_sha256,
        protocol_sha256=protocol_sha256,
        normalized_calibration_sha256=calibration_digest,
        provenance=dict(provenance),
    )
