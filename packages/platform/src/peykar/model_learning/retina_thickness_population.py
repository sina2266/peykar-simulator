"""Central retinal thickness after treatment: configurations of the generic model.

The model is ``eye_recovery_population`` (the generic form of IOP model 1.1) applied
to central retinal thickness by OCT as a fraction of the pretreatment value, with
contexts ``species/indication/treatment``. Only declared constants live here; each
development round of the leave-one-study-out log adds a named candidate and never
changes an earlier one.

Round 0 (in-sample, development records only, ``experiments/eye/retina/model-1.0``):
the dof-adjusted residual SD of best fits over the ten control group means with at
least five follow-ups was 0.1139 without a transient term and 0.1038, 0.1031 and
0.1036 with a fixed transient of 7, 14 and 30 days, so the 14-day transient is kept.
The level of the last follow-up fell with the baseline thickness (slope of
``ln f`` on ``ln(CST0 / 300 um)`` about -0.6), so the level covariate is kept with a
300 um reference. Nothing here reads files or claims clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population import DAY_MONTHS, RecoveryConfig

MODEL_ID = "eye.retina-thickness-population"


def _config(
    version: str,
    *,
    grouping: tuple[str, ...],
    transient_days: float = 14.0,
    deviation: tuple[float, float] = (0.0, 3.0),
    deviation_in_fit: bool = True,
) -> RecoveryConfig:
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="central retinal thickness",
        covariate_reference=300.0,
        covariate_unit="um",
        reference_months=DAY_MONTHS,
        transient_months=transient_days * DAY_MONTHS,
        grouping=grouping,
        deviation_fraction=deviation[0],
        deviation_months=deviation[1],
        deviation_in_population_fit=deviation_in_fit,
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # r0: the IOP 1.1 structure (one context level) with the round-0 transient.
    "r0": _config("1.0.0-r0", grouping=("context",)),
    # r1: contexts nested in their treatment, so a new indication of a known treatment
    # borrows that treatment's kinetics (not adopted: initial state lost 0.07 pp).
    "r1": _config("1.0.0-r1", grouping=("treatment", "context")),
    # r2a-d: r0 with persistent (Ornstein-Uhlenbeck) deviations from the law.
    "r2a": _config("1.0.0-r2a", grouping=("context",), deviation=(0.5, 3.0)),
    "r2b": _config("1.0.0-r2b", grouping=("context",), deviation=(0.5, 12.0)),
    "r2c": _config("1.0.0-r2c", grouping=("context",), deviation=(0.8, 3.0)),
    "r2d": _config("1.0.0-r2d", grouping=("context",), deviation=(0.8, 12.0)),
    # r3: r2d's deviations in forecasting only; population fitted as in r0.
    "r3": _config("1.0.0-r3", grouping=("context",), deviation=(0.8, 12.0), deviation_in_fit=False),
}
