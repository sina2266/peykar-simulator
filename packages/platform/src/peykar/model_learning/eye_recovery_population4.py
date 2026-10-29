"""Population model of an eye measurement after treatment, generic version 4.

Generic model 3 (``eye_recovery_population3``, unchanged, with the law, posterior and
predictive distribution of version 2, which protocols pin) with two options for the
population hierarchy. With both at their defaults the fit is version 2's, unchanged.

``random_correlation``:

- ``"none"`` (versions 1–3): every random level (the grouping levels, the study and the
  group-mean and single-eye series) has a diagonal covariance, so the long-term level
  ``ell`` and the transient amplitude ``e`` of a group vary independently.
- ``"level_excess"``: the study level and the two series levels share one correlation
  coefficient ``rho`` between ``ell`` and ``e``. It is estimated in the same penalized
  REML as the variances: ``rho = 0.95 tanh(phi)``, with a standard normal penalty on
  ``phi`` that starts at ``phi = 0`` (the independent model). The grouping levels stay
  diagonal. A group's deviation from the law is ``d_ell + r(t) d_e`` at time ``t``
  (``r`` falls from 1 to 0), so an early deviation and the long-term deviation have
  covariance ``var_ell + r(t) rho sd_ell sd_e``: the fitted ``rho`` decides how much of
  an early shortfall (or gain) a forecast expects to persist, where the independent
  model fixes ``rho = 0``.

``slope_groups`` (default ``()``: one slope): named groups of treatments (the last part
of a context) whose long-term level gets its own slope on the baseline covariate,
``ell = context level + b_g x``; every other treatment shares the default slope. A group
without a training series in a family uses the default slope there (or, without a
default member, the first fitted slope).

Nothing here reads files, registers models or claims clinical validity.
"""

from __future__ import annotations

import math
from collections.abc import Mapping, Sequence
from dataclasses import dataclass, field, replace
from typing import Any, cast

import numpy as np

from peykar.model_learning import eye_recovery_population as base
from peykar.model_learning import eye_recovery_population2 as version2
from peykar.model_learning import eye_recovery_population3 as version3
from peykar.model_learning.corneal_population import nelder_mead
from peykar.model_learning.eye_recovery_population import FloatArray, RecoverySeries
from peykar_core.errors import InvalidRequest

__all__ = [
    "CORRELATED_LEVELS",
    "CORRELATIONS",
    "DEFAULT_SLOPE",
    "GENERIC_VERSION",
    "FamilyFit",
    "RecoveryConfig",
    "RecoveryFit",
    "fit_family",
    "fit_population",
    "level_excess_correlation",
    "slope_group",
]

GENERIC_VERSION = "4.0.0"
CORRELATIONS = {"none", "level_excess"}
CORRELATED_LEVELS = base.SERIES_LEVELS
RHO_MAX = 0.95
DEFAULT_SLOPE = "default"


@dataclass(frozen=True)
class RecoveryConfig(version3.RecoveryConfig):
    """Version 3's configuration plus the random-effect correlation and slope groups."""

    random_correlation: str = "none"
    slope_groups: tuple[tuple[str, tuple[str, ...]], ...] = ()

    def __post_init__(self) -> None:
        super().__post_init__()
        if self.random_correlation not in CORRELATIONS:
            raise InvalidRequest(f"random_correlation must be one of {sorted(CORRELATIONS)}")
        names = [name for name, _ in self.slope_groups]
        treatments = [t for _, group in self.slope_groups for t in group]
        if (
            DEFAULT_SLOPE in names
            or len(set(names)) != len(names)
            or len(set(treatments)) != len(treatments)
            or any(not name or not group for name, group in self.slope_groups)
        ):
            raise InvalidRequest("Slope groups need distinct names and disjoint treatments")

    @property
    def uses_version_4_fit(self) -> bool:
        return self.random_correlation != "none" or bool(self.slope_groups)

    def describe(self) -> dict[str, Any]:
        return {
            **super().describe(),
            "generic_model_version": GENERIC_VERSION,
            "random_correlation": self.random_correlation,
            "slope_groups": {name: list(group) for name, group in self.slope_groups},
        }


