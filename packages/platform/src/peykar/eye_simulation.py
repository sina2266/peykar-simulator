"""Eye components that run on generic eye model 2: registry, record loading and fits.

Each component names its frozen model version and configuration, the record schema
converter, the protocol of its one-shot sealed test and a short statement of that
test's result, so that every forecast reports what was and was not tested. Fitting
uses the component's training records as given (the operator chooses the record
folders; privately kept records never leave the machine). Nothing here registers a
model with the platform, downloads data or claims clinical validity.
"""

from __future__ import annotations

import hashlib
import json
import math
from collections.abc import Callable, Mapping, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from peykar.model_learning import (
    acuity_population,
    endothelial_population,
    iop_population_1_2,
    refraction_population,
    retina_thickness_population_1_1,
    thickness_population,
)
from peykar.model_learning import eye_recovery_population2 as population
from peykar.model_learning.eye_forecast import (
    QUANTILES,
    check_support,
    fit_from_json,
    fit_to_json,
    forecast_scenario,
    time_support,
)
from peykar.validation.cornea_acuity_data import (
    COVARIATE_UNIT_LOGMAR,
    MODEL_UNIT_LOGMAR,
    AcuitySeries,
    to_acuity_series,
)
from peykar.validation.cornea_endothelium_data import DensitySeries, to_density_series
from peykar.validation.cornea_refraction_data import (
    COVARIATE_UNIT_D,
    MODEL_UNIT_D,
    RefractionSeries,
    to_refraction_series_by_error,
)
from peykar.validation.cornea_thickness_data import ThicknessSeries, to_thickness_series
from peykar.validation.eye_iop_data_1_2 import to_iop_series
from peykar.validation.eye_retina_data_1_1 import to_retina_series_1_1
from peykar_core.errors import InvalidRequest

__all__ = [
    "ADDITIVE_COMPONENTS",
    "COMPONENTS",
    "CORNEAL_COMPONENTS",
    "AdditiveComponent",
    "Component",
    "CornealComponent",
    "fit_component",
    "forecast",
    "load_fit",
    "load_series",
    "load_series_any",
    "training_support",
]

MAX_RECORDS = 2_000
MAX_RECORD_BYTES = 5 * 1024 * 1024


@dataclass(frozen=True)
class Component:
    component_id: str
    model_version: str
    candidate: str
    quantity: str
    unit: str
    context_form: str
    config: population.RecoveryConfig
    to_series: Callable[[Mapping[str, Any]], Sequence[Any]]
    protocol: str
    sealed_result: str
    context_of_use: str
    dose_rule: Callable[[str, str, str | None], float | None] | None = None

    def describe(self) -> dict[str, Any]:
        return {
            "component": self.component_id,
            "model_version": self.model_version,
            "candidate": self.candidate,
            "quantity": self.quantity,
            "unit": self.unit,
            "context_form": self.context_form,
            "protocol": self.protocol,
            "sealed_result": self.sealed_result,
            "context_of_use": self.context_of_use,
            "configuration": self.config.describe(),
        }


