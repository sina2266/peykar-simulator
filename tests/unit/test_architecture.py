"""Real package boundaries and exported contract examples, not a mirror of internals."""

import ast
import json
from pathlib import Path

import jsonschema

ROOT = Path(__file__).resolve().parents[2]


def test_core_and_engines_keep_dependency_direction():
    forbidden = {"core": ("peykar", "peykar_engines"), "engines": ("peykar",)}
    for package, names in forbidden.items():
        for path in (ROOT / "packages" / package / "src").rglob("*.py"):
            for node in ast.walk(ast.parse(path.read_text())):
                if isinstance(node, ast.Import):
                    modules = [item.name for item in node.names]
                elif isinstance(node, ast.ImportFrom):
                    modules = [node.module or ""]
                else:
                    continue
                assert not any(module.split(".")[0] in names for module in modules), path


def test_generated_examples_validate_against_public_schemas():
    mappings = [
        ("SimulationPlan", "experiments/manifests/decay.json"),
        ("SimulationPlan", "experiments/manifests/logistic.json"),
        ("ModelPackage", "model_packages/reference/decay/0.3.0/model.json"),
        ("ModelPackage", "model_packages/reference/logistic/0.3.0/model.json"),
        ("Observation", "experiments/baselines/analytic-decay.json"),
        ("DatasetSpec", "experiments/benchmarks/analytic-decay-spec.json"),
    ]
    for name, path in mappings:
        schema = json.loads((ROOT / "schemas" / f"{name}-1.0.0.json").read_text())
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(json.loads((ROOT / path).read_text()), schema)