def slope_group(config: RecoveryConfig, context: str) -> str:
    """The slope group of ``context`` (``"default"`` unless its treatment is listed)."""
    treatment = base.treatment_of(context)
    for name, group in config.slope_groups:
        if treatment in group:
            return name
    return DEFAULT_SLOPE


@dataclass(frozen=True)
class FamilyFit(base.FamilyFit):
    """Version 1's family fit plus the fixed-effect column of each slope group."""

    slope_columns: Mapping[str, int] = field(default_factory=dict)

    def slope_column(self, group: str) -> int:
        if group in self.slope_columns:
            return self.slope_columns[group]
        if DEFAULT_SLOPE in self.slope_columns:
            return self.slope_columns[DEFAULT_SLOPE]
        return min(self.slope_columns.values())


def _correlated(log_sd: FloatArray, rho: float) -> FloatArray:
    matrix = base._diagonal(log_sd)
    matrix[0, 1] = matrix[1, 0] = rho * math.exp(float(log_sd[0] + log_sd[1]))
    return matrix


def level_excess_correlation(matrix: FloatArray) -> float:
    """The correlation of ``ell`` and ``e`` in one level's covariance matrix."""
    return float(matrix[0, 1] / math.sqrt(matrix[0, 0] * matrix[1, 1]))


def _slope_columns(
    config: RecoveryConfig, summaries: Sequence[Mapping[str, Any]]
) -> dict[str, int]:
    """One fixed-effect column per slope group with a member, after the ``d`` means."""
    present = {slope_group(config, m["context"]) for m in summaries}
    order = [DEFAULT_SLOPE, *(name for name, _ in config.slope_groups)]
    d = config.dimensions
    return {name: d + k for k, name in enumerate(n for n in order if n in present)}


def _blocks(
    config: RecoveryConfig, summaries: Sequence[Mapping[str, Any]], columns: Mapping[str, int]
) -> list[Any]:
    blocks = base._blocks(config, summaries)
    d = config.dimensions
    width = d + len(columns)

    def design(member: Mapping[str, Any]) -> FloatArray:
        matrix = np.zeros((d, width))
        matrix[:, :d] = np.eye(d)
        matrix[0, columns[slope_group(config, member["context"])]] = member["covariate"]
        return matrix

    return [
        replace(
            block,
            fixed_design=np.concatenate([design(summaries[i]) for i in block.members]),
        )
        for block in blocks
    ]