COMPONENTS: dict[str, Component] = {
    "retina-thickness": Component(
        component_id="retina-thickness",
        model_version="1.1.0",
        candidate="s5",
        quantity="central retinal thickness by OCT",
        unit="um",
        context_form="human/<indication>/<treatment>, for example human/dme/anti_vegf",
        config=retina_thickness_population_1_1.DEVELOPMENT_CANDIDATES["s5"],
        to_series=to_retina_series_1_1,
        protocol="experiments/eye/retina/model-1.1/protocol-1.1.0.json",
        sealed_result=(
            "tranche-2 sealed test, 92 group means: H1-H4 pass (coverage 0.844/0.896; next "
            "observation 7.91 vs persistence 8.81 pp; landmark 9.90 vs 11.55; initial state "
            "13.00 vs 18.51); H5 fails (model 1.0.0 with the same training 7.79/9.73/13.25)"
        ),
        context_of_use=(
            "human group means after treatment of macular edema, neovascular AMD and related "
            "conditions; single eyes and treatment effects are not claimed"
        ),
        dose_rule=retina_thickness_population_1_1.effect_duration_months,
    ),
    "iop": Component(
        component_id="iop",
        model_version="1.2.0",
        candidate="c1",
        quantity="intraocular pressure",
        unit="mmHg",
        context_form="human/<procedure>, for example human/trabeculectomy",
        config=iop_population_1_2.DEVELOPMENT_CANDIDATES["c1"],
        to_series=to_iop_series,
        protocol="experiments/eye/iop/model-1.2/protocol-1.2.0.json",
        sealed_result=(
            "tranche-3 sealed test, 130 group means: all five hypotheses pass (coverage "
            "0.835/0.931; next observation 5.89 vs persistence 7.21 pp; landmark 7.76 vs "
            "11.89; initial state 10.17 vs 18.06; first-month coverage 0.824/0.939 vs 1.1.0 "
            "0.679/0.817 at the same accuracy)"
        ),
        context_of_use=(
            "human group means after glaucoma procedures; single eyes, non-human species "
            "and treatment effects are not claimed"
        ),
    ),
}


def component(name: str) -> Component:
    if name not in COMPONENTS:
        raise InvalidRequest(f"Unknown eye component {name!r}; known: {sorted(COMPONENTS)}")
    return COMPONENTS[name]


def record_paths(folders: Sequence[Path]) -> list[Path]:
    paths = sorted((p for folder in folders for p in Path(folder).glob("*.json")), key=str)
    if not paths:
        raise InvalidRequest("No record files were found in the given folders")
    if len(paths) > MAX_RECORDS:
        raise InvalidRequest(f"At most {MAX_RECORDS} records can be loaded")
    return paths


def load_series(item: Component, paths: Sequence[Path]) -> tuple[list[Any], dict[str, str]]:
    """Series of every record (validated by the component's converter) and record hashes."""
    return load_series_any(item, paths)


def load_series_any(
    item: Component | CornealComponent | AdditiveComponent, paths: Sequence[Path]
) -> tuple[list[Any], dict[str, str]]:
    """As :func:`load_series`, for any component."""
    series: list[Any] = []
    hashes: dict[str, str] = {}
    for path in paths:
        raw = Path(path).read_bytes()
        if len(raw) > MAX_RECORD_BYTES:
            raise InvalidRequest(f"{Path(path).name} exceeds {MAX_RECORD_BYTES} bytes")
        series.extend(item.to_series(json.loads(raw)))
        hashes[Path(path).name] = hashlib.sha256(raw).hexdigest()
    if len({s.series_id for s in series}) != len(series):
        raise InvalidRequest("Series identities repeat across the given records")
    return series, hashes


def population_series(
    item: Component | CornealComponent | AdditiveComponent, series: Sequence[Any]
) -> list[Any]:
    """The series that fit a component's population, as the harnesses select them:
    controls (with a baseline value for the generic-model components)."""
    if isinstance(item, CornealComponent):
        return [s for s in series if s.role == "control"]
    return [s for s in series if s.role == "control" and s.reference_value > 0]


def training_support(
    item: Component | CornealComponent | AdditiveComponent, series: Sequence[Any]
) -> dict[str, Any]:
    """Visit months of the series that fit the population (``time_support``)."""
    return time_support(population_series(item, series))


def fit_component(
    item: Component | AdditiveComponent, series: Sequence[Any]
) -> population.RecoveryFit:
    """The population fit the harness makes: controls with a baseline value fit the
    population; every series gives noise estimates."""
    controls = population_series(item, series)
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        raise InvalidRequest("Fitting needs control series from at least two studies")
    return population.fit_population(item.config, controls, noise_series=series)


