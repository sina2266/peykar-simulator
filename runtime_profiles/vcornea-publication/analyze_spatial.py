"""Read actual lattice snapshots; report explicit 2D structural proxies and EGF fields."""

import argparse
import json
from collections import deque
from pathlib import Path

COLORS = [
    "#101015",
    "#ff007f",
    "#ff7faf",
    "#ffbe99",
    "#0055ff",
    "#55ffff",
    "#ff4dce",
    "#c681ff",
    "#9d9d9d",
    "#ffffff",
    "#55ff00",
]
LABELS = {1: "Stem", 3: "Basal", 4: "Wing", 5: "Superficial"}


def membrane_surface(initial):
    result = {}
    for x, column in enumerate(initial):
        ys = [y for y, kind in enumerate(column) if kind in (2, 6)]
        if ys and max(ys) + 1 < len(column):
            result[x] = max(ys) + 1
    return result


def exposed_columns(kinds, baseline, um_per_site=2):
    """4-connected tear/medium access from the top to the ORIGINAL basal surface.

    This is an explicit local cross-sectional proxy, not whole-eye defect area,
    fluorescein staining, permeability, tight-junction integrity or clinical healing.
    """
    nx, ny = len(kinds), len(kinds[0])
    queue = deque((x, ny - 1) for x in range(nx) if kinds[x][ny - 1] in (0, 10))
    reached = set(queue)
    while queue:
        x, y = queue.popleft()
        for i, j in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= i < nx and 0 <= j < ny and (i, j) not in reached and kinds[i][j] in (0, 10):
                reached.add((i, j))
                queue.append((i, j))
    exposed = sorted(x for x, y in baseline.items() if (x, y) in reached)
    return {
        "eligible_basement_columns": len(baseline),
        "exposed_columns": exposed,
        "exposed_cross_section_width_um": len(exposed) * um_per_site,
        "exposed_fraction": len(exposed) / len(baseline) if baseline else None,
        "observation": "descriptive_top_connected_fluid_access_to_original_basal_surface",
        "clinical_wound_closure": False,
    }


