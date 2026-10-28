"""In-sample diagnostics behind IOP model 1.1 (training data only; never tranche 2).

Reads the 72 tranche-1 records (47 development records, public and privately kept,
and the 25 sealed records consumed by the 1.0.0 test) and prints:

1. mean signed residual of the 1.0.0 law (best exponential fit per group mean with
   at least four follow-ups) by context and time since surgery;
2. dof-adjusted residual SD of group means with at least five follow-ups without and
   with a fast transient ``exp(-(t - 1 day)/tq)`` for several fixed ``tq``;
3. the dependence of group-mean residual SD on the number of eyes;
4. residual RMS by time since surgery and the lag-1 residual correlation.
"""

from __future__ import annotations

import json
import math
from collections import defaultdict
from pathlib import Path

import numpy as np

from peykar.model_learning import iop_population as population
from peykar.validation.eye_iop_data import PressureSeries, to_pressure_series

ROOT = Path(__file__).resolve().parents[4]
DIRS = (
    ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28/development/records",
    ROOT / ".peykar/iop-development-private/records",
    ROOT / ".peykar/iop-sealed-2026-10-28/records",
)
DAY = population.DAY_MONTHS
BINS = [
    (0, 0.1, "<=3d"),
    (0.1, 0.5, "3d-2wk"),
    (0.5, 1.5, "2-6wk"),
    (1.5, 4.5, "6wk-4.5mo"),
    (4.5, 9, "4.5-9mo"),
    (9, 18, "9-18mo"),
    (18, 1e9, ">18mo"),
]


def load() -> list[PressureSeries]:
    return [
        s
        for directory in DIRS
        for path in sorted(directory.glob("*.json"))
        for s in to_pressure_series(json.loads(path.read_text()))
    ]


def best(item: PressureSeries, tq: float | None) -> tuple[float, np.ndarray]:
    y = np.log(np.asarray(item.fractions))
    t = np.asarray(item.months)
    extra = None if tq is None else np.exp(-(t - DAY) / tq)
    result = (math.inf, y)
    for row in population.recovery_ratio(item.months, population.LOG_TAU_GRID):
        columns = [np.ones_like(row), row] + ([] if extra is None else [extra])
        design = np.stack(columns, axis=1)
        beta, *_ = np.linalg.lstsq(design, y, rcond=None)
        residual = y - design @ beta
        sse = float(residual @ residual)
        if sse < result[0]:
            result = (sse, residual)
    return result


def main() -> None:
    series = load()
    groups = [s for s in series if s.kind == "aggregate"]
    print(f"{len(series)} series, {len(groups)} group means")
    signed: dict[str, dict[str, list[float]]] = defaultdict(lambda: defaultdict(list))
    rms: dict[str, list[float]] = defaultdict(list)
    pairs = []
    sds = []
    for item in groups:
        if len(item.months) < 4:
            continue
        sse, residual = best(item, None)
        context = item.context.split("/")[1]
        for month, value in zip(item.months, residual, strict=True):
            for low, high, name in BINS:
                if low < month <= high:
                    signed[context][name].append(float(value))
                    signed["ALL"][name].append(float(value))
                    rms[name].append(float(value) * math.sqrt(len(residual) / (len(residual) - 3)))
        pairs.extend(zip(residual[:-1], residual[1:], strict=True))
        if item.aggregate_n:
            sds.append((item.aggregate_n, max(1e-3, math.sqrt(sse / (len(residual) - 3)))))
    print("1. mean signed residual x100 (count) by context and time since surgery")
    print(" " * 22 + "".join(f"{b[2]:>13s}" for b in BINS))
    for context in sorted(signed, key=lambda c: (c != "ALL", c)):
        cells = [
            f"{100 * np.mean(signed[context][b[2]]):+7.2f}({len(signed[context][b[2]]):3d})"
            if signed[context][b[2]]
            else " " * 13
            for b in BINS
        ]
        print(f"{context:22s}" + "".join(cells))
    print("2. group means with >=5 follow-ups: dof-adjusted residual SD")
    rich = [s for s in groups if len(s.months) >= 5]
    for tq in (None, 3 * DAY, 7 * DAY, 14 * DAY, 1.0, 2.0):
        sse = dof = 0.0
        for item in rich:
            value, _ = best(item, tq)
            sse += value
            dof += len(item.months) - (3 if tq is None else 4)
        label = "without transient" if tq is None else f"tq = {tq / DAY:5.1f} days"
        print(f"   {label:18s} series {len(rich)} dof {int(dof)} sd {math.sqrt(sse / dof):.4f}")
    n = np.array([s[0] for s in sds], dtype=float)
    sd = np.array([s[1] for s in sds])
    slope, intercept = np.polyfit(np.log(n), np.log(sd), 1)
    print(f"3. ln sd = {intercept:.3f} {slope:+.3f} ln n over {len(sds)} group means")
    for low, high in ((0, 20), (20, 40), (40, 80), (80, 160), (160, 1e9)):
        chosen = (n > low) & (n <= high)
        if chosen.any():
            median = float(np.median(sd[chosen]))
            print(f"   n in ({low}, {high}]: {chosen.sum():3d} series, median sd {median:.4f}")
    print("4. residual RMS by time since surgery")
    for _, _, name in BINS:
        value = math.sqrt(float(np.mean(np.square(rms[name]))))
        print(f"   {name:10s} n {len(rms[name]):4d} rms {value:.4f}")
    a = np.array(pairs)
    print(
        f"   lag-1 residual correlation {np.corrcoef(a[:, 0], a[:, 1])[0, 1]:+.3f} ({len(a)} pairs)"
    )


if __name__ == "__main__":
    main()
