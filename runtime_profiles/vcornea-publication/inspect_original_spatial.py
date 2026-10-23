"""Inspect retained original VTK observations without asserting cohort correspondence.

Run explicitly in the native scientific environment. These original spatial groups
are distinct from the four scalar cohorts in the frozen reproduction protocol.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path


def read_ascii_fields(raw, required, expected_dimensions=(200, 90, 1)):
    """Read this archive's scalar FIELD layout without trusting allocated VTK arrays.

    A truncated optional field is retained as a source-quality gap. Required
    fields must contain every actual finite value; no missing value is filled.
    """
    lines = raw.decode("ascii").splitlines()
    if (
        len(lines) < 10
        or not lines[0].startswith("# vtk DataFile Version ")
        or lines[2].strip() != "ASCII"
        or lines[3].strip() != "DATASET STRUCTURED_POINTS"
    ):
        raise ValueError("Unsupported original VTK layout")
    dimensions = lines[4].split()
    if dimensions[0] != "DIMENSIONS" or tuple(map(int, dimensions[1:])) != expected_dimensions:
        raise ValueError("Original VTK dimensions differ")
    points = math.prod(expected_dimensions)
    if lines[7].split() != ["POINT_DATA", str(points)]:
        raise ValueError("Original VTK point count differs")
    declaration = lines[8].split()
    if len(declaration) != 3 or declaration[:2] != ["FIELD", "FieldData"]:
        raise ValueError("Original VTK FIELD declaration differs")
    declared = int(declaration[2])
    if not 1 <= declared <= 64:
        raise ValueError("Original VTK field count is outside the reviewed bound")
    tokens = " ".join(lines[9:]).split()
    cursor = 0
    arrays, fields = {}, []
    names = set()
    for _ in range(declared):
        if cursor == len(tokens):
            break
        if len(tokens) - cursor < 4:
            raise ValueError("Incomplete original VTK field header")
        name, components, tuples, kind = tokens[cursor : cursor + 4]
        cursor += 4
        if (
            name in names
            or components != "1"
            or tuples != str(points)
            or kind not in {"char", "long", "double", "float", "int"}
        ):
            raise ValueError("Unsupported or duplicate original VTK scalar field")
        names.add(name)
        observed = min(points, len(tokens) - cursor)
        try:
            values = [float(v) for v in tokens[cursor : cursor + observed]]
        except ValueError as error:
            raise ValueError("Nonnumeric original VTK field: " + name) from error
        cursor += observed
        finite = all(math.isfinite(v) for v in values)
        complete = observed == points and finite
        fields.append(
            {
                "name": name,
                "expected_values": points,
                "observed_values": observed,
                "finite": finite,
                "complete": complete,
            }
        )
        if name in required:
            if not complete:
                raise ValueError("Required original VTK field is incomplete/nonfinite: " + name)
            arrays[name] = values
        if observed != points:
            break
    if cursor != len(tokens):
        raise ValueError("Unexpected trailing original VTK data")
    if set(arrays) != set(required):
        raise ValueError("Required original VTK fields are missing")
    quality = {
        "reader": "checked_original_ascii_scalar_fields-1.0.0",
        "declared_field_count": declared,
        "discovered_field_count": len(fields),
        "undiscovered_field_count": declared - len(fields),
        "all_declared_fields_complete": len(fields) == declared
        and all(f["complete"] for f in fields),
        "required_fields_complete": True,
        "fields": fields,
    }
    return arrays, quality


def inspect(source, catalog, output):
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np
    from analyze_spatial import COLORS
    from matplotlib.colors import BoundaryNorm, ListedColormap

    pins = json.loads(catalog.read_bytes())
    loaded = []
    for member in pins["archive_members"]:
        path = (source / member["path"]).resolve(strict=True)
        if (
            not path.is_relative_to(source.resolve(strict=True))
            or hashlib.sha256(path.read_bytes()).hexdigest() != member["sha256"]
        ):
            raise ValueError("Original spatial artifact identity/confinement differs")
        if path.suffix != ".vtk":
            continue
        values, quality = read_ascii_fields(
            path.read_bytes(), ["CellType", "CellId", "ClusterId", "EGF", "SLS"]
        )
        arrays = {}
        for key in ["CellType", "CellId", "ClusterId", "EGF", "SLS"]:
            # VTK flattened order is x-fastest. Export explicitly as x,y.
            arrays[key] = np.asarray(values[key]).reshape((90, 200)).T
        if not np.isin(arrays["CellType"], range(11)).all():
            raise ValueError("Unknown original cell type")
        kinds = arrays["CellType"].astype(int)
        ids = arrays["CellId"]
        for cell_id in np.unique(ids):
            if len(np.unique(kinds[ids == cell_id])) != 1:
                raise ValueError("Original cell identity maps to multiple types")
        mcs = int(path.stem.split("_")[-1])
        label = member["path"].removeprefix("Data/").split("/LatticeData/")[0]
        loaded.append((member, label, mcs, arrays, quality))
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema_version": "1.0.0",
        "scope": "Descriptive original simulation observations; no candidate acceptance",
        "catalog_sha256": hashlib.sha256(catalog.read_bytes()).hexdigest(),
        "coordinate_mapping": "Original 200x90 lattice; paper2.2 assigns 2 micrometres/site",
        "cohort_correspondence": pins["cohort_correspondence"],
        "independent_biological_validation": False,
        "snapshots": [],
    }
    for ordinal, (member, label, mcs, arrays, quality) in enumerate(loaded):
        kinds = arrays["CellType"].astype(int)
        counts = {
            str(kind): len(np.unique(arrays["CellId"][kinds == kind]))
            for kind in np.unique(kinds)
            if kind != 0
        }
        report["snapshots"].append(
            {
                "path": member["path"],
                "sha256": member["sha256"],
                "mcs": mcs,
                "time_hours": mcs / 10,
                "original_field_completeness": quality,
                "cell_counts_by_original_type_id": counts,
                "lattice_sites_by_type_id": {
                    str(kind): int((kinds == kind).sum()) for kind in np.unique(kinds)
                },
                "egf_range_original_units": [
                    float(arrays["EGF"].min()),
                    float(arrays["EGF"].max()),
                ],
                "sls_total_original_units": float(arrays["SLS"].sum()),
            }
        )
        figure, axes = plt.subplots(1, 3, figsize=(14, 3.5), sharey=True)
        extent = [0, 400, 0, 180]
        axes[0].imshow(
            kinds.T,
            origin="lower",
            extent=extent,
            cmap=ListedColormap(COLORS),
            norm=BoundaryNorm(np.arange(12) - 0.5, 11),
            interpolation="nearest",
        )
        axes[0].set_title("Original cell types")
        for ax, key in zip(axes[1:], ["EGF", "SLS"], strict=True):
            view = ax.imshow(
                arrays[key].T,
                origin="lower",
                extent=extent,
                cmap="magma",
                interpolation="nearest",
            )
            ax.set_title(key + " (original model units)")
            figure.colorbar(view, ax=ax, shrink=0.7)
        for ax in axes:
            ax.set_xlabel("x (µm)")
        axes[0].set_ylabel("y (µm)")
        figure.suptitle(f"{label} · {mcs / 10:g} hours · published simulation observation")
        figure.tight_layout()
        figure.savefig(output / f"original-{ordinal:02d}-{mcs}.png", dpi=150)
        plt.close(figure)
    report["files"] = {
        p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in output.iterdir() if p.is_file()
    }
    report["snapshots_with_incomplete_optional_fields"] = sum(
        not row["original_field_completeness"]["all_declared_fields_complete"]
        for row in report["snapshots"]
    )
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(
        "Inspected",
        len(loaded),
        "original VTK snapshots; incomplete optional-field snapshots:",
        report["snapshots_with_incomplete_optional_fields"],
        "; no scalar-cohort match inferred",
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    inspect(args.source, args.catalog, args.output)
