"""Reviewed CellML 2 ODE driver. Execute only inside the offline bounded runtime.

libCellML's parser/validator/analyser/generator own model semantics. SciPy owns
numerical integration. This profile does not establish biological validity.
"""

import ast
import csv
import hashlib
import json
import math
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import libcellml
import numpy as np
from scipy.integrate import solve_ivp


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def errors(logger):
    return [logger.error(i).description() for i in range(logger.errorCount())]


def run(work):
    raw = (work / "model.cellml").read_bytes()
    controls = json.loads((work / "controls.json").read_text())
    text = raw.decode("utf-8-sig")
    if len(raw) > 10 * 1024**2 or any(v in text.upper() for v in ("<!DOCTYPE", "<!ENTITY")):
        raise ValueError("Unsupported XML")
    root = ET.fromstring(text)
    ns = "http://www.cellml.org/cellml/2.0#"
    if root.tag != f"{{{ns}}}model" or any(
        e.tag in (f"{{{ns}}}import", f"{{{ns}}}reset")
        or e.tag.startswith("{http://www.w3.org/2001/XInclude}")
        for e in root.iter()
    ):
        raise ValueError("Only self-contained CellML 2 ODEs without resets are supported")
    parser = libcellml.Parser()
    model = parser.parseModel(text)
    if errors(parser):
        raise ValueError("CellML parse: " + "; ".join(errors(parser)))
    validator = libcellml.Validator()
    validator.validateModel(model)
    if errors(validator):
        raise ValueError("CellML validation: " + "; ".join(errors(validator)))
    analyser = libcellml.Analyser()
    analyser.analyseModel(model)
    analysed = analyser.analyserModel()
    if errors(analyser) or analysed.type() != libcellml.AnalyserModel.Type.ODE:
        raise ValueError(
            "Only fully determined ODE models are supported: " + "; ".join(errors(analyser))
        )
    generator = libcellml.Generator()
    code = generator.implementationCode(analysed, libcellml.GeneratorProfile.Profile.PYTHON)
    # This is generated mathematical code, never uploaded Python. Still only run
    # in the explicit, no-network, non-root, resource-bounded operator container.
    tree = ast.parse(code)
    for node in ast.walk(tree):
        if isinstance(node, ast.Import) or (
            isinstance(node, ast.ImportFrom) and (node.module not in ("math", "enum") or node.level)
        ):
            raise ValueError("Unexpected generated import")
        if isinstance(node, ast.Call) and (
            not isinstance(node.func, ast.Name)
            or node.func.id
            in {"eval", "exec", "open", "compile", "__import__", "getattr", "globals", "locals"}
        ):
            raise ValueError("Unsupported generated call")
        if isinstance(node, ast.Attribute) and node.attr.startswith("__"):
            raise ValueError("Unsupported generated attribute")
    generated = {}
    exec(compile(tree, "generated_cellml", "exec"), generated)
    voi = generated["VOI_INFO"]
    if controls["time_unit"] != voi["units"]:
        raise ValueError("Controls must use the original variable-of-integration unit identifier")
    counts = [
        generated[k + "_COUNT"]
        for k in ("STATE", "CONSTANT", "COMPUTED_CONSTANT", "ALGEBRAIC_VARIABLE")
    ]
    if (
        not 1 <= counts[0] <= 256
        or sum(counts) > 4096
        or sum(counts) * (controls["intervals"] + 1) > 400000
    ):
        raise ValueError("Model exceeds the reviewed solver dimensions")
    states, constants, computed, algebraic = ([math.nan] * n for n in counts)
    rates = [math.nan] * len(states)
    generated["initialise_arrays"](states, rates, constants, computed, algebraic)
    generated["compute_computed_constants"](
        controls["start"], states, rates, constants, computed, algebraic
    )

    def rhs(t, y):
        generated["compute_rates"](t, y, rates, constants, computed, algebraic)
        if not all(math.isfinite(value) for value in rates):
            raise ValueError("Non-finite CellML rate")
        return list(rates)

    grid = np.linspace(controls["start"], controls["end"], controls["intervals"] + 1)
    solved = solve_ivp(
        rhs,
        (controls["start"], controls["end"]),
        states,
        t_eval=grid,
        method=controls["method"],
        rtol=controls["relative_tolerance"],
        atol=controls["absolute_tolerance"],
        max_step=controls["max_step"],
    )
    if not solved.success or len(solved.t) != len(grid) or not np.all(np.isfinite(solved.y)):
        raise ValueError("CellML numerical integration did not complete")
    columns = []
    for kind, info in zip(
        ("state", "constant", "computed_constant", "algebraic"),
        ("STATE_INFO", "CONSTANT_INFO", "COMPUTED_CONSTANT_INFO", "ALGEBRAIC_VARIABLE_INFO"),
        strict=True,
    ):
        columns.extend({**m, "kind": kind} for m in generated[info])
    names = [m["component"] + "/" + m["name"] for m in columns]
    if len(set(names)) != len(names):
        raise ValueError("Ambiguous CellML column identity")
    rows = []
    for index, t in enumerate(grid):
        y = solved.y[:, index]
        rhs(t, y)
        generated["compute_variables"](t, y, rates, constants, computed, algebraic)
        row = [float(t), *map(float, y), *constants, *computed, *algebraic]
        if not all(math.isfinite(v) for v in row):
            raise ValueError("Non-finite CellML output")
        rows.append(row)
    # Publish only complete results; failure never leaves a success artifact.
    (work / "generated.py").write_text(code)
    with (work / "series.csv").open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["time", *names])
        writer.writerows(rows)
    result = {
        "status": "succeeded",
        "model_digest": sha(raw),
        "driver_digest": sha(Path(__file__).read_bytes()),
        "controls": controls,
        "libcellml_version": libcellml.versionString(),
        "voi": voi,
        "columns": columns,
        "solver": {
            "name": "scipy.solve_ivp",
            "evaluations": int(solved.nfev),
            "message": solved.message,
        },
        "warnings": [validator.warning(i).description() for i in range(validator.warningCount())]
        + [analyser.warning(i).description() for i in range(analyser.warningCount())],
        "scientific_validity": "not_established",
    }
    (work / "result.json").write_text(json.dumps(result, indent=2, allow_nan=False) + "\n")


if __name__ == "__main__":
    try:
        run(Path("/work"))
    except Exception as error:
        print(f"CellML execution failed: {error}", file=sys.stderr)
        raise SystemExit(1) from None