def save_fit(
    item: Component,
    fit: population.RecoveryFit,
    hashes: Mapping[str, str],
    support: Mapping[str, Any] | None = None,
) -> str:
    """A stored fit with its record hashes and, if given, its training visit months."""
    provenance: dict[str, Any] = {
        "component": item.component_id,
        "model_version": item.model_version,
        "record_sha256": dict(sorted(hashes.items())),
    }
    if support is not None:
        provenance["training_support"] = dict(support)
    return json.dumps(fit_to_json(fit, **provenance), indent=1)


def load_fit(item: Component, text: str) -> tuple[population.RecoveryFit, dict[str, Any]]:
    data = json.loads(text)
    provenance = data.get("provenance", {})
    if provenance.get("component") != item.component_id:
        raise InvalidRequest("The stored fit belongs to another component")
    return fit_from_json(item.config, data), provenance


def forecast(
    item: Component,
    fit: population.RecoveryFit,
    *,
    context: str,
    baseline: float,
    months: Sequence[float],
    group_size: int | None = None,
    observed: Sequence[tuple[float, float]] = (),
    regimen: str | None = None,
    agent: str | None = None,
    support: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """Primary forecast of one scenario, with the component's declared dose rule;
    ``support`` (:func:`training_support`) adds warnings outside the training visits."""
    duration = None
    if item.dose_rule is not None and regimen is not None:
        treatment = context.rsplit("/", 1)[-1]
        duration = item.dose_rule(treatment, regimen, agent)
    result = forecast_scenario(
        item.config,
        fit,
        context=context,
        baseline=baseline,
        months=months,
        group_size=group_size,
        observed=observed,
        effect_duration_months=duration,
        support=support,
    )
    return {
        "component": item.component_id,
        "model_version": item.model_version,
        "unit": item.unit,
        "sealed_result": item.sealed_result,
        "context_of_use": item.context_of_use,
        **result,
    }


# --- corneal components (their own frozen model APIs; fitted on the fly) ---------------

DENSITY_REFERENCES = ("patient_preoperative", "donor_eye_bank", "donor_other")
STUDENT_DF = 4.0
DRAWS = 2000


@dataclass(frozen=True)
class CornealComponent:
    """A corneal component with its frozen model version and tested primary methods."""

    component_id: str
    model_version: str
    quantity: str
    unit: str
    context_form: str
    protocol: str
    sealed_result: str
    context_of_use: str
    primary_initial: str
    primary_forecast: str
    to_series: Callable[[Mapping[str, Any]], Sequence[Any]]

    def describe(self) -> dict[str, Any]:
        return {
            "component": self.component_id,
            "model_version": self.model_version,
            "quantity": self.quantity,
            "unit": self.unit,
            "context_form": self.context_form,
            "protocol": self.protocol,
            "sealed_result": self.sealed_result,
            "context_of_use": self.context_of_use,
            "primary_initial_state": self.primary_initial,
            "primary_forecast": self.primary_forecast,
            "stored_fits": False,
        }


CORNEAL_COMPONENTS: dict[str, CornealComponent] = {
    "corneal-endothelium": CornealComponent(
        component_id="corneal-endothelium",
        model_version=endothelial_population.MODEL_VERSION,
        quantity="corneal endothelial cell density",
        unit="cells/mm2",
        context_form="<species>/<procedure>, for example human/dmek or human/cataract",
        protocol="experiments/vcornea/endothelium/protocol-1.0.0.json",
        sealed_result=(
            "sealed test, 30 group means: H1-H4 pass (coverage 0.711/0.829; next observation "
            "5.60 vs persistence 7.36 pp; landmark 6.85 vs 12.41; initial state 7.05 vs 8.16)"
        ),
        context_of_use=(
            "group means after keratoplasty, cataract, glaucoma, phakic-lens and vitreoretinal "
            "surgery; the reference kind (patient preoperative or donor count) must be given"
        ),
        primary_initial="population_initial_state (Gaussian prior, predictive mean)",
        primary_forecast="population_anchored_t4 (predictive mean)",
        to_series=to_density_series,
    ),
    "corneal-thickness": CornealComponent(
        component_id="corneal-thickness",
        model_version=thickness_population.MODEL_VERSION,
        quantity="central corneal thickness",
        unit="um",
        context_form="<species>/<procedure>, for example human/cataract or human/dmek",
        protocol="experiments/vcornea/thickness/protocol-1.0.0.json",
        sealed_result=(
            "sealed test, 29 forecast series: H1-H4 pass (coverage 0.892/0.969; next "
            "observation 1.60 vs persistence 2.06 pp; landmark 2.31 vs 4.01; initial state "
            "5.17 vs 6.82)"
        ),
        context_of_use=(
            "group means after cataract, keratoplasty, glaucoma, phakic-lens and vitreoretinal "
            "surgery; late graft thickening and YAG capsulotomy are not represented"
        ),
        primary_initial="population_initial_state (Gaussian prior, predictive mean)",
        primary_forecast="population_assimilation_robust (predictive mean)",
        to_series=to_thickness_series,
    ),
}


def any_component(name: str) -> Component | CornealComponent | AdditiveComponent:
    if name in COMPONENTS:
        return COMPONENTS[name]
    if name in CORNEAL_COMPONENTS:
        return CORNEAL_COMPONENTS[name]
    if name in ADDITIVE_COMPONENTS:
        return ADDITIVE_COMPONENTS[name]
    known = sorted({*COMPONENTS, *CORNEAL_COMPONENTS, *ADDITIVE_COMPONENTS})
    raise InvalidRequest(f"Unknown eye component {name!r}; known: {known}")


def fit_corneal(item: CornealComponent, series: Sequence[Any]) -> Any:
    """The harness's fit: controls fit the population, every series gives noise."""
    controls = [s for s in series if s.role == "control"]
    if len(controls) < 2 or len({s.study_id for s in controls}) < 2:
        raise InvalidRequest("Fitting needs control series from at least two studies")
    if item.component_id == "corneal-endothelium":
        return endothelial_population.fit_population(controls, noise_series=series)
    return thickness_population.fit_population(controls, noise_series=series)


def _corneal_series(
    item: CornealComponent,
    context: str,
    baseline: float,
    pairs: Sequence[tuple[float, float]],
    n: int | None,
    reference_kind: str,
) -> Any:
    months = tuple(m for m, _ in pairs) or (1.0,)
    fractions = tuple(v / baseline for _, v in pairs) or (1.0,)
    if item.component_id == "corneal-endothelium":
        return DensitySeries(
            series_id="scenario/series",
            study_id="scenario",
            context=context,
            reference_ecd=baseline,
            months=months,
            fractions=fractions,
            aggregate_n=n,
            reference_kind=reference_kind,
        )
    return ThicknessSeries(
        series_id="scenario/series",
        study_id="scenario",
        context=context,
        reference_um=baseline,
        months=months,
        fractions=fractions,
        aggregate_n=n,
    )


def forecast_corneal(
    item: CornealComponent,
    fit: Any,
    *,
    context: str,
    baseline: float,
    months: Sequence[float],
    group_size: int | None = None,
    observed: Sequence[tuple[float, float]] = (),
    reference_kind: str = "patient_preoperative",
    support: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """The tested primary forecast of one corneal scenario (predictive means)."""
    if not (baseline > 0 and baseline < float("inf")):
        raise InvalidRequest("The baseline value must be finite and positive")
    targets = [float(m) for m in months]
    if not targets or any(not (0 < m < float("inf")) for m in targets):
        raise InvalidRequest("Every forecast month must be finite and positive")
    if reference_kind not in DENSITY_REFERENCES:
        raise InvalidRequest(f"The reference kind must be one of {DENSITY_REFERENCES}")
    pairs = sorted((float(m), float(v)) for m, v in observed)
    if len({m for m, _ in pairs}) != len(pairs):
        raise InvalidRequest("Observed months must be unique")
    n = None if group_size is None else int(group_size)
    series = _corneal_series(item, context, baseline, pairs, n, reference_kind)
    if item.component_id == "corneal-endothelium":
        model = endothelial_population
        key = model.model_context(series)
        prior = fit.prior(key, series.kind)
        noise = fit.noise(series.kind, key)
        if pairs:
            fitted = model.anchored_posterior(
                prior, series=series, noise=noise, student_df=STUDENT_DF
            )
        else:
            fitted = model.posterior(prior, series=None, noise=noise)
        result = model.predictive(fitted, months=targets, draws=DRAWS, seed=0, quantiles=QUANTILES)
    else:
        tmodel = thickness_population
        key = context
        prior = fit.prior(key, series.kind)
        noise = fit.noise(series.kind, key)
        offset = tmodel.level_offset(series)
        gaussian = tmodel.gaussian(prior["mean"], prior["covariance"])
        variant = tmodel.robust(gaussian) if pairs else gaussian
        thickness_fit = tmodel.posterior(
            variant, series=series if pairs else None, noise=noise, offset=offset
        )
        result = tmodel.predictive(thickness_fit, months=targets, quantiles=QUANTILES)
    level_used = fit.level_used(key)
    quantiles = result["quantiles"]
    warnings = []
    if level_used != key:
        warnings.append(
            f"context {key!r} has no training series; the prior comes from {level_used}"
        )
    if n is None:
        warnings.append("single-eye forecasts were reported in the sealed tests, not tested")
    if pairs and min(targets) <= pairs[-1][0]:
        warnings.append("some forecast months are not after the last observed follow-up")
    view, outside = check_support(
        support, context=key, targets=targets, observed_months=[m for m, _ in pairs]
    )
    warnings.extend(outside)
    return {
        "component": item.component_id,
        "model_version": item.model_version,
        "unit": item.unit,
        "sealed_result": item.sealed_result,
        "context_of_use": item.context_of_use,
        "context": key,
        "level_used": level_used,
        "method": item.primary_forecast if pairs else item.primary_initial,
        "point": "predictive mean",
        "baseline": baseline,
        "training_support": view,
        "group_size": n,
        "observed": [{"month": m, "value": v, "fraction": v / baseline} for m, v in pairs],
        "forecast": [
            {
                "month": month,
                "fraction": {
                    "point": result["mean_fraction"][i],
                    "interval_80": [quantiles["0.1"][i], quantiles["0.9"][i]],
                    "interval_90": [quantiles["0.05"][i], quantiles["0.95"][i]],
                },
                "value": {
                    "point": baseline * result["mean_fraction"][i],
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


# --- additive components (outcomes that change in their own unit) ----------------------


@dataclass(frozen=True)
class AdditiveComponent:
    """A component whose outcome changes additively (refraction in dioptres, visual
    acuity in logMAR).

    The generic model sees the change from baseline as the pseudo-fraction
    ``exp(change / model_unit)`` and the baseline as ``exp(baseline / covariate_unit)``
    (``cornea_refraction_data``, ``cornea_acuity_data``); forecasts are turned back into
    the outcome's unit.
    Fitted at forecast time from record folders; the tested primaries are the
    Gaussian initial state and the robust assimilation, with the predictive median.
    """

    component_id: str
    model_version: str
    candidate: str
    quantity: str
    unit: str
    context_form: str
    config: population.RecoveryConfig
    to_series: Callable[[Mapping[str, Any]], Sequence[Any]]
    protocol: str
    sealed_result: str
    context_of_use: str
    model_unit: float
    covariate_unit: float
    # (context, baseline, months, changes, group size) -> the component's series type
    scenario_series: Callable[[str, float, tuple[float, ...], tuple[float, ...], int | None], Any]
    single_eye_note: str = "single-eye forecasts were not tested (no single-eye series)"

    def describe(self) -> dict[str, Any]:
        return {
            "component": self.component_id,
            "model_version": self.model_version,
            "candidate": self.candidate,
            "quantity": self.quantity,
            "unit": self.unit,
            "context_form": self.context_form,
            "protocol": self.protocol,
            "sealed_result": self.sealed_result,
            "context_of_use": self.context_of_use,
            "primary_initial_state": "population_initial_state (Gaussian prior, median)",
            "primary_forecast": "population_assimilation_robust (median)",
            "stored_fits": False,
            "configuration": self.config.describe(),
        }


def _refraction_scenario(
    context: str,
    baseline: float,
    months: tuple[float, ...],
    changes: tuple[float, ...],
    n: int | None,
) -> RefractionSeries:
    return RefractionSeries(
        series_id="scenario/series",
        study_id="scenario",
        context=context,
        reference_d=baseline,
        months=months,
        changes_d=changes,
        aggregate_n=n,
    )


def _acuity_scenario(
    context: str,
    baseline: float,
    months: tuple[float, ...],
    changes: tuple[float, ...],
    n: int | None,
) -> AcuitySeries:
    return AcuitySeries(
        series_id="scenario/series",
        study_id="scenario",
        context=context,
        reference_logmar=baseline,
        months=months,
        changes_logmar=changes,
        aggregate_n=n,
    )


ADDITIVE_COMPONENTS: dict[str, AdditiveComponent] = {
    "refraction": AdditiveComponent(
        component_id="refraction",
        model_version="1.0.0",
        candidate="f1",
        quantity="spherical equivalent after corneal laser refractive surgery",
        unit="D",
        context_form=(
            "human/<refractive error>/<procedure>, for example human/myopia/lasik, "
            "human/myopia/lenticule_extraction or human/hyperopia/surface_ablation"
        ),
        config=refraction_population.DEVELOPMENT_CANDIDATES["f1"],
        to_series=to_refraction_series_by_error,
        protocol="experiments/eye/refraction/model-1.0/protocol-1.0.0.json",
        sealed_result=(
            "sealed test, 13 group-mean forecast series: all five hypotheses pass (coverage "
            "0.821/0.857; next observation 0.29 vs persistence 0.37 D; landmark 0.47 vs "
            "0.78 D; initial state 0.40 vs full correction 0.50 D)"
        ),
        context_of_use=(
            "human group means with a plano target after surface ablation, LASIK or "
            "lenticule extraction; myopic corrections tested, hyperopic ones only trained"
        ),
        model_unit=MODEL_UNIT_D,
        covariate_unit=COVARIATE_UNIT_D,
        scenario_series=_refraction_scenario,
    ),
    "acuity": AdditiveComponent(
        component_id="acuity",
        model_version="1.0.0",
        candidate="a0",
        quantity="corrected distance visual acuity after keratoplasty or cross-linking",
        unit="logMAR",
        context_form=(
            "human/<procedure> with procedure pk, dalk, dsaek, dmek, cxl or other, for "
            "example human/dmek or human/cxl"
        ),
        config=acuity_population.DEVELOPMENT_CANDIDATES["a0"],
        to_series=to_acuity_series,
        protocol="experiments/eye/acuity/model-1.0/protocol-1.0.0.json",
        sealed_result=(
            "sealed test, 21 group-mean forecast series: H1-H4 pass (coverage 0.829/0.927; "
            "next observation 5.00 vs persistence 5.06 letters; landmark 5.24 vs 7.27 letters; "
            "initial state 9.04 vs pooled curve 9.79 letters); on development it lost H2/H3 to "
            "the pooled-curve increment"
        ),
        context_of_use=(
            "human group means after cross-linking and endothelial keratoplasty (tested mostly "
            "on cross-linking and DMEK); keratoplasty groups that recover poorly for their "
            "preoperative acuity are forecast too optimistically"
        ),
        model_unit=MODEL_UNIT_LOGMAR,
        covariate_unit=COVARIATE_UNIT_LOGMAR,
        scenario_series=_acuity_scenario,
        single_eye_note="single-eye forecasts were scored but not tested",
    ),
}


def forecast_additive(
    item: AdditiveComponent,
    fit: population.RecoveryFit,
    *,
    context: str,
    baseline: float,
    months: Sequence[float],
    group_size: int | None = None,
    observed: Sequence[tuple[float, float]] = (),
    support: Mapping[str, Any] | None = None,
) -> dict[str, Any]:
    """The tested primary forecast of one additive scenario, in the outcome's unit."""
    if not math.isfinite(baseline):
        raise InvalidRequest("The baseline value must be finite")
    targets = [float(m) for m in months]
    if not targets or any(not (0 < m < float("inf")) for m in targets):
        raise InvalidRequest("Every forecast month must be finite and positive")
    pairs = sorted((float(m), float(v)) for m, v in observed)
    if any(not (0 < m < float("inf") and math.isfinite(v)) for m, v in pairs):
        raise InvalidRequest("Observed months must be positive and values finite")
    if len({m for m, _ in pairs}) != len(pairs):
        raise InvalidRequest("Observed months must be unique")
    if group_size is not None and (isinstance(group_size, bool) or int(group_size) < 1):
        raise InvalidRequest("The group size must be a positive integer")
    n = None if group_size is None else int(group_size)
    series = item.scenario_series(
        context,
        baseline,
        tuple(m for m, _ in pairs) or (1.0,),
        tuple(v - baseline for _, v in pairs) or (0.0,),
        n,
    )
    config = item.config
    prior = fit.prior(context, series.kind, population.level_covariate(config, series))
    level_used = fit.level_used(context)
    gaussian = population.gaussian(prior["mean"], prior["covariance"])
    noise = fit.noise(series.kind, context, n)
    if pairs:
        fitted = population.posterior(
            config, population.robust(config, gaussian), series=series, noise=noise
        )
        method = "population_assimilation_robust"
    else:
        fitted = population.posterior(config, gaussian, series=None, noise=noise)
        method = "population_initial_state"
    result = population.predictive(
        config,
        fitted,
        months=targets,
        quantiles=QUANTILES,
        extra_noise=fit.forecast_inflation(series.kind, context, n),
    )
    quantiles = result["quantiles"]

    def value(fraction: float) -> float:
        return baseline + item.model_unit * math.log(fraction)

    warnings = []
    if level_used != context:
        warnings.append(
            f"context {context!r} has no training series; the prior comes from {level_used}"
        )
    if n is None:
        warnings.append(item.single_eye_note)
    if pairs and min(targets) <= pairs[-1][0]:
        warnings.append("some forecast months are not after the last observed follow-up")
    view, outside = check_support(
        support, context=context, targets=targets, observed_months=[m for m, _ in pairs]
    )
    warnings.extend(outside)
    return {
        "component": item.component_id,
        "model_version": item.model_version,
        "unit": item.unit,
        "sealed_result": item.sealed_result,
        "context_of_use": item.context_of_use,
        "context": context,
        "level_used": level_used,
        "method": method,
        "point": "predictive median",
        "baseline": baseline,
        "training_support": view,
        "group_size": n,
        "observed": [{"month": m, "value": v, "change": v - baseline} for m, v in pairs],
        "forecast": [
            {
                "month": month,
                "value": {
                    "point": value(result["point_fraction"][i]),
                    "interval_80": [value(quantiles["0.1"][i]), value(quantiles["0.9"][i])],
                    "interval_90": [value(quantiles["0.05"][i]), value(quantiles["0.95"][i])],
                },
            }
            for i, month in enumerate(targets)
        ],
        "warnings": warnings,
    }
