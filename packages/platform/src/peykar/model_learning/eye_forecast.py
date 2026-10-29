"""Scenario forecasts from a fitted generic eye model (version 2), and fit caching.

A *scenario* is one group (or one eye) with a context, a baseline value, an optional
group size, optional single-dose effect duration and optional observed follow-ups.
:func:`forecast_scenario` returns the tested primary forecasts of the model: the
initial-state prediction (Gaussian population prior, no observed follow-up) or the
causal forecast that assimilates the observed follow-ups (Gaussian population prior
updated by the observations), with predictive medians and 80% / 90% intervals in the
quantity's own unit and as fractions of the baseline.

:func:`fit_to_json` and :func:`fit_from_json` store a fitted population as plain JSON
(no pickle), so that an operator can reuse a fit; loading checks that the stored
configuration equals the given one. Nothing here reads files or claims clinical
validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field
from typing import Any

import numpy as np

from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning.eye_recovery_population import FamilyFit, NoiseCenters
from peykar_core.errors import InvalidRequest

__all__ = ["ScenarioSeries", "fit_from_json", "fit_to_json", "forecast_scenario"]

FIT_SCHEMA = "eye.recovery-fit-2.0.0"
QUANTILES = (0.05, 0.1, 0.5, 0.9, 0.95)
MAX_TARGETS = 200


@dataclass(frozen=True)
class ScenarioSeries:
    """The observed follow-ups of one scenario, in the generic series protocol."""

    series_id: str
    study_id: str
    context: str
    reference_value: float
    months: tuple[float, ...]
    fractions: tuple[float, ...]
    aggregate_n: int | None = None
    role: str = "control"
    metadata: Mapping[str, Any] = field(default_factory=dict)

    @property
    def kind(self) -> str:
        return "aggregate" if self.aggregate_n is not None else "individual"


def _positive(value: float, name: str) -> float:
    number = float(value)
    if not (math.isfinite(number) and number > 0):
        raise InvalidRequest(f"{name} must be finite and positive")
    return number


def forecast_scenario(
    config: population.RecoveryConfig,
    fit: population.RecoveryFit,
    *,
    context: str,
    baseline: float,
    months: Sequence[float],
    group_size: int | None = None,
    observed: Sequence[tuple[float, float]] = (),
    effect_duration_months: float | None = None,
) -> dict[str, Any]:
    """Primary forecasts of one scenario; ``observed`` holds (month, value) pairs in the
    quantity's own unit, after the baseline."""
    baseline = _positive(baseline, "The baseline value")
    targets = [_positive(m, "Every forecast month") for m in months]
    if not targets or len(targets) > MAX_TARGETS:
        raise InvalidRequest(f"Give 1..{MAX_TARGETS} forecast months")
    if group_size is not None and (isinstance(group_size, bool) or int(group_size) < 1):
        raise InvalidRequest("The group size must be a positive integer")
    n = None if group_size is None else int(group_size)
    duration = None
    if effect_duration_months is not None:
        duration = _positive(effect_duration_months, "The effect duration")
    pairs = sorted(
        (_positive(m, "An observed month"), _positive(v, "An observed value")) for m, v in observed
    )
    if len({m for m, _ in pairs}) != len(pairs):
        raise InvalidRequest("Observed months must be unique")
    series = ScenarioSeries(
        series_id="scenario/series",
        study_id="scenario",
        context=context,
        reference_value=baseline,
        months=tuple(m for m, _ in pairs),
        fractions=tuple(v / baseline for _, v in pairs),
        aggregate_n=n,
        metadata={"effect_duration_months": duration},
    )
    prior = fit.prior(context, series.kind, population.level_covariate(config, series))
    level_used = fit.level_used(context)
    variant = population.gaussian(prior["mean"], prior["covariance"])
    noise = fit.noise(series.kind, context, n)
    if pairs:
        fitted = population.posterior(config, variant, series=series, noise=noise)
        method = "population_assimilation"
    else:
        fitted = population.posterior(config, variant, series=None, noise=noise, duration=duration)
        method = "population_initial_state"
    result = population.predictive(
        config,
        fitted,
        months=targets,
        quantiles=QUANTILES,
        extra_noise=fit.forecast_inflation(series.kind, context, n),
    )
    quantiles = result["quantiles"]
    warnings = []
    if level_used != context:
        warnings.append(
            f"context {context!r} has no training series; the prior comes from {level_used}"
        )
    if n is None:
        warnings.append("single-eye forecasts were reported in the sealed tests, not tested")
    if pairs and min(targets) <= pairs[-1][0]:
        warnings.append("some forecast months are not after the last observed follow-up")
    return {
        "context": context,
        "level_used": level_used,
        "method": method,
        "point": "predictive median",
        "baseline": baseline,
        "group_size": n,
        "effect_duration_months": duration,
        "observed": [{"month": m, "value": v, "fraction": v / baseline} for m, v in pairs],
        "forecast": [
            {
                "month": month,
                "fraction": {
                    "point": quantiles["0.5"][i],
                    "interval_80": [quantiles["0.1"][i], quantiles["0.9"][i]],
                    "interval_90": [quantiles["0.05"][i], quantiles["0.95"][i]],
                },
                "value": {
                    "point": baseline * quantiles["0.5"][i],
                    "interval_80": [baseline * quantiles["0.1"][i], baseline * quantiles["0.9"][i]],
                    "interval_90": [
                        baseline * quantiles["0.05"][i],
                        baseline * quantiles["0.95"][i],
                    ],
                },
            }
            for i, month in enumerate(targets)
        ],
        "warnings": warnings,
    }


