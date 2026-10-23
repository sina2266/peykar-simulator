"""Read-only trajectory observations around the reviewed upstream headless entrypoint."""

import argparse
import csv
import importlib
import json
import math
import random
import runpy
import signal
import sys
import time
from pathlib import Path
from typing import Any


def main() -> None:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("--wall-seconds", type=float, required=True)
    args, engine_args = parser.parse_known_args()
    if not math.isfinite(args.wall_seconds) or not 1 <= args.wall_seconds <= 86400:
        raise ValueError("Invalid execution deadline")
    signal.signal(signal.SIGALRM, signal.SIG_DFL)
    signal.alarm(math.ceil(args.wall_seconds))
    numpy = importlib.import_module("numpy")
    random.seed(args.seed)
    numpy.random.seed(args.seed)
    setup: Any = importlib.import_module("cc3d.CompuCellSetup")
    steppables: Any = importlib.import_module("cc3d.core.PySteppables")
    root = Path(__file__).resolve().parent
    manifest = json.loads((root / "manifest.json").read_bytes())
    horizon = manifest["controls"]["steps_mcs"]
    injury = manifest.get("parameter_overrides", {}).get("InjuryTime")
    snapshots = {0, 240, 720, 1680, 3600, horizon}
    if injury is not None:
        snapshots |= {
            injury,
            injury + 10,
            injury + 240,
            injury + 720,
            injury + 1200,
            injury + 1680,
            injury + 4800,
        }
    started = time.monotonic()

    class Observer(steppables.SteppableBasePy):  # type: ignore[misc]
        def __init__(self) -> None:
            super().__init__(frequency=1)

        def step(self, mcs: int) -> None:
            if mcs % 1000 != 0 and mcs not in snapshots:
                return
            counts = {
                name: len(self.cell_list_by_type(kind))
                for kind, name in [(1, "Stem"), (3, "Basal"), (4, "Wing"), (5, "Superficial")]
            }
            progress = {
                "step_mcs": mcs,
                "time_hours": mcs / 10,
                "elapsed_seconds": time.monotonic() - started,
                "counts": counts,
            }
            with (root / "progress.jsonl").open("a") as stream:
                stream.write(json.dumps(progress, allow_nan=False) + "\n")
            if mcs in snapshots:
                with (root / f"cells-{mcs}.csv").open("w", newline="") as stream:
                    writer = csv.writer(stream)
                    writer.writerow(
                        ["CellID", "TypeID", "X_lattice", "Y_lattice", "Volume_lattice"]
                    )
                    for cell in self.cell_list:
                        writer.writerow([cell.id, cell.type, cell.xCOM, cell.yCOM, cell.volume])
                # Exact lattice observations, with explicit x,y axis order; no state/RNG writes.
                kinds = numpy.zeros((self.dim.x, self.dim.y), dtype=numpy.uint8)
                egf = numpy.zeros(kinds.shape, dtype=numpy.float64)
                sls = numpy.zeros(kinds.shape, dtype=numpy.float64)
                for x in range(self.dim.x):
                    for y in range(self.dim.y):
                        cell = self.cell_field[x, y, 0]
                        kinds[x, y] = 0 if cell is None else cell.type
                        egf[x, y] = self.field.EGF[x, y, 0]
                        sls[x, y] = self.field.SLS[x, y, 0]
                numpy.savez_compressed(
                    root / f"lattice-{mcs}.npz", cell_type=kinds, egf=egf, sls=sls
                )

    original_run = setup.run

    def observed_run() -> None:
        setup.register_steppable(steppable=Observer())
        original_run()

    setup.run = observed_run
    sys.argv = ["cc3d.run_script", *engine_args]
    runpy.run_module("cc3d.run_script", run_name="__main__")


if __name__ == "__main__":
    main()