def fit_family(
    config: RecoveryConfig,
    family: str,
    summaries: Sequence[Mapping[str, Any]],
    modes: FloatArray | None = None,
) -> base.FamilyFit:
    """Version 1's penalized REML and BLUE/BLUP, with the study and series levels
    correlated in ``(ell, e)`` and one level slope per slope group as configured."""
    if not config.uses_version_4_fit:
        return base.fit_family(config, family, summaries, modes)
    correlated = config.random_correlation == "level_excess"
    d = config.dimensions
    columns = _slope_columns(config, summaries)
    fixed_n = d + len(columns)
    levels = config.levels
    groups = len(config.grouping)
    variance_params = len(levels) * d
    penalty = np.asarray(config.penalty_modes if modes is None else modes, dtype=np.float64)
    blocks = _blocks(config, summaries, columns)

    def level_matrix(params: FloatArray, index: int) -> FloatArray:
        log_sd = params[index * d : index * d + d]
        if correlated and levels[index] in CORRELATED_LEVELS:
            return _correlated(log_sd, RHO_MAX * math.tanh(float(params[variance_params])))
        return base._diagonal(log_sd)

    def residual_covariance(params: FloatArray, block: Any) -> FloatArray:
        study = level_matrix(params, groups)
        group_mean = level_matrix(params, groups + 1)
        single = level_matrix(params, groups + 2)
        matrix = cast(FloatArray, np.kron(block.same_study, study))
        for row in range(len(block.members)):
            deviation = group_mean if block.aggregate[row] else single
            matrix[d * row : d * row + d, d * row : d * row + d] += (
                deviation + block.series_cov[row]
            )
        return matrix

    def marginal(params: FloatArray, block: Any) -> FloatArray:
        matrix = residual_covariance(params, block)
        for level in range(groups):
            matrix += cast(
                FloatArray, np.kron(block.same_group[level], level_matrix(params, level))
            )
        return matrix

    def objective(params: FloatArray) -> float:
        if np.any(np.abs(params) > 12.0):
            return 1e12
        a_total = np.zeros((fixed_n, fixed_n))
        b_total = np.zeros(fixed_n)
        pieces = []
        for block in blocks:
            try:
                chol = np.linalg.cholesky(marginal(params, block))
            except np.linalg.LinAlgError:
                return 1e12
            inv_design = np.linalg.solve(chol, block.fixed_design)
            inv_y = np.linalg.solve(chol, block.y)
            a_total += inv_design.T @ inv_design
            b_total += inv_design.T @ inv_y
            pieces.append((chol, inv_design, inv_y))
        sign, logdet_a = np.linalg.slogdet(a_total)
        if sign <= 0:
            return 1e12
        beta = np.linalg.solve(a_total, b_total)
        value = 0.5 * logdet_a
        for chol, inv_design, inv_y in pieces:
            residual = inv_y - inv_design @ beta
            value += float(np.sum(np.log(np.diag(chol)))) + 0.5 * float(residual @ residual)
        for level in range(len(levels)):
            value -= base._gamma2_log_penalty(np.exp(params[d * level : d * level + d]), penalty)
        if correlated:
            value += 0.5 * float(params[variance_params]) ** 2
        return value

    start = np.tile(np.log(penalty), len(levels))
    if correlated:
        start = np.concatenate([start, [0.0]])
    best, _ = nelder_mead(objective, start, iterations=base.REML_ITERATIONS)
    for _ in range(base.REML_RESTARTS):  # deterministic restarts from the incumbent
        best, _ = nelder_mead(objective, best, step=0.2, iterations=base.REML_ITERATIONS)
    value = objective(best)
    if value >= 1e12:
        raise InvalidRequest(f"Population fit of {family!r} did not converge")
    effect_index: dict[tuple[str, str], int] = {}
    offset = fixed_n
    for level in config.grouping:
        for key in sorted({base.group_key(level, m["context"]) for m in summaries}):
            effect_index[(level, key)] = offset
            offset += d
    precision = np.zeros((offset, offset))
    rhs = np.zeros(offset)
    for block in blocks:
        n = len(block.members)
        matrix = np.zeros((d * n, offset))
        matrix[:, :fixed_n] = block.fixed_design
        for row, index in enumerate(block.members):
            context = summaries[index]["context"]
            for level in config.grouping:
                begin = effect_index[(level, base.group_key(level, context))]
                matrix[d * row : d * row + d, begin : begin + d] = np.eye(d)
        inverse = np.linalg.inv(residual_covariance(best, block))
        precision += matrix.T @ inverse @ matrix
        rhs += matrix.T @ inverse @ block.y
    for (level, _), begin in effect_index.items():
        position = config.grouping.index(level)
        precision[begin : begin + d, begin : begin + d] += np.linalg.inv(
            level_matrix(best, position)
        )
    covariance = np.linalg.inv(precision)
    solution = covariance @ rhs
    return FamilyFit(
        family=family,
        grouping=config.grouping,
        fixed=solution[:fixed_n],
        variances={level: level_matrix(best, i) for i, level in enumerate(levels)},
        effect_index=effect_index,
        solution=solution,
        covariance=covariance,
        studies=tuple(sorted({m["study_id"] for m in summaries})),
        series=len(summaries),
        objective=float(value),
        slope_columns=columns,
    )


