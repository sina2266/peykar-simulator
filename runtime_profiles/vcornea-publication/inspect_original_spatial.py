"""Inspect retained original VTK observations without asserting cohort correspondence.

Run explicitly in the native scientific environment. These original spatial groups
are distinct from the four scalar cohorts in the frozen reproduction protocol.
"""

import argparse
import hashlib
import json
from pathlib import Path


def inspect(source, catalog, output):
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np
    import vtk
    from analyze_spatial import COLORS
    from matplotlib.colors import BoundaryNorm, ListedColormap
    from vtk.util.numpy_support import vtk_to_numpy

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
        reader = vtk.vtkStructuredPointsReader()
        reader.SetFileName(str(path))
        reader.ReadAllScalarsOn()
        reader.ReadAllFieldsOn()
        reader.Update()
        data = reader.GetOutput()
        if data.GetDimensions() != (200, 90, 1) or data.GetNumberOfPoints() != 18000:
            raise ValueError("Original VTK dimensions differ")
        arrays = {}
        for key in ["CellType", "CellId", "ClusterId", "EGF", "SLS"]:
            array = data.GetPointData().GetArray(key)
            if array is None:
                raise ValueError("Original VTK field is missing: " + key)
            values = vtk_to_numpy(array).copy()
            if values.shape != (18000,) or not np.isfinite(values).all():
                raise ValueError("Original VTK field is incomplete/nonfinite")
            # VTK flattened order is x-fastest. Export explicitly as x,y.
            arrays[key] = values.reshape((90, 200)).T
        if not np.isin(arrays["CellType"], range(11)).all():
            raise ValueError("Unknown original cell type")
        kinds = arrays["CellType"].astype(int)
        ids = arrays["CellId"]
        for cell_id in np.unique(ids):
            if len(np.unique(kinds[ids == cell_id])) != 1:
                raise ValueError("Original cell identity maps to multiple types")
        mcs = int(path.stem.split("_")[-1])
        label = member["path"].removeprefix("Data/").split("/LatticeData/")[0]
        loaded.append((member, label, mcs, arrays))
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
    for ordinal, (member, label, mcs, arrays) in enumerate(loaded):
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
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print("Inspected", len(loaded), "original VTK snapshots; no scalar-cohort match inferred")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    inspect(args.source, args.catalog, args.output)
