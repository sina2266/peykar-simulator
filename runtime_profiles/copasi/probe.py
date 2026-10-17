"""Execute a native COPASI mass-conserving reaction and retain numerical evidence."""

import argparse
import hashlib
import json
import math
from pathlib import Path

import basico
import COPASI
from copasi_compat import prepare_output_flags

# ruff: noqa: E501
SBML = """<?xml version="1.0" encoding="UTF-8"?>
<sbml xmlns="http://www.sbml.org/sbml/level3/version2/core" level="3" version="2">
<model id="peykar_transfer" substanceUnits="mole" timeUnits="second" volumeUnits="litre" extentUnits="mole">
<listOfUnitDefinitions><unitDefinition id="per_second"><listOfUnits><unit kind="second" exponent="-1" scale="0" multiplier="1"/></listOfUnits></unitDefinition></listOfUnitDefinitions>
<listOfCompartments><compartment id="cell" size="1" spatialDimensions="3" units="litre" constant="true"/></listOfCompartments>
<listOfSpecies><species id="A" compartment="cell" initialConcentration="1" substanceUnits="mole" hasOnlySubstanceUnits="false" boundaryCondition="false" constant="false"/>
<species id="B" compartment="cell" initialConcentration="0" substanceUnits="mole" hasOnlySubstanceUnits="false" boundaryCondition="false" constant="false"/></listOfSpecies>
<listOfParameters><parameter id="k" value="0.1" units="per_second" constant="true"/></listOfParameters>
<listOfReactions><reaction id="transfer" reversible="false"><listOfReactants><speciesReference species="A" stoichiometry="1" constant="true"/></listOfReactants><listOfProducts><speciesReference species="B" stoichiometry="1" constant="true"/></listOfProducts>
<kineticLaw><math xmlns="http://www.w3.org/1998/Math/MathML"><apply><times/><ci>k</ci><ci>A</ci><ci>cell</ci></apply></math></kineticLaw></reaction></listOfReactions>
</model></sbml>"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("."))
    args = parser.parse_args()
    args.output.mkdir(exist_ok=True, parents=True)
    patched = prepare_output_flags()
    model = basico.load_model_from_string(SBML)
    frame = basico.run_time_course(duration=10, intervals=10, start_time=0, model=model)
    frame.to_csv(args.output / "native.csv")
    columns = list(frame.columns)
    a = next(c for c in columns if c in ("A", "[A]"))
    b = next(c for c in columns if c in ("B", "[B]"))
    times = [float(t) for t in frame.index]
    av = [float(v) for v in frame[a]]
    bv = [float(v) for v in frame[b]]
    errors = [abs(v - math.exp(-0.1 * t)) for t, v in zip(times, av, strict=True)]
    conservation = max(abs(x + y - 1) for x, y in zip(av, bv, strict=True))
    assert len(times) == 11 and times[0] == 0 and times[-1] == 10
    assert max(errors) < 1e-6 and conservation < 1e-10 and min(av + bv) >= -1e-12
    (args.output / "model.sbml").write_text(SBML)
    receipt = {
        "engine": "COPASI",
        "version": COPASI.CVersion.VERSION.getVersion(),
        "native_execution": "passed",
        "output_flag_compatibility_bridge": patched,
        "model": "A -> B, first order; synthetic numerical verification",
        "time_unit": "second",
        "concentration_unit": "mole/liter",
        "samples": len(times),
        "max_absolute_error": max(errors),
        "max_conservation_error": conservation,
        "scientific_validity": "not_established",
        "artifacts": {
            p.name: hashlib.sha256(p.read_bytes()).hexdigest()
            for p in args.output.iterdir()
            if p.name in ("native.csv", "model.sbml")
        },
    }
    (args.output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
