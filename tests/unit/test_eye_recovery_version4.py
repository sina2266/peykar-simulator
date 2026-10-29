"""Generic eye model version 4 (correlated random effects), harness 4, additive evaluation 2
and the acuity 1.1 candidates."""

from __future__ import annotations

import math
from typing import Any, cast

import numpy as np
import pytest

from peykar.model_learning import acuity_population as acuity10
from peykar.model_learning import acuity_population_1_1 as acuity11
from peykar.model_learning import eye_recovery_population2 as version2
from peykar.model_learning import eye_recovery_population4 as version4
from peykar.validation import eye_additive_evaluation as additive1
from peykar.validation import eye_additive_evaluation2 as additive2
from peykar.validation import eye_recovery_evaluation3 as evaluation3
from peykar.validation import eye_recovery_evaluation4 as evaluation4
from peykar.validation.eye_retina_data import RetinaSeries
from peykar_core.errors import InvalidRequest

BASE = acuity10.DEVELOPMENT_CANDIDATES["a0"]
MONTHS = (1.0, 3.0, 6.0, 12.0, 24.0)
SCALE = additive1.AdditiveScale(
    quantity="acuity", unit="logMAR", model_unit=1.0, covariate_unit=1.0, report_unit=0.02
)


def _fields(config: version2.RecoveryConfig) -> dict[str, Any]:
    return {name: getattr(config, name) for name in config.__dataclass_fields__}


def _config(
    correlation: str, slope_groups: tuple[tuple[str, tuple[str, ...]], ...] = ()
) -> version4.RecoveryConfig:
    return version4.RecoveryConfig(
        **{**_fields(BASE), "random_correlation": correlation, "slope_groups": slope_groups}
    )


def _series(
    rho: float, seed: int = 5, slopes: tuple[float, float] = (-0.6, -0.6)
) -> list[RetinaSeries]:
    """Two contexts, six studies each, two group-mean series per study; the study-level
    deviations of (level, excess) have correlation ``rho``; ``slopes`` are the level
    slopes on logMAR0 of cxl and dmek."""
    rng = np.random.default_rng(seed)
    contexts = {"human/cxl": (-0.05, 0.04, 0.0, 3.0), "human/dmek": (-0.5, 0.3, 0.0, 2.0)}
    covariance = 0.01 * np.array([[1.0, rho], [rho, 1.0]])
    out = []
    for c, (context, (level, excess, transient, tau)) in enumerate(sorted(contexts.items())):
        for s in range(6):
            study = f"acu-v4{c}{s}"
            shift = rng.multivariate_normal([0.0, 0.0], covariance)
            for k in range(2):
                logmar0 = float(rng.uniform(0.3, 1.2))
                beta = np.array(
                    [level + slopes[c] * logmar0 + shift[0], excess + shift[1], transient],
                    dtype=float,
                )
                law = version2.design(BASE, MONTHS, np.array([math.log(tau)]))[0] @ beta
                values = np.exp(law + rng.normal(0.0, 0.02, len(MONTHS)))
                out.append(
                    RetinaSeries(
                        series_id=f"{study}/{k}",
                        study_id=study,
                        context=context,
                        reference_value=math.exp(logmar0),
                        months=MONTHS,
                        fractions=tuple(float(v) for v in values),
                        aggregate_n=30,
                    )
                )
    return out


def test_configuration() -> None:
    config = _config("level_excess", (("crosslinking", ("cxl", "other")),))
    described = config.describe()
    assert described["generic_model_version"] == "4.0.0"
    assert described["random_correlation"] == "level_excess"
    assert described["slope_groups"] == {"crosslinking": ["cxl", "other"]}
    assert described["point"] == "median"
    assert version4.slope_group(config, "human/other") == "crosslinking"
    assert version4.slope_group(config, "human/dmek") == "default"
    assert not _config("none").uses_version_4_fit
    for bad in (
        (("default", ("cxl",)),),
        (("a", ("cxl",)), ("b", ("cxl",))),
        (("a", ("cxl",)), ("a", ("pk",))),
        (("a", ()),),
    ):
        with pytest.raises(InvalidRequest):
            _config("none", bad)
    with pytest.raises(InvalidRequest):
        _config("full")


def test_slope_group_without_members_is_version_2_fit() -> None:
    series = _series(0.0)
    two = version2.fit_population(BASE, series)
    four = version4.fit_population(_config("none", (("graft", ("pk",)),)), series)
    assert isinstance(four, version4.RecoveryFit)
    fitted, other = two.families["human/all"], four.families["human/all"]
    assert np.array_equal(fitted.solution, other.solution)
    for level, matrix in fitted.variances.items():
        assert np.array_equal(matrix, other.variances[level])
    assert four.prior("human/dmek", "aggregate", 0.7)["mean"] == pytest.approx(
        two.prior("human/dmek", "aggregate", 0.7)["mean"]
    )


