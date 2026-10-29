"""Forecast eye measurements with the frozen, sealed-tested eye component models."""

import json
from pathlib import Path
from typing import Annotated, Any

import typer

from peykar import eye_simulation
from peykar_core.errors import InvalidRequest

eye_app = typer.Typer(
    no_args_is_help=True,
    help="Eye component forecasts (frozen models with one-shot sealed tests; not clinical)",
)
MAX_FIT_BYTES = 50 * 1024 * 1024


def _emit(value: Any) -> None:
    typer.echo(json.dumps(value, indent=2, ensure_ascii=False, allow_nan=False))


def _months(text: str) -> list[float]:
    try:
        return [float(part) for part in text.split(",") if part.strip()]
    except ValueError as exc:
        raise InvalidRequest("Months must be a comma-separated list of numbers") from exc


def _observed(text: str | None) -> list[tuple[float, float]]:
    if not text:
        return []
    pairs = []
    for part in text.split(","):
        month, sep, value = part.partition(":")
        if not sep:
            raise InvalidRequest("Observed follow-ups must look like month:value,month:value")
        try:
            pairs.append((float(month), float(value)))
        except ValueError as exc:
            raise InvalidRequest("Observed months and values must be numbers") from exc
    return pairs


@eye_app.command("components")
def components() -> None:
    """List the components, their frozen versions and their sealed-test results."""
    _emit(
        [item.describe() for item in eye_simulation.COMPONENTS.values()]
        + [item.describe() for item in eye_simulation.CORNEAL_COMPONENTS.values()]
        + [item.describe() for item in eye_simulation.ADDITIVE_COMPONENTS.values()]
    )


@eye_app.command("fit")
def fit(
    component: Annotated[str, typer.Option(help="component id, see `peykar eye components`")],
    records: Annotated[list[Path], typer.Option(exists=True, file_okay=False)],
    output: Annotated[Path, typer.Option(dir_okay=False)],
) -> None:
    """Fit a component's population to the record folders and store it as JSON
    (retina-thickness and iop; corneal and refraction components are fitted at
    forecast time)."""
    item = eye_simulation.component(component)
    if output.exists():
        raise InvalidRequest("--output must not exist")
    series, hashes = eye_simulation.load_series(item, eye_simulation.record_paths(records))
    fitted = eye_simulation.fit_component(item, series)
    with output.open("x", encoding="utf-8") as handle:
        handle.write(eye_simulation.save_fit(item, fitted, hashes) + "\n")
    _emit(
        {
            "component": item.component_id,
            "model_version": item.model_version,
            "records": len(hashes),
            "training_studies": len(fitted.training_studies),
            "training_series": len(fitted.training_series),
            "output": str(output),
        }
    )


@eye_app.command("forecast")
def forecast(
    component: Annotated[str, typer.Option()],
    context: Annotated[str, typer.Option(help="for example human/dme/anti_vegf")],
    baseline: Annotated[
        float, typer.Option(help="pretreatment value in the component's unit (SE0 for refraction)")
    ],
    months: Annotated[str, typer.Option(help="comma-separated forecast months")],
    fit_file: Annotated[
        Path | None, typer.Option("--fit", exists=True, dir_okay=False, help="stored fit")
    ] = None,
    records: Annotated[
        list[Path] | None,
        typer.Option(exists=True, file_okay=False, help="record folders to fit now"),
    ] = None,
    group_size: Annotated[int | None, typer.Option("--n", help="eyes in the group")] = None,
    observed: Annotated[str | None, typer.Option(help="month:value,... after baseline")] = None,
    regimen: Annotated[str | None, typer.Option(help="for a component's dose rule")] = None,
    agent: Annotated[str | None, typer.Option(help="drug or device, for the dose rule")] = None,
    reference_kind: Annotated[
        str, typer.Option(help="corneal endothelium: patient_preoperative or a donor count")
    ] = "patient_preoperative",
) -> None:
    """Forecast one group (or eye) from its context, baseline and observed follow-ups."""
    item = eye_simulation.any_component(component)
    if (fit_file is None) == (not records):
        raise InvalidRequest("Give exactly one of --fit and --records")
    pairs = _observed(observed)
    targets = _months(months)
    if isinstance(item, eye_simulation.CornealComponent):
        if records is None or regimen is not None or agent is not None:
            raise InvalidRequest("Corneal components need --records and take no dose rule")
        series, hashes = eye_simulation.load_series_any(item, eye_simulation.record_paths(records))
        result = eye_simulation.forecast_corneal(
            item,
            eye_simulation.fit_corneal(item, series),
            context=context,
            baseline=baseline,
            months=targets,
            group_size=group_size,
            observed=pairs,
            reference_kind=reference_kind,
        )
        _emit({**result, "fit_records": len(hashes)})
        return
    if isinstance(item, eye_simulation.AdditiveComponent):
        if records is None or regimen is not None or agent is not None:
            raise InvalidRequest("Additive components need --records and take no dose rule")
        series, hashes = eye_simulation.load_series_any(item, eye_simulation.record_paths(records))
        result = eye_simulation.forecast_additive(
            item,
            eye_simulation.fit_component(item, series),
            context=context,
            baseline=baseline,
            months=targets,
            group_size=group_size,
            observed=pairs,
        )
        _emit({**result, "fit_records": len(hashes)})
        return
    if fit_file is not None:
        if fit_file.stat().st_size > MAX_FIT_BYTES:
            raise InvalidRequest("The stored fit is too large")
        fitted, provenance = eye_simulation.load_fit(item, fit_file.read_text())
        count = len(provenance.get("record_sha256", {}))
    else:
        series, hashes = eye_simulation.load_series(
            item, eye_simulation.record_paths(records or [])
        )
        fitted, count = eye_simulation.fit_component(item, series), len(hashes)
    result = eye_simulation.forecast(
        item,
        fitted,
        context=context,
        baseline=baseline,
        months=targets,
        group_size=group_size,
        observed=pairs,
        regimen=regimen,
        agent=agent,
    )
    _emit({**result, "fit_records": count})
