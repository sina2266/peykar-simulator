"""Round 0 of the thickness model: in-sample kinetic-law comparison on development data.

Three-parameter laws ``ln f = ell + (s - ell) h(t)`` are fitted by least squares
over a 400-point tau grid to every development series with at least four
follow-ups. Prints the dof-adjusted residual SD of ``ln f`` per family and kind.
Reads development records only (public and privately kept); never sealed data.
"""

from __future__ import annotations

import collections
import json
import math
from pathlib import Path

import numpy as np

from peykar.validation.cornea_thickness_data import to_thickness_series

ROOT = Path(__file__).resolve().parents[3]
ROOTS = (
    ROOT / "experiments/data/cornea-thickness-2026-10-28/development/records",
    ROOT / ".peykar/thickness-development-private/records",
)
TAU = np.geomspace(0.003, 60, 400)
GRAFT = {"dmek", "dsaek", "pk", "dalk", "other"}


def shape(months: tuple[float, ...], law: str) -> np.ndarray:
    t = np.asarray(months)[None, :]
    tau = TAU[:, None]
    if law == "exponential":
        return np.exp(-t / tau)
    exponent = {"hyperbolic": 1.0, "power_0.5": 0.5, "power_2": 2.0}[law]
    return (1 + t / tau) ** (-exponent)


def best_sse(months: tuple[float, ...], fractions: tuple[float, ...], law: str) -> float:
    y = np.log(np.asarray(fractions))
    best = math.inf
    for row in shape(months, law):
        design = np.stack([1 - row, row], axis=1)
        beta, *_ = np.linalg.lstsq(design, y, rcond=None)
        residual = y - design @ beta
        best = min(best, float(residual @ residual))
    return best


def main() -> None:
    series = [
        s
        for root in ROOTS
        for path in sorted(root.glob("*.json"))
        for s in to_thickness_series(json.loads(path.read_text()))
    ]
    for law in ("exponential", "hyperbolic", "power_0.5", "power_2"):
        totals: dict[str, list[float]] = collections.defaultdict(lambda: [0.0, 0.0])
        for item in series:
            if len(item.months) < 4:
                continue
            family = "graft" if item.context.split("/")[1] in GRAFT else "host"
            key = f"{family}/{item.kind}"
            totals[key][0] += best_sse(item.months, item.fractions, law)
            totals[key][1] += len(item.months) - 3
        summary = {k: round(math.sqrt(v[0] / v[1]), 4) for k, v in sorted(totals.items())}
        dof = {k: int(v[1]) for k, v in sorted(totals.items())}
        print(law, summary, dof)


if __name__ == "__main__":
    main()