# --- plain-JSON storage of a fitted population -------------------------------------------


def _array(value: Any) -> list[Any]:
    return np.asarray(value, dtype=np.float64).tolist()  # type: ignore[no-any-return]


def _centers_to_json(centers: NoiseCenters | None) -> dict[str, Any] | None:
    if centers is None:
        return None
    return {
        "centers": [
            [family, kind, value] for (family, kind), value in sorted(centers.centers.items())
        ],
        "exponent": centers.exponent,
        "reference_n": centers.reference_n,
        "floor": centers.floor,
    }


def _centers_from_json(data: Mapping[str, Any] | None) -> NoiseCenters | None:
    if data is None:
        return None
    return NoiseCenters(
        centers={(family, kind): float(value) for family, kind, value in data["centers"]},
        exponent=float(data["exponent"]),
        reference_n=float(data["reference_n"]),
        floor=float(data["floor"]),
    )


def fit_to_json(fit: population.RecoveryFit, **provenance: Any) -> dict[str, Any]:
    """A fitted population as plain JSON, with the configuration and any provenance."""
    families = {}
    for name, family in fit.families.items():
        families[name] = {
            "family": family.family,
            "grouping": list(family.grouping),
            "fixed": _array(family.fixed),
            "variances": {level: _array(v) for level, v in family.variances.items()},
            "effect_index": [
                [level, key, offset] for (level, key), offset in sorted(family.effect_index.items())
            ],
            "solution": _array(family.solution),
            "covariance": _array(family.covariance),
            "studies": list(family.studies),
            "series": family.series,
            "objective": family.objective,
        }
    return {
        "schema": FIT_SCHEMA,
        "configuration": fit.config.describe(),
        "families": families,
        "noise_centers": _centers_to_json(fit.noise_centers),
        "forecast_noise_centers": _centers_to_json(fit.forecast_noise_centers),
        "study_noise_sd": dict(fit.study_noise_sd),
        "training_series": list(fit.training_series),
        "training_studies": list(fit.training_studies),
        "unfitted_families": dict(fit.unfitted_families),
        "provenance": provenance,
    }


def fit_from_json(
    config: population.RecoveryConfig, data: Mapping[str, Any]
) -> population.RecoveryFit:
    """Rebuild a stored fit; the stored configuration must equal ``config``."""
    if data.get("schema") != FIT_SCHEMA:
        raise InvalidRequest("Unknown stored fit schema")
    if data.get("configuration") != config.describe():
        raise InvalidRequest("The stored fit was made with another configuration")
    families = {}
    for name, item in data["families"].items():
        families[name] = FamilyFit(
            family=item["family"],
            grouping=tuple(item["grouping"]),
            fixed=np.asarray(item["fixed"], dtype=np.float64),
            variances={k: np.asarray(v, dtype=np.float64) for k, v in item["variances"].items()},
            effect_index={(level, key): int(offset) for level, key, offset in item["effect_index"]},
            solution=np.asarray(item["solution"], dtype=np.float64),
            covariance=np.asarray(item["covariance"], dtype=np.float64),
            studies=tuple(item["studies"]),
            series=int(item["series"]),
            objective=float(item["objective"]),
        )
    centers = _centers_from_json(data["noise_centers"])
    if centers is None:
        raise InvalidRequest("A stored fit needs its noise centres")
    return population.RecoveryFit(
        config=config,
        families=families,
        noise_centers=centers,
        study_noise_sd={k: float(v) for k, v in data["study_noise_sd"].items()},
        training_series=tuple(data["training_series"]),
        training_studies=tuple(data["training_studies"]),
        unfitted_families=dict(data["unfitted_families"]),
        forecast_noise_centers=_centers_from_json(data["forecast_noise_centers"]),
    )
