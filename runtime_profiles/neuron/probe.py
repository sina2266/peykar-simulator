"""Execute an actual NEURON HH compartment/current-clamp engineering benchmark."""

import argparse
import csv
import hashlib
import json
import math
from pathlib import Path

import neuron
from neuron import h


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("."))
    args = parser.parse_args()
    args.output.mkdir(exist_ok=True, parents=True)
    h.load_file("stdrun.hoc")
    soma = h.Section(name="soma")
    soma.L = 20
    soma.diam = 20
    soma.nseg = 1
    soma.insert("hh")
    stimulus = h.IClamp(soma(0.5))
    stimulus.delay = 2
    stimulus.dur = 2
    stimulus.amp = 0.1
    time = h.Vector().record(h._ref_t)
    voltage = h.Vector().record(soma(0.5)._ref_v)
    h.dt = 0.025
    h.finitialize(-65)
    h.continuerun(10)
    times = list(time)
    values = list(voltage)
    assert len(times) == 401 and all(math.isfinite(v) for v in values)
    assert all(a < b for a, b in zip(times, times[1:], strict=False)) and max(values) > 0
    raw = args.output / "native.csv"
    with raw.open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(("time_ms", "voltage_mV"))
        writer.writerows(zip(times, values, strict=True))
    receipt = {
        "engine": "NEURON",
        "version": neuron.__version__,
        "native_version": h.nrnversion(),
        "native_execution": "passed",
        "model": "Single HH compartment; 0.1 nA clamp at 2..4 ms",
        "time_unit": "millisecond",
        "voltage_unit": "millivolt",
        "samples": len(times),
        "peak_voltage": max(values),
        "scientific_validity": "not_established",
        "limitations": [
            "Textbook HH engineering verification; human neuron/eye validity is unestablished."
        ],
        "artifacts": {"native.csv": hashlib.sha256(raw.read_bytes()).hexdigest()},
    }
    (args.output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
