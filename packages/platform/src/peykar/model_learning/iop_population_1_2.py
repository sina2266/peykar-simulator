"""IOP after glaucoma procedures, model 1.2: configurations of generic eye model 2.

Model 1.1.0 (``iop_population_1_1``, unchanged because its protocol pins it) is the
starting point: candidate ``c0`` is the same law, hierarchy and noise model written as
a configuration of ``eye_recovery_population2``. Version 1.2 addresses the main
weakness of the 1.1 sealed test: in the first month after surgery the forecast
intervals were too narrow (80% and 90% coverage 0.585 and 0.734) and the errors were
largest. Candidates add forecast-only persistent deviations and forecast-only early
noise; neither changes the population fit.

Each development round of ``experiments/eye/iop/model-1.2/development-log.md`` adds a
named candidate and never changes an earlier one. Nothing here reads files or claims
clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population2 import RecoveryConfig

MODEL_ID = "eye.iop-population"


def _config(
    version: str,
    *,
    deviation: tuple[float, float] = (0.0, 3.0),
    early_noise: tuple[float, float] | None = None,
) -> RecoveryConfig:
    # Model 1.1.0's constants are the generic defaults with a 20 mmHg reference.
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="intraocular pressure",
        covariate_reference=20.0,
        covariate_unit="mmHg",
        deviation_fraction=deviation[0],
        deviation_months=deviation[1],
        deviation_in_population_fit=False,
        early_noise=early_noise,
        early_noise_in_likelihood=False,
        # As in model 1.1.0: each procedure arm of a multi-arm study has its own study
        # effect (generic model 1 would share one across procedures).
        study_effects="per_group",
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # c0: model 1.1.0 in generic version 2; must reproduce the 1.1 harness exactly.
    "c0": _config("1.2.0-c0"),
    # c2: c0 with forecast-only persistent deviations (half the residual variance,
    # 3-month correlation time).
    "c2": _config("1.2.0-c2", deviation=(0.5, 3.0)),
    # c1 (round 2): c0 with forecast-only early noise, k = 8 and T = 1 month, chosen by
    # the declared calibration procedure on c0's development rows.
    "c1": _config("1.2.0-c1", early_noise=(8.0, 1.0)),
}