def test_slope_groups_fit_their_own_slopes() -> None:
    series = _series(0.0, slopes=(-0.1, -0.8))
    one = version4.fit_population(_config("none", (("graft", ("pk",)),)), series)
    two = version4.fit_population(_config("none", (("crosslinking", ("cxl",)),)), series)
    slopes = two.describe()["level_slopes"]["human/all"]
    assert slopes["crosslinking"]["slope"] == pytest.approx(-0.1, abs=0.1)
    assert slopes["default"]["slope"] == pytest.approx(-0.8, abs=0.1)
    shared = one.describe()["level_slopes"]["human/all"]["default"]["slope"]
    assert slopes["crosslinking"]["slope"] > shared > slopes["default"]["slope"]
    low = two.prior("human/cxl", "aggregate", 0.3)["mean"][0]
    high = two.prior("human/cxl", "aggregate", 1.1)["mean"][0]
    assert high - low == pytest.approx(0.8 * slopes["crosslinking"]["slope"])


def test_none_is_version_2_fit() -> None:
    series = _series(0.0)
    two = version2.fit_population(BASE, series)
    four = version4.fit_population(_config("none"), series)
    assert type(four) is version2.RecoveryFit
    for family, fitted in two.families.items():
        other = four.families[family]
        assert np.array_equal(fitted.solution, other.solution)
        for level, matrix in fitted.variances.items():
            assert np.array_equal(matrix, other.variances[level])
    with pytest.raises(InvalidRequest):
        version4.fit_population(cast(Any, BASE), series)


def test_level_excess_correlation_is_fitted() -> None:
    series = _series(0.95)
    fit = version4.fit_population(_config("level_excess"), series)
    assert isinstance(fit, version4.RecoveryFit)
    fitted = fit.families["human/all"]
    rho = version4.level_excess_correlation(fitted.variances["study"])
    assert 0.0 < rho <= version4.RHO_MAX
    for level in version4.CORRELATED_LEVELS:
        assert version4.level_excess_correlation(fitted.variances[level]) == pytest.approx(rho)
    assert (
        np.count_nonzero(
            fitted.variances["context"] - np.diag(np.diag(fitted.variances["context"]))
        )
        == 0
    )
    prior = fit.prior("human/dmek", "aggregate", 0.8)
    assert prior["covariance"][0, 1] > 0.0
    described = fit.describe()
    assert described["level_excess_correlation"]["human/all"]["study"] == pytest.approx(rho)


def _strip(report: dict[str, Any]) -> dict[str, Any]:
    return {
        **report,
        "evaluation_version": None,
        "studies": [{**s, "evaluation_version": None} for s in report["studies"]],
    }


def test_harness_4_with_none_equals_harness_3() -> None:
    series = _series(0.3)
    config = _config("none")
    three = evaluation3.leave_one_study_out(config, series, studies=["acu-v400"])
    four = evaluation4.leave_one_study_out(config, series, studies=["acu-v400"])
    assert four["evaluation_version"] == "4.0.0"
    assert _strip(three) == _strip(four)
    with pytest.raises(InvalidRequest):
        evaluation4.evaluate_study(cast(Any, BASE), study_series=series[:2], training=series[2:])


def test_additive_evaluation_2_dispatch() -> None:
    series = _series(0.3)
    one = additive1.leave_one_study_out(BASE, series, scale=SCALE, studies=["acu-v410"])
    two = additive2.leave_one_study_out(BASE, series, scale=SCALE, studies=["acu-v410"])
    assert two["harness_version"] == "2.0.0" and two["evaluation_version"] == "2.0.0"
    assert two["studies"][0]["additive_evaluation_version"] == "2.0.0"
    stripped = {**two["studies"][0], "additive_evaluation_version": "1.0.0"}
    assert stripped == one["studies"][0]
    assert two["summary"] == one["summary"]
    four = additive2.leave_one_study_out(
        _config("level_excess"), series, scale=SCALE, studies=["acu-v410"]
    )
    assert four["harness_version"] == "4.0.0"
    fit = four["studies"][0]["population_fit"]
    assert "level_excess_correlation" in fit


def test_acuity_1_1_candidates() -> None:
    candidates = acuity11.DEVELOPMENT_CANDIDATES
    assert candidates["a0"] is acuity10.DEVELOPMENT_CANDIDATES["a0"]
    assert acuity11.REFERENCE == "a0"
    for name in candidates:
        assert acuity11.adapter(name) is acuity10.adapter("a0")
    with pytest.raises(KeyError):
        acuity11.adapter("zz")
    a0 = _fields(BASE)
    for name, options in (
        ("b1", {"random_correlation": "level_excess", "slope_groups": ()}),
        ("b2", {"random_correlation": "none", "slope_groups": acuity11.CROSSLINKING_SLOPE}),
    ):
        config = candidates[name]
        assert isinstance(config, version4.RecoveryConfig)
        fields = _fields(config)
        assert fields.pop("model_version") == f"1.1.0-{name}"
        assert fields.pop("point") == "median"
        assert {k: fields.pop(k) for k in options} == options
        assert fields == {k: v for k, v in a0.items() if k != "model_version"}
