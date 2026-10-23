"""Freeze reference-derived, explicitly limited reproduction checks before candidate runs."""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
import pandas as pd

GROUPS = {
    "self-organization": ("STEM_ONLY_proper_thickness_6months/Comb0", 21, 4320),
    "slight-injury": ("(Slight) 750_G/Comb0", 13, 5040),
    "mild-injury": ("(Mild) 1500_G/Comb1", 16, 5040),
    "moderate-injury": ("(Moderate) 2500_G/Comb1", 15, 5040),
}
COLUMNS = ["Superficial", "Wing", "Basal", "Stem"]


def freeze(data: Path, membrane: Path, destination: Path) -> None:
    if destination.exists():
        raise ValueError("Frozen comparison already exists; preserve it")
    base = pd.read_csv(membrane, header=None, names=["X", "Y"])
    baseline = {
        i: float(base[(i * 20 <= base.X) & ((i + 1) * 20 > base.X)].Y.mean()) for i in range(10)
    }
    protocol = {
        "id": "vcornea.publication-reproduction",
        "version": "1.0.0",
        "scope": (
            "Reference-variability checks: new memory-safe model and reconstructed runtime; "
            "no exact archived seed or independent biological validation claim."
        ),
        "frozen_before_full_candidate_execution": True,
        "count_columns": COLUMNS,
        "stable_from_hour": 500,
        "mean_envelope_sd_multiplier": 3,
        "pointwise_sd_multiplier": 3,
        "count_quantization_allowance": 1,
        "minimum_pointwise_coverage": 0.9,
        "thickness_lattice_to_um": 2,
        "thickness_quantization_allowance_um": 2,
        "membrane_bins_lattice": baseline,
        "groups": {},
        "limitations": [
            "Thresholds check reference variability, not clinical acceptance "
            "or confidence intervals.",
            "Autocorrelated time samples are not independent biological replicates.",
            "Original archived per-run parameter files and seeds are unavailable.",
            "The lifetime repair changes undefined memory access; agreement must be measured.",
        ],
    }
    for label, (relative, expected, horizon) in GROUPS.items():
        paths = sorted((data / relative).rglob("cell_count_*.csv"))
        height_paths = sorted((data / relative).rglob("thickness_rep_*.parquet"))
        if len(paths) != expected or len(height_paths) != expected:
            raise ValueError("Reference cohort is incomplete")
        frames = [pd.read_csv(p) for p in paths]
        for f in frames:
            if (
                list(f.columns) != COLUMNS + ["Time"]
                or len(f) != horizon + 1
                or not np.array_equal(f.Time, np.arange(horizon + 1))
            ):
                raise ValueError("Reference time grid/schema differs")
        stack = np.stack([f[COLUMNS].to_numpy(dtype=float) for f in frames])
        stable = stack[:, 500:, :].mean(axis=1)
        count_means = {
            name: {"mean": float(stable[:, i].mean()), "sample_sd": float(stable[:, i].std(ddof=1))}
            for i, name in enumerate(COLUMNS)
        }
        height_means = []
        for p in height_paths:
            f = pd.read_parquet(p)
            f = f[f.Time >= 500]
            means = f.groupby("Bin").Height.mean()
            if set(means.index) != set(range(10)):
                raise ValueError("Reference height bins differ")
            height_means.append([(float(means[i]) - baseline[i]) * 2 for i in range(10)])
        heights = np.asarray(height_means)
        protocol["groups"][label] = {
            "source_directory": relative,
            "reference_replicates": expected,
            "end_hour": horizon,
            "initial_counts": dict(zip(COLUMNS, stack[0, 0, :].tolist(), strict=True)),
            "reference_files": {
                str(p.relative_to(data)): hashlib.sha256(p.read_bytes()).hexdigest()
                for p in paths + height_paths
            },
            "stable_count_means": count_means,
            "pointwise_count_mean": stack.mean(axis=0).tolist(),
            "pointwise_count_sample_sd": stack.std(axis=0, ddof=1).tolist(),
            "stable_thickness_um": {
                str(i): {
                    "mean": float(heights[:, i].mean()),
                    "sample_sd": float(heights[:, i].std(ddof=1)),
                }
                for i in range(10)
            },
        }
    destination.write_text(json.dumps(protocol, separators=(",", ":"), allow_nan=False) + "\n")
    print(
        "Frozen comparison for",
        sum(g["reference_replicates"] for g in protocol["groups"].values()),
        "published replicates; four source-defined cohorts.",
    )


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--data", required=True, type=Path)
    p.add_argument("--membrane", required=True, type=Path)
    p.add_argument("--output", required=True, type=Path)
    args = p.parse_args()
    freeze(args.data, args.membrane, args.output)
