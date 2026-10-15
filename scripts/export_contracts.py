"""Export deterministic scientific schemas, OpenAPI and synthetic examples."""

import argparse
import inspect
import json
from pathlib import Path

from peykar.api.app import create_app
from peykar.examples import analytical_dataset, reference_model, reference_plan
from peykar_core import contracts

ROOT = Path(__file__).resolve().parents[1]


def exports() -> dict[Path, str]:
    values: dict[Path, object] = {}
    for name, cls in inspect.getmembers(contracts, inspect.isclass):
        if cls is contracts.Contract or not issubclass(cls, contracts.Contract):
            continue
        schema = cls.model_json_schema()
        schema["$schema"] = "https://json-schema.org/draft/2020-12/schema"
        schema["$id"] = f"urn:peykar:schema:{name}:1.0.0"
        values[ROOT / "schemas" / f"{name}-1.0.0.json"] = schema
    # OpenAPI generation does not enter lifespan or initialize a database.
    app = create_app(api_token="schema-generation-only-placeholder-token")
    values[ROOT / "schemas" / "openapi.json"] = app.openapi()
    for kind in ("decay", "logistic"):
        values[ROOT / "model_packages" / "reference" / kind / "0.3.0" / "model.json"] = (
            reference_model(kind).model_dump(mode="json")
        )
        values[ROOT / "experiments" / "manifests" / f"{kind}.json"] = reference_plan(
            kind
        ).model_dump(mode="json")
    spec, obs = analytical_dataset()
    values[ROOT / "experiments" / "baselines" / "analytic-decay.json"] = obs.model_dump(mode="json")
    values[ROOT / "experiments" / "benchmarks" / "analytic-decay-spec.json"] = spec.model_dump(
        mode="json"
    )
    csv_spec = contracts.CSVImportSpec(
        id="decay.csv",
        version="1.0.0",
        title="Synthetic analytical CSV fixture",
        origin="synthetic",
        role="benchmark",
        context=spec.context,
        license=spec.license,
        source="Analytical exp(-0.1*t), synthetic infrastructure fixture",
        independence_group=spec.independence_group,
        endpoint=obs.endpoint,
        time_column="seconds",
        value_column="mol_per_liter",
        time_unit=obs.time.unit,
        value_unit=obs.values.unit,
    )
    values[ROOT / "experiments/data/analytic-decay-import.json"] = csv_spec.model_dump(mode="json")
    frozen_spec = contracts.EvaluationSetSpec(
        id="decay.holdout",
        version="1.0.0",
        context=spec.context,
        endpoint=obs.endpoint,
        evaluation=(contracts.Reference(id=csv_spec.id, version=csv_spec.version),),
        tolerance=contracts.Quantity(value=1e-12, unit=obs.values.unit),
    )
    values[ROOT / "experiments/benchmarks/decay-evaluation-set.json"] = frozen_spec.model_dump(
        mode="json"
    )
    exported = {
        path: json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False) + "\n"
        for path, value in values.items()
    }
    exported[ROOT / "experiments/data/analytic-decay.csv"] = "seconds,mol_per_liter\n" + "".join(
        f"{t},{value}\n" for t, value in zip(obs.time.values, obs.values.values, strict=True)
    )
    return exported


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Fail on a missing/stale export")
    args = parser.parse_args()
    stale = []
    for path, content in exports().items():
        if args.check:
            if not path.is_file() or path.read_text() != content:
                stale.append(str(path.relative_to(ROOT)))
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
    if stale:
        raise SystemExit("Stale generated artifacts; run make schemas:\n" + "\n".join(stale))
    print("Contract exports are current" if args.check else "Contract exports generated")


if __name__ == "__main__":
    main()
