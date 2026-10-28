"""Round 0 of the IOP model: in-sample comparisons on development data only.

1. Three-parameter laws ``ln f = ell + (s - ell) h(t)`` are fitted by least squares
   over a 400-point tau grid to every development series with at least four
   follow-ups; prints the dof-adjusted residual SD of ``ln f`` per series kind.
2. Dependence of the late level on the preoperative IOP: mean ``ln f`` over
   follow-ups at or after 3 months of control group means, regressed on
   ``x = ln(IOP0 / 20 mmHg)``, without and with procedure-context intercepts.
3. Early deviation (first follow-up within 0.25 months minus the late level)
   against ``x``.

Reads development records only (public and privately kept); never sealed data.
"""

from __future__ import annotations

import collections
import json
import math
from pathlib import Path

import numpy as np

from peykar.validation.eye_iop_data import PressureSeries, to_pressure_series

ROOT = Path(__file__).resolve().parents[3]
ROOTS = (
    ROOT / "experiments/data/iop-glaucoma-surgery-2026-10-28/development/records",
    ROOT / ".peykar/iop-development-private/records",
)
TAU = np.geomspace(0.003, 120, 400)
LAWS = ("exponential", "hyperbolic", "power_0.5", "power_2")


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


def late_level(item: PressureSeries) -> float | None:
    late = [math.log(f) for m, f in zip(item.months, item.fractions, strict=True) if m >= 3]
    return float(np.mean(late)) if late else None


def main() -> None:
    series = [
        s
        for root in ROOTS
        for path in sorted(root.glob("*.json"))
        for s in to_pressure_series(json.loads(path.read_text()))
    ]
    for law in LAWS:
        totals: dict[str, list[float]] = collections.defaultdict(lambda: [0.0, 0.0])
        for item in series:
            if len(item.months) < 4:
                continue
            totals[item.kind][0] += best_sse(item.months, item.fractions, law)
            totals[item.kind][1] += len(item.months) - 3
        summary = {k: round(math.sqrt(v[0] / v[1]), 4) for k, v in sorted(totals.items())}
        dof = {k: int(v[1]) for k, v in sorted(totals.items())}
        print(law, summary, dof)

    controls = [s for s in series if s.role == "control" and s.kind == "aggregate"]
    points = [
        (s.context, math.log(s.reference_mmhg / 20.0), level)
        for s in controls
        if (level := late_level(s)) is not None
    ]
    x = np.array([p[1] for p in points])
    y = np.array([p[2] for p in points])
    coef, *_ = np.linalg.lstsq(np.stack([np.ones_like(x), x], axis=1), y, rcond=None)
    print(
        f"late level, all control group means (n={len(x)}): "
        f"ln f_late = {coef[0]:.3f} + {coef[1]:.3f} x (elasticity {coef[1] + 1:.3f})"
    )
    contexts = sorted({p[0] for p in points})
    design = np.stack(
        [x, *(np.array([1.0 if p[0] == c else 0.0 for p in points]) for c in contexts)], axis=1
    )
    coef2, *_ = np.linalg.lstsq(design, y, rcond=None)
    intercepts = {c: round(float(v), 3) for c, v in zip(contexts, coef2[1:], strict=True)}
    print(f"with context intercepts: slope {coef2[0]:.3f}; intercepts {intercepts}")
    for context in contexts:
        chosen = [i for i, p in enumerate(points) if p[0] == context]
        if len(chosen) >= 4:
            local = np.stack([np.ones(len(chosen)), x[chosen]], axis=1)
            fitted, *_ = np.linalg.lstsq(local, y[chosen], rcond=None)
            print(f"  {context}: n={len(chosen)} slope {fitted[1]:.3f}")

    early = [
        (math.log(s.reference_mmhg / 20.0), math.log(s.fractions[0]) - level)
        for s in controls
        if s.months[0] <= 0.25 and (level := late_level(s)) is not None
    ]
    xe = np.array([e[0] for e in early])
    ye = np.array([e[1] for e in early])
    coef3, *_ = np.linalg.lstsq(np.stack([np.ones_like(xe), xe], axis=1), ye, rcond=None)
    print(f"early deviation vs x (n={len(xe)}): {coef3[0]:.3f} + {coef3[1]:.3f} x")


if __name__ == "__main__":
    main()
