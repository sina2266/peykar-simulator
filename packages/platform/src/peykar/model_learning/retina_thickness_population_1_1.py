"""Central retinal thickness after treatment, model 1.1: configurations and dose rule.

Model 1.0.0 (``retina_thickness_population``, candidate ``r3``; unchanged because its
protocol pins it) is the starting point. Version 1.1 uses generic eye model version 2
(``eye_recovery_population2``) to address the two failures of the 1.0.0 sealed test:
the missed re-thickening after one dexamethasone implant, and contexts without
training series, which fell back to the species mean.

Declared dose rule (``effect_duration_months``): a series of a group that received
**one** dexamethasone implant (regimen ``single``; treatment ``steroid_implant``, or
``combination`` when the agent names a dexamethasone implant) has an effect duration
of ``DEXAMETHASONE_IMPLANT_MONTHS``. Fluocinolone implants (designed to release drug
for about three years) and every other group have none. The duration is the midpoint
of the logistic wear-off of the generic model; the constants come from the implant's
known course (largest effect near month 2, waning from month 3, oedema recurring
around month 4) and from the training records' implant series, never from tranche 2.

Each development round of ``experiments/eye/retina/model-1.1/development-log.md``
adds a named candidate and never changes an earlier one. Nothing here reads files or
claims clinical validity.
"""

from __future__ import annotations

from peykar.model_learning.eye_recovery_population2 import DAY_MONTHS, RecoveryConfig

MODEL_ID = "eye.retina-thickness-population"
DEXAMETHASONE_IMPLANT_MONTHS = 3.5
WEAR_OFF_WIDTH_MONTHS = 0.5
LONG_ACTING_IMPLANTS = ("fluocinolone", "iluvien", "yutiq", "retisert")


def effect_duration_months(treatment: str, regimen: str, agent: str | None) -> float | None:
    """Declared single-dose effect duration of a group (months), or ``None``."""
    if regimen != "single" or not agent:
        return None
    text = agent.lower()
    if any(name in text for name in LONG_ACTING_IMPLANTS):
        return None
    if treatment == "steroid_implant":
        # Protocol 1.0.0: a steroid implant is dexamethasone or fluocinolone.
        return DEXAMETHASONE_IMPLANT_MONTHS
    if treatment == "combination" and (
        "ozurdex" in text or ("dexamethasone" in text and "implant" in text)
    ):
        return DEXAMETHASONE_IMPLANT_MONTHS
    return None


def _config(
    version: str,
    *,
    wear_off_loss: float = 0.0,
    treatment_fallback: bool = False,
    noise_pooling: str = "median",
    forecast_noise_pooling: str | None = None,
) -> RecoveryConfig:
    # Model 1.0.0's adopted r3, plus the declared options.
    return RecoveryConfig(
        model_id=MODEL_ID,
        model_version=version,
        quantity="central retinal thickness",
        covariate_reference=300.0,
        covariate_unit="um",
        reference_months=DAY_MONTHS,
        transient_months=14.0 * DAY_MONTHS,
        grouping=("context",),
        deviation_fraction=0.8,
        deviation_months=12.0,
        deviation_in_population_fit=False,
        wear_off_width_months=WEAR_OFF_WIDTH_MONTHS,
        wear_off_loss=wear_off_loss,
        treatment_fallback=treatment_fallback,
        noise_pooling=noise_pooling,
        forecast_noise_pooling=forecast_noise_pooling,
    )


DEVELOPMENT_CANDIDATES: dict[str, RecoveryConfig] = {
    # s0: model 1.0.0 (r3) in generic version 2; must reproduce r3 exactly.
    "s0": _config("1.1.0-s0"),
    # s1: untrained contexts borrow the trained contexts of their treatment.
    "s1": _config("1.1.0-s1", treatment_fallback=True),
    # s2: one dexamethasone implant loses 80% of its effect as it wears off.
    "s2": _config("1.1.0-s2", wear_off_loss=0.8),
    # s3: s1 and s2 together.
    "s3": _config("1.1.0-s3", wear_off_loss=0.8, treatment_fallback=True),
    # s4 (round 2): s2 with the pooled noise centre instead of the median.
    "s4": _config("1.1.0-s4", wear_off_loss=0.8, noise_pooling="pooled"),
    # s5 (round 3): s2 with the pooled noise centre for forecast intervals only.
    "s5": _config("1.1.0-s5", wear_off_loss=0.8, forecast_noise_pooling="pooled"),
}