def analyze(directory, output):
    import matplotlib
    from compare import checksum, load_run

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np
    import pandas as pd
    from matplotlib.colors import BoundaryNorm, ListedColormap
    from matplotlib.patches import Patch

    plan = json.loads((directory / "plan.json").read_bytes())
    protocol_digest = checksum(directory / "comparison.json")
    protocol = json.loads((directory / "comparison.json").read_bytes())
    if plan["status"] != "execution_completed" or plan["protocol_digest"] != protocol_digest:
        raise ValueError("Complete the original frozen series before structural analysis")
    loaded = [(job, load_run(plan, job, protocol_digest, protocol)) for job in plan["jobs"]]
    output.mkdir(parents=True, exist_ok=False)
    report = {
        "schema_version": "1.0.0",
        "plan_digest": checksum(directory / "plan.json"),
        "observation_version": "vcornea.spatial-diagnostics-1.0.0",
        "spatial_mapping": "x,y lattice order; 200x90 sites; 2 micrometres/site (paper2.2)",
        "scope": "Descriptive endpoints, not preregistered acceptance or clinical validation",
        "independent_biological_validation": False,
        "jobs": [],
    }
    for job, (root, manifest, record, counts, _height, _pulse) in loaded:
        snapshots = {}
        rows = []
        baseline = None
        for path in sorted(root.glob("lattice-*.npz"), key=lambda p: int(p.stem.split("-")[-1])):
            mcs = int(path.stem.split("-")[-1])
            if record["outputs"].get(path.name) != checksum(path):
                raise ValueError("Lattice snapshot was not admitted by native execution")
            with np.load(path, allow_pickle=False) as values:
                if set(values.files) != {"cell_type", "egf", "sls"}:
                    raise ValueError("Unexpected spatial fields")
                kinds, egf, sls = (values[key].copy() for key in ("cell_type", "egf", "sls"))
            if (
                kinds.shape != (200, 90)
                or egf.shape != kinds.shape
                or sls.shape != kinds.shape
                or not np.isfinite(egf).all()
                or not np.isfinite(sls).all()
                or not np.isin(kinds, range(11)).all()
            ):
                raise ValueError(
                    "Spatial coordinates, cell-type identities or finite fields differ"
                )
            if mcs == 0:
                baseline = membrane_surface(kinds.tolist())
            if not baseline:
                raise ValueError("Original basement surface snapshot is missing")
            cells_path = root / f"cells-{mcs}.csv"
            if record["outputs"].get(cells_path.name) != checksum(cells_path):
                raise ValueError("Cell snapshot was not admitted by native execution")
            cells = pd.read_csv(cells_path)
            count = counts[counts.Time == mcs / 10].iloc[0]
            for kind, label in LABELS.items():
                if len(cells[cells.TypeID == kind]) != count[label]:
                    raise ValueError("Independent observer and original count tracker disagree")
            for kind in cells.TypeID.unique():
                if (kinds == kind).sum() != cells[cells.TypeID == kind].Volume_lattice.sum():
                    raise ValueError("Cell volumes and actual lattice occupancy disagree")
            row = {
                "mcs": mcs,
                "hour": mcs / 10,
                "epithelial_lattice_sites": int(np.isin(kinds, list(LABELS)).sum()),
                "basement_lattice_sites": int(np.isin(kinds, [2, 6]).sum()),
                "epithelial_cell_counts": {label: int(count[label]) for label in LABELS.values()},
                "median_cell_y_um": {
                    label: None
                    if (selected := cells[cells.TypeID == kind]).empty
                    else float(selected.Y_lattice.median() * 2)
                    for kind, label in LABELS.items()
                },
                "egf_minimum": float(egf.min()),
                "egf_maximum": float(egf.max()),
                "sls_lattice_integral": float(sls.sum()),
                "basal_exposure": exposed_columns(kinds.tolist(), baseline),
            }
            rows.append(row)
            snapshots[mcs] = kinds, egf
        injury = manifest.get("parameter_overrides", {}).get("InjuryTime")
        times = (
            [0, 240, 720, 1680, 3600, manifest["controls"]["steps_mcs"]]
            if injury is None
            else [injury + t for t in [0, 240, 720, 1200, 1680, 4800]]
        )
        if any(t not in snapshots for t in times):
            raise ValueError("Publication formation/recovery snapshots are incomplete")
        figure, axes = plt.subplots(2, 3, figsize=(14, 7), sharex=True, sharey=True)
        cmap = ListedColormap(COLORS)
        norm = BoundaryNorm(np.arange(-0.5, 11.5), len(COLORS))
        for t, ax in zip(times, axes.flat, strict=True):
            ax.imshow(
                snapshots[t][0].T,
                origin="lower",
                cmap=cmap,
                norm=norm,
                extent=(0, 400, 0, 180),
                interpolation="nearest",
            )
            day = (t - (injury or 0)) / 240
            ax.set_title(f"Day {day:g}" + (" after pulse" if injury else ""))
            ax.set_xlabel("Radial position (µm)")
            ax.set_ylabel("Depth position (µm)")
        figure.suptitle(f"{job['case']} · seed {job['seed']} · actual 2D lattice")
        figure.legend(
            handles=[Patch(color=COLORS[k], label=label) for k, label in LABELS.items()],
            loc="lower center",
            ncols=4,
        )
        figure.tight_layout(rect=(0, 0.05, 1, 0.96))
        figure.savefig(output / f"{job['case']}-{job['seed']}-structure.png", dpi=160)
        plt.close(figure)
        if injury is not None:
            figure, axes = plt.subplots(1, 3, figsize=(14, 4), sharey=True)
            for t, ax in zip([injury, injury + 10, injury + 4800], axes, strict=True):
                if t not in snapshots:
                    raise ValueError("Original field comparison times are incomplete")
                field = snapshots[t][1]
                image = ax.imshow(
                    field.T,
                    origin="lower",
                    extent=(0, 400, 0, 180),
                    vmin=0,
                    vmax=1,
                    cmap="turbo",
                    interpolation="nearest",
                )
                ax.set_title(f"{(t - injury) / 10:g} hours after pulse")
                ax.set_xlabel("Radial position (µm)")
            axes[0].set_ylabel("Depth position (µm)")
            figure.colorbar(image, ax=axes, label="Original EGF field (model scale0–1)", shrink=0.8)
            figure.suptitle(f"{job['case']} · seed {job['seed']} · actual EGF field")
            figure.savefig(
                output / f"{job['case']}-{job['seed']}-egf.png", dpi=160, bbox_inches="tight"
            )
            plt.close(figure)
        report["jobs"].append(
            {
                "case": job["case"],
                "seed": job["seed"],
                "manifest_digest": job["manifest_digest"],
                "snapshots": rows,
            }
        )
    report["files"] = {p.name: checksum(p) for p in output.iterdir() if p.is_file()}
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print("Actual lattice/cell consistency and spatial diagnostics:", len(report["jobs"]), "runs")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runs", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    analyze(args.runs, args.output)
