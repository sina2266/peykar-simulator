"""Compare original saved cell-level EGF with original summed/mean field values.

This describes source correspondence, never candidate acceptance or parameter fit.
"""

import argparse
import csv
import hashlib
import json
from pathlib import Path

from inspect_original_spatial import read_ascii_fields


def audit(source, catalog, output):
    import numpy as np

    pins = json.loads(catalog.read_bytes())
    snapshots, cells = [], []
    for member in pins["archive_members"]:
        if not member["path"].endswith(".vtk"):
            continue
        path = (source / member["path"]).resolve(strict=True)
        raw = path.read_bytes()
        if (
            not path.is_relative_to(source.resolve(strict=True))
            or hashlib.sha256(raw).hexdigest() != member["sha256"]
        ):
            raise ValueError("Original sensing artifact identity/confinement differs")
        fields, quality = read_ascii_fields(raw, ["CellType", "CellId", "EGF"])
        row = {
            "path": member["path"],
            "sha256": member["sha256"],
            "original_field_completeness": quality,
            "egf_seen_available": any(
                f["name"] == "EGF_Seen" and f["complete"] for f in quality["fields"]
            ),
        }
        if not row["egf_seen_available"]:
            row["positive_epithelial_cells"] = None
            snapshots.append(row)
            continue
        fields, _ = read_ascii_fields(raw, ["CellType", "CellId", "EGF", "EGF_Seen"])
        fields = {key: np.asarray(values) for key, values in fields.items()}
        selected = []
        epithelial_ids = np.unique(fields["CellId"][np.isin(fields["CellType"], [1, 3, 4, 5])])
        for cid in epithelial_ids:
            mask = fields["CellId"] == cid
            seen = np.unique(fields["EGF_Seen"][mask])
            kind = np.unique(fields["CellType"][mask])
            if len(seen) != 1 or len(kind) != 1:
                raise ValueError("Original cell-level EGF/type differs inside one cell")
            total, volume = float(fields["EGF"][mask].sum()), int(mask.sum())
            if seen[0] > 0 and total > 1e-12:
                reported = float(seen[0])
                selected.append((reported, total, total / volume))
                cells.append(
                    {
                        "path": member["path"],
                        "cell_id": int(cid),
                        "type_id": int(kind[0]),
                        "volume_lattice": volume,
                        "reported_egf_seen": reported,
                        "saved_egf_sum": total,
                        "saved_egf_mean": total / volume,
                        "absolute_difference_from_sum": abs(reported - total),
                        "relative_difference_from_sum": abs(reported - total) / total,
                    }
                )
        row["positive_epithelial_cells"] = len(selected)
        if selected:
            values = np.asarray(selected)
            row.update(
                seen_over_saved_sum_quantiles=np.quantile(
                    values[:, 0] / values[:, 1], [0.05, 0.5, 0.95]
                ).tolist(),
                seen_over_saved_mean_quantiles=np.quantile(
                    values[:, 0] / values[:, 2], [0.05, 0.5, 0.95]
                ).tolist(),
                maximum_absolute_difference_from_sum=float(
                    np.abs(values[:, 0] - values[:, 1]).max()
                ),
                maximum_relative_difference_from_sum=float(
                    (np.abs(values[:, 0] - values[:, 1]) / values[:, 1]).max()
                ),
            )
        snapshots.append(row)
    output.mkdir(parents=True, exist_ok=False)
    if cells:
        with (output / "original-egf-by-cell.csv").open("x", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=list(cells[0]))
            writer.writeheader()
            writer.writerows(cells)
    report = {
        "schema_version": "1.0.0",
        "scope": "Descriptive original EGF; no candidate comparison or exact source inference",
        "catalog_sha256": hashlib.sha256(catalog.read_bytes()).hexdigest(),
        "reader_sha256": hashlib.sha256(
            Path(__file__).with_name("inspect_original_spatial.py").read_bytes()
        ).hexdigest(),
        "audit_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "eligibility": "Epithelial cell types1/3/4/5, EGF_Seen>0 and saved sum>1e-12 for division",
        "quantile_probabilities": [0.05, 0.5, 0.95],
        "independent_biological_validation": False,
        "limitations": [
            "Saved per-cell values and fields have their original steppable timing/precision.",
            "These descriptive ratios introduce no candidate acceptance or parameter changes.",
            "Spatial groups differ from the four frozen scalar cohorts.",
            "Unavailable/truncated original fields remain missing; no values are filled.",
        ],
        "snapshots": snapshots,
        "files": {
            p.name: hashlib.sha256(p.read_bytes()).hexdigest()
            for p in output.iterdir()
            if p.is_file()
        },
    }
    (output / "report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(
        "Original EGF sensing audit:",
        len(cells),
        "positive epithelial cells; unavailable snapshots:",
        sum(not r["egf_seen_available"] for r in snapshots),
        "; no candidate acceptance inferred",
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    audit(args.source, args.catalog, args.output)
