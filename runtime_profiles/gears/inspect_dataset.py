"""Inspect actual AnnData measurements without loading an external model/pickle."""

import argparse
import hashlib
import json
from pathlib import Path

import anndata
import numpy as np
from scipy import sparse


def checksum(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    data = anndata.read_h5ad(args.input, backed="r")
    try:
        if data.n_obs < 2 or data.n_vars < 2 or not data.obs_names.is_unique:
            raise ValueError("AnnData requires unique cells and a nonempty measurement matrix")
        # Bounded materialization. Whole-matrix statistics are not inferred from this sample.
        sample = data.X[: min(256, data.n_obs), : min(512, data.n_vars)]
        values = sample.data if sparse.issparse(sample) else np.asarray(sample)
        if not np.isfinite(values).all():
            raise ValueError("Sampled expression matrix contains nonfinite values")
        conditions = {
            str(name): {str(k): int(v) for k, v in data.obs[name].value_counts().items()}
            for name in ("condition", "cell_type", "perturbation")
            if name in data.obs
        }
        summary = {
            "observations": data.n_obs,
            "features": data.n_vars,
            "obs_columns": list(data.obs.columns),
            "var_columns": list(data.var.columns),
            "conditions": conditions,
            "sampled_matrix_shape": list(sample.shape),
            "sampled_values_finite": True,
            "sampled_minimum": float(values.min()),
            "sampled_maximum": float(values.max()),
            "matrix_scale": "Not inferred; preserve the provider's preprocessing description",
            "limitations": [
                "Only the declared matrix subset was checked for finite values.",
                "No donor independence, original split, or model accuracy is established.",
                "This is measurement ingestion/QC, not a perturbation prediction.",
            ],
        }
        artifact = args.output / "dataset-summary.json"
        artifact.write_text(json.dumps(summary, indent=2) + "\n")
        receipt = {
            "engine": "GEARS",
            "native_execution": "passed",
            "reader": "anndata " + anndata.__version__,
            "scientific_validity": "not_established",
            "input_sha256": checksum(args.input),
            "artifacts": {artifact.name: checksum(artifact)},
        }
        (args.output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
        print(json.dumps(receipt))
    finally:
        data.file.close()


if __name__ == "__main__":
    main()
