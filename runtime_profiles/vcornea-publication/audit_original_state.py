"""Recover explicit initial-state constraints from checksum-verified original VTKs.

This inverse check conditions on the lambda values in both reviewed sources. It
does not infer an unknown lambda, historical seed or unrecorded dynamic state.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path


def pressure_targets(rows, known_lambda):
    if not math.isfinite(known_lambda) or known_lambda <= 0 or not rows:
        raise ValueError("A known positive source lambda and observed rows are required")
    solutions = None
    for row in rows:
        volume, pressure = row["volume"], row["absolute_pressure"]
        if not all(math.isfinite(v) and v >= 0 for v in [volume, pressure]):
            raise ValueError("Nonfinite/negative original pressure or volume")
        candidates = {
            round(volume - pressure / (2 * known_lambda), 8),
            round(volume + pressure / (2 * known_lambda), 8),
        }
        candidates = {v for v in candidates if v >= 0}
        solutions = candidates if solutions is None else solutions & candidates
    return sorted(solutions)


def audit(source, catalog, piff, protocol, output):
    import numpy as np
    import vtk
    from vtk.util.numpy_support import vtk_to_numpy

    pins = json.loads(catalog.read_bytes())
    members = {m["path"]: m for m in pins["archive_members"]}

    def read(relative):
        path = (source / relative).resolve(strict=True)
        if (
            not path.is_relative_to(source.resolve(strict=True))
            or hashlib.sha256(path.read_bytes()).hexdigest() != members[relative]["sha256"]
        ):
            raise ValueError("Original VTK identity differs")
        reader = vtk.vtkStructuredPointsReader()
        reader.SetFileName(str(path))
        reader.ReadAllFieldsOn()
        reader.Update()
        data = reader.GetOutput()
        if data.GetDimensions() != (200, 90, 1):
            raise ValueError("Original state dimensions differ")
        arrays = {}
        for key in ["CellType", "CellId", "Pressure", "CellLineage", "SLS"]:
            values = vtk_to_numpy(data.GetPointData().GetArray(key)).copy()
            if values.shape != (18000,) or not np.isfinite(values).all():
                raise ValueError("Incomplete original state field")
            arrays[key] = values.reshape(90, 200).T
        return arrays

    expected_piff = "906902426fb315c670f41337a8e91aeef4d32bcea7280f0bf8af9a790509e6c4"
    if hashlib.sha256(piff.read_bytes()).hexdigest() != expected_piff:
        raise ValueError("Initial PIFF differs from the admitted original archive")
    names = {
        "STEM": 1,
        "LIMB": 2,
        "BASAL": 3,
        "WING": 4,
        "SUPER": 5,
        "MEMB": 6,
        "STROMA": 7,
        "WALL": 9,
        "TEAR": 10,
    }
    original_types = np.zeros((200, 90), dtype=int)
    original_ids = np.zeros((200, 90), dtype=int)
    for line in piff.read_text().splitlines():
        fields = line.split()
        if not fields or not fields[0].isdigit():
            continue
        cid, _cluster, kind, x0, x1, y0, y1, _z0, _z1 = fields
        original_types[int(x0) : int(x1) + 1, int(y0) : int(y1) + 1] = names[kind]
        original_ids[int(x0) : int(x1) + 1, int(y0) : int(y1) + 1] = int(cid)
    frozen = json.loads(protocol.read_bytes())
    report = {
        "schema_version": "1.0.0",
        "scope": "Observed constraints for a declared derived historical-source injury candidate",
        "piff_sha256": expected_piff,
        "comparison_protocol_sha256": hashlib.sha256(protocol.read_bytes()).hexdigest(),
        "source": "Original Zenodo16884414 VTK/PIFF; CC-BY-4.0, original authors retained",
        "pressure_formula": "abs(2*known_lambda*(observed_volume-initial_target_volume))",
        "lambda_scope": "Conditioned on reviewed source values, not independently recovered",
        "initial_observations": [],
        "limitations": [
            "Spatial groups differ from the frozen full scalar cohorts.",
            "MCS0 saved geometry follows a sweep; exact pre-sweep geometry/seeds not recovered.",
            "Old HPC mechanisms plus a separately evidenced PIFF form a new derived version.",
            "No independent biological validation or original per-run environment is established.",
        ],
    }
    labels = {1: "Stem", 3: "Basal", 4: "Wing", 5: "Superficial"}
    for group in ["(Mild) 1500_G/Comb0", "(Moderate) 2500_G"]:
        relative = f"Data/{group}/LatticeData/Step_000000.vtk"
        arrays = read(relative)
        kinds, ids = arrays["CellType"], arrays["CellId"]
        same_ids = np.array_equal(np.unique(ids), np.unique(original_ids))
        same_types = same_ids and all(
            np.array_equal(
                np.unique(kinds[ids == cid]), np.unique(original_types[original_ids == cid])
            )
            for cid in np.unique(ids)
        )
        if not same_types:
            raise ValueError(
                "Archived PIFF cell identities/types differ from original observations"
            )
        row = {
            "path": relative,
            "sha256": members[relative]["sha256"],
            "piff_cell_id_sets_and_types_equal": bool(same_types),
            "mcs0_type_pixel_differences": int((kinds != original_types).sum()),
            "mcs0_id_pixel_differences": int((ids != original_ids).sum()),
            "pressure_constraints": {},
            "counts": {},
            "lineage_labels": [],
        }
        for kind, label in labels.items():
            cells = np.unique(ids[kinds == kind])
            row["counts"][label] = len(cells)
            observed = []
            for cid in cells:
                mask = ids == cid
                pressure = np.unique(arrays["Pressure"][mask])
                if len(pressure) != 1:
                    raise ValueError("Cell-level pressure is not constant inside the cell")
                observed.append(
                    {
                        "cell_id": int(cid),
                        "volume": int(mask.sum()),
                        "absolute_pressure": float(pressure[0]),
                    }
                )
                if kind in (1, 3):
                    lineage = np.unique(arrays["CellLineage"][mask])
                    if len(lineage) != 1:
                        raise ValueError("Cell-level lineage differs inside one cell")
                    row["lineage_labels"].append(float(lineage[0]))
            lam = 5 if kind == 5 else 2
            row["pressure_constraints"][label] = {
                "known_source_lambda": lam,
                "cells": observed,
                "common_target_volume_solutions": pressure_targets(observed, lam),
            }
        for case in ["slight-injury", "mild-injury", "moderate-injury"]:
            if row["counts"] != frozen["groups"][case]["initial_counts"]:
                raise ValueError("Initial counts differ from selected scalar cohorts")
        report["initial_observations"].append(row)
    relative = "Data/(Mild) 1500_G/Comb0/LatticeData/Step_003000.vtk"
    pulse = read(relative)["SLS"]
    report["separate_mild_comb0_pulse"] = {
        "path": relative,
        "sha256": members[relative]["sha256"],
        "mcs": 3000,
        "positive_lattice_coordinates_xy": np.argwhere(pulse > 0).tolist(),
        "total_original_units": float(pulse.sum()),
        "scope": "Spatial position evidence; selected Comb1 dose/time stay separately frozen",
    }
    if output.exists():
        raise ValueError("Preserve the prior state audit")
    output.write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(
        "Original pressure/state/chemical-position constraints retained; no candidate pass claimed"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--piff", required=True, type=Path)
    parser.add_argument("--protocol", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    audit(args.source, args.catalog, args.piff, args.protocol, args.output)
