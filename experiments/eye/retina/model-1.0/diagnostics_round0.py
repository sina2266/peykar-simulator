"""Round-0 diagnostics of the retinal-thickness development records (in-sample only).

Run from the repository root with ``uv run --frozen python
experiments/eye/retina/model-1.0/diagnostics_round0.py``. Reads only development
records (public ones in the repository and the private copies in
``.peykar/retina-development-private/records``); never a sealed record.

Prints, for the declared candidate laws ``ln f = ell + e r(t) [+ s q(t)]`` with
``r = exp(-(t - t1)/tau)`` (tau on the model grid) and ``q = exp(-(t - t1)/tq)``:
pooled dof-adjusted residual SDs of best fits over series with enough follow-ups,
the level-versus-baseline relation, and residual SD against the number of eyes.
"""

from __future__ import annotations

import json
import math
from collections import Counter, defaultdict
from pathlib import Path

import numpy as np

from peykar.validation.eye_retina_data import RetinaSeries, to_retina_series

ROOT = Path(__file__).resolve().parents[4]
PUBLIC = ROOT / "experiments/data/retina-cst-2026-10-28/development/records"
PRIVATE = ROOT / ".peykar/retina-development-private/records"
DAY = 1.0 / 30.4375
TAU = np.log(np.geomspace(0.01, 120.0, 80))


def load() -> list[RetinaSeries]:
    series: list[RetinaSeries] = []
    for folder in (PUBLIC, PRIVATE):
        for path in sorted(folder.glob("*.json")):
            record = json.loads(path.read_text())
            if record["split"] != "development":
                raise SystemExit(f"{path} is not a development record")
            series.extend(to_retina_series(record))
    return series


def columns(months: np.ndarray, log_tau: float, t1: float, tq: float | None) -> np.ndarray:
    parts = [np.ones_like(months), np.exp(-(months - t1) / math.exp(log_tau))]
    if tq is not None:
        parts.append(np.exp(-(months - t1) / tq))
    return np.stack(parts, axis=1)


def best_sse(item: RetinaSeries, t1: float, tq: float | None) -> float:
    t = np.asarray(item.months)
    y = np.log(np.asarray(item.fractions))
    best = math.inf
    for log_tau in TAU:
        x = columns(t, log_tau, t1, tq)
        beta, *_ = np.linalg.lstsq(x, y, rcond=None)
        r = y - x @ beta
        best = min(best, float(r @ r))
    return best


def pooled_sd(members: list[RetinaSeries], t1: float, tq: float | None) -> tuple[float, int]:
    k = 3 if tq is None else 4
    usable = [m for m in members if len(m.months) >= k + 1]
    sse = sum(best_sse(m, t1, tq) for m in usable)
    dof = sum(len(m.months) - k for m in usable)
    return (math.sqrt(sse / dof) if dof else float("nan")), len(usable)


def treatment(item: RetinaSeries) -> str:
    return item.context.rsplit("/", 1)[1]


def main() -> None:
    series = load()
    controls = [s for s in series if s.role == "control"]
    print(f"{len(series)} series, {len({s.study_id for s in series})} studies;")
    print(f"{len(controls)} control series in {len({s.study_id for s in controls})} studies")
    print("contexts:", Counter(s.context for s in controls).most_common())
    print(
        "treatments (studies):",
        {
            t: len({s.study_id for s in controls if treatment(s) == t})
            for t in sorted({treatment(s) for s in controls})
        },
    )
    print("follow-up months:", sorted(Counter(m for s in series for m in s.months).items())[:30])

    print("\n1. Law comparison, pooled dof-adjusted residual SD of ln f (series used)")
    sets = {
        "control group means": [s for s in controls if s.is_aggregate],
        "all group means": [s for s in series if s.is_aggregate],
        "single eyes": [s for s in series if not s.is_aggregate],
    }
    for t1_label, t1 in (("t1=1 day", DAY), ("t1=1 week", 7 * DAY)):
        for name, members in sets.items():
            row = []
            for label, tq in (
                ("no transient", None),
                ("tq=3.5d", 3.5 * DAY),
                ("tq=7d", 7 * DAY),
                ("tq=14d", 14 * DAY),
                ("tq=30d", 30 * DAY),
            ):
                sd, used = pooled_sd(members, t1, tq)
                row.append(f"{label}: {sd:.4f} ({used})")
            print(f"  {t1_label} | {name}: " + "; ".join(row))

    print("\n2. Same series set for both laws (>=5 follow-ups), control group means")
    five = [s for s in controls if s.is_aggregate and len(s.months) >= 5]
    for label, tq in (
        ("no transient", None),
        ("tq=7d", 7 * DAY),
        ("tq=14d", 14 * DAY),
        ("tq=30d", 30 * DAY),
    ):
        k = 3 if tq is None else 4
        sse = sum(best_sse(m, DAY, tq) for m in five)
        dof = sum(len(m.months) - k for m in five)
        print(f"  {label}: SD {math.sqrt(sse / dof):.4f} over {len(five)} series")

    print("\n3. Level versus baseline thickness (controls): ln f at the last follow-up")
    x = np.array([math.log(s.reference_value / 300.0) for s in controls])
    y = np.array([math.log(s.fractions[-1]) for s in controls])
    slope, intercept = np.polyfit(x, y, 1)
    print(f"  all controls: slope {slope:.3f}, intercept {intercept:.3f}, n={len(controls)}")
    for t in sorted({treatment(s) for s in controls}):
        idx = [i for i, s in enumerate(controls) if treatment(s) == t]
        if len(idx) >= 4 and float(np.ptp(x[idx])) > 0:
            b, a = np.polyfit(x[idx], y[idx], 1)
            print(f"  {t}: slope {b:.3f}, intercept {a:.3f}, n={len(idx)}")
    late = [i for i, s in enumerate(controls) if s.months[-1] >= 3.0]
    b, a = np.polyfit(x[late], y[late], 1)
    print(f"  last follow-up >=3 months: slope {b:.3f}, intercept {a:.3f}, n={len(late)}")
    print(
        f"  baseline thickness (um): median {np.median([s.reference_value for s in controls]):.0f},"
        f" 10-90% {np.percentile([s.reference_value for s in controls], 10):.0f}-"
        f"{np.percentile([s.reference_value for s in controls], 90):.0f}"
    )

    print("\n4. Residual SD (law with tq=7d, >=5 follow-ups) against eyes, group means")
    by_study: dict[str, list[RetinaSeries]] = defaultdict(list)
    for s in series:
        if s.is_aggregate and len(s.months) >= 5:
            by_study[s.study_id].append(s)
    points = []
    for study, members in sorted(by_study.items()):
        sd, _ = pooled_sd(members, DAY, 7 * DAY)
        eyes = float(np.mean([m.aggregate_n or 0 for m in members]))
        points.append((eyes, sd))
        print(f"  {study}: eyes {eyes:.0f}, SD {sd:.4f}, series {len(members)}")
    if len(points) >= 4:
        px = np.log([p[0] for p in points if p[0] > 0])
        py = np.log([p[1] for p in points if p[0] > 0])
        print(f"  slope of ln SD on ln eyes: {np.polyfit(px, py, 1)[0]:.3f}")

    print("\n5. Residual SD, 3-parameter law (>=4 follow-ups), by kind")
    for name, members in sets.items():
        sd, used = pooled_sd(members, DAY, None)
        print(f"  {name}: {sd:.4f} ({used} series)")


if __name__ == "__main__":
    main()