@dataclass(frozen=True)
class RecoveryFit(version2.RecoveryFit):
    """Version 2's fitted population with slope groups; ``describe`` adds the fitted
    correlations and slopes."""

    def location(self, context: str, covariate: float) -> tuple[FloatArray, FloatArray]:
        """Context-level ``theta`` at covariate ``x``, the covariate entering through
        the slope of the context's slope group (version 2's rule otherwise)."""
        fitted = self._family(context)
        config = self.config
        d = config.dimensions
        column = d
        if isinstance(fitted, FamilyFit) and isinstance(config, RecoveryConfig):
            column = fitted.slope_column(slope_group(config, context))
        row = np.zeros((d, fitted.solution.size))
        row[:, :d] = np.eye(d)
        row[0, column] = covariate
        starts = self._fallback_starts(fitted, context)
        if starts:
            for begin in starts:
                row[:, begin : begin + d] += np.eye(d) / len(starts)
            return row @ fitted.solution, (
                row @ fitted.covariance @ row.T + fitted.variances["context"]
            )
        known = self._known(fitted, context)
        for _, begin in known:
            row[:, begin : begin + d] = np.eye(d)
        mean = row @ fitted.solution
        covariance = row @ fitted.covariance @ row.T
        for level in fitted.grouping:
            if level not in {name for name, _ in known}:
                covariance = covariance + fitted.variances[level]
        return mean, covariance

    def describe(self) -> dict[str, Any]:
        out = super().describe()
        out["level_excess_correlation"] = {
            name: {
                level: level_excess_correlation(fitted.variances[level])
                for level in CORRELATED_LEVELS
            }
            for name, fitted in self.families.items()
        }
        out["level_slopes"] = {
            name: {
                group: {
                    "slope": float(fitted.solution[column]),
                    "sd": float(math.sqrt(fitted.covariance[column, column])),
                }
                for group, column in getattr(fitted, "slope_columns", {}).items()
            }
            for name, fitted in self.families.items()
        }
        return out


def fit_population(
    config: RecoveryConfig,
    series: Sequence[RecoverySeries],
    *,
    noise_series: Sequence[RecoverySeries] | None = None,
) -> version2.RecoveryFit:
    """Version 2's population fit with this version's hierarchy options; with both at
    their defaults it is version 2's fit itself."""
    if not isinstance(config, RecoveryConfig):
        raise InvalidRequest("Version-4 fitting needs a version-4 configuration")
    if not config.uses_version_4_fit:
        return version2.fit_population(config, series, noise_series=noise_series)
    if not 2 <= len(series) <= base.MAX_SERIES:
        raise InvalidRequest("Population fitting needs 2..5000 series")
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Population fitting requires unique series identities")
    if any(s.role != "control" for s in series):
        raise InvalidRequest("Population priors are fitted to control series only")
    summary_config = config.likelihood_config
    if not config.deviation_in_population_fit:
        summary_config = replace(summary_config, deviation_fraction=0.0)
    estimates = version2.study_noise(
        summary_config, noise_series if noise_series is not None else series
    )
    centers = version2.pooled_noise_centers(config, estimates)
    forecast_centers = None
    pooling = config.forecast_noise_pooling
    if pooling is not None and pooling != config.noise_pooling:
        forecast_centers = version2.pooled_noise_centers(
            replace(config, noise_pooling=pooling), estimates
        )
    summaries = []
    for item in series:
        if item.reference_value <= 0:
            continue
        family = base.family_of(item.context)
        own = estimates.get((item.study_id, family, item.kind))
        sigma = own.sd if own is not None else centers.center(family, item.kind, item.aggregate_n)
        summaries.append({**version2.series_summary(summary_config, item, sigma), "family": family})
    fitted: dict[str, base.FamilyFit] = {}
    unfitted: dict[str, str] = {}
    for family in sorted({m["family"] for m in summaries}):
        members = [m for m in summaries if m["family"] == family]
        studies = tuple(sorted({m["study_id"] for m in members}))
        if len(studies) < 2:
            unfitted[family] = "fewer than two training studies"
            continue
        if config.study_effects == "per_group":
            top = config.grouping[0]
            members = [
                {**m, "study_id": f"{m['study_id']}@{base.group_key(top, m['context'])}"}
                for m in members
            ]
        fitted[family] = replace(fit_family(config, family, members), studies=studies)
    if not fitted:
        raise InvalidRequest("No family has two training studies")
    return RecoveryFit(
        config=config,
        families=fitted,
        noise_centers=centers,
        study_noise_sd={f"{s}/{f}/{k}": v.sd for (s, f, k), v in estimates.items()},
        training_series=tuple(s.series_id for s in series),
        training_studies=tuple(sorted({s.study_id for s in series})),
        unfitted_families=unfitted,
        forecast_noise_centers=forecast_centers,
    )
