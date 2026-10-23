"""Operator analysis software tests. Synthetic fixtures never validate corneal biology."""

import json
import tempfile
import unittest
from pathlib import Path

import numpy as np
import pandas as pd
from compare import checksum, identity, load_run, run


class AnalysisTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.runtime = {"scope": "synthetic analysis test only"}
        self.columns = ["Superficial", "Wing", "Basal", "Stem"]
        self.plan = {
            "status": "execution_completed",
            "profile_digest": identity(self.runtime),
            "jobs": [],
        }
        group = {
            "end_hour": 2,
            "initial_counts": dict(zip(self.columns, [2, 3, 4, 5], strict=True)),
            "reference_replicates": 2,
            "reference_files": {},
            "stable_count_means": {
                k: {"mean": v, "sample_sd": 1}
                for k, v in zip(self.columns, [2, 3, 4, 5], strict=True)
            },
            "pointwise_count_mean": [[2, 3, 4, 5]] * 3,
            "pointwise_count_sample_sd": [[1] * 4] * 3,
            "stable_thickness_um": {str(i): {"mean": 20, "sample_sd": 1} for i in range(10)},
        }
        self.protocol = {
            "groups": {
                k: group
                for k in ("self-organization", "slight-injury", "mild-injury", "moderate-injury")
            },
            "count_columns": self.columns,
            "stable_from_hour": 0,
            "frozen_before_full_candidate_execution": True,
            "mean_envelope_sd_multiplier": 3,
            "pointwise_sd_multiplier": 3,
            "count_quantization_allowance": 1,
            "minimum_pointwise_coverage": 0.9,
            "membrane_bins_lattice": {str(i): 0 for i in range(10)},
            "thickness_lattice_to_um": 2,
            "thickness_quantization_allowance_um": 2,
            "limitations": ["Synthetic fixtures exercise analysis only"],
        }
        (self.root / "comparison.json").write_text(json.dumps(self.protocol))
        self.protocol_digest = checksum(self.root / "comparison.json")
        self.plan["protocol_digest"] = self.protocol_digest
        for case in self.protocol["groups"]:
            for seed in [7, 11]:
                root = self.root / f"{case}-{seed}"
                project = root / "project"
                project.mkdir(parents=True)
                (project / "Parameters.py").write_text("# Synthetic fixture; never executed\n")
                (root / "driver.py").write_text("# Synthetic fixture; never executed\n")
                manifest = {
                    "publication_recipe": case,
                    "controls": {"seed": seed, "steps_mcs": 20},
                    "execution_files": {"Parameters.py": checksum(project / "Parameters.py")},
                    "driver_digest": checksum(root / "driver.py"),
                    "comparison_protocol_digest": self.protocol_digest,
                }
                (root / "manifest.json").write_text(json.dumps(manifest))
                (root / "runtime-profile.json").write_text(json.dumps(self.runtime))
                pd.DataFrame(
                    [[2, 3, 4, 5, t] for t in range(3)], columns=self.columns + ["Time"]
                ).to_csv(project / "cell_count_21.csv", index=False)
                pd.DataFrame(
                    [(t, i, 10.0) for t in range(3) for i in range(10)],
                    columns=["Time", "Bin", "Height"],
                ).to_parquet(project / "thickness_rep_21.parquet", index=False)
                record = {
                    "status": "succeeded",
                    "returncode": 0,
                    "elapsed_seconds": 1,
                    "manifest_digest": identity(manifest),
                    "runtime_digest": identity(self.runtime),
                    "outputs": {
                        p.relative_to(root).as_posix(): checksum(p)
                        for p in project.iterdir()
                        if p.suffix in (".csv", ".parquet")
                    },
                }
                (root / "record.json").write_text(json.dumps(record))
                self.plan["jobs"].append(
                    {
                        "case": case,
                        "seed": seed,
                        "directory": str(root),
                        "manifest_digest": identity(manifest),
                    }
                )
        (self.root / "plan.json").write_text(json.dumps(self.plan))

    def test_complete_analysis_retains_scope_and_each_run(self):
        result = run(self.root, self.root, self.root / "report")
        self.assertEqual(result["status"], "passed")
        self.assertEqual(len(result["jobs"]), 8)
        self.assertFalse(result["independent_biological_validation"])

    def test_incomplete_grid_is_rejected_even_when_artifact_digest_is_updated(self):
        job = self.plan["jobs"][0]
        root = Path(job["directory"])
        path = root / "project/cell_count_21.csv"
        pd.read_csv(path).iloc[:-1].to_csv(path, index=False)
        record = json.loads((root / "record.json").read_bytes())
        record["outputs"]["project/cell_count_21.csv"] = checksum(path)
        (root / "record.json").write_text(json.dumps(record))
        with self.assertRaisesRegex(ValueError, "full horizon"):
            load_run(self.plan, job, self.protocol_digest, self.protocol)

    def test_negative_or_nonfinite_height_and_changed_source_cannot_pass(self):
        job = self.plan["jobs"][0]
        root = Path(job["directory"])
        path = root / "project/thickness_rep_21.parquet"
        frame = pd.read_parquet(path)
        frame.loc[0, "Height"] = np.nan
        frame.to_parquet(path, index=False)
        record = json.loads((root / "record.json").read_bytes())
        record["outputs"]["project/thickness_rep_21.parquet"] = checksum(path)
        (root / "record.json").write_text(json.dumps(record))
        with self.assertRaisesRegex(ValueError, "height"):
            load_run(self.plan, job, self.protocol_digest, self.protocol)
        (root / "project/Parameters.py").write_text("# Changed source\n")
        with self.assertRaisesRegex(ValueError, "identity/confinement"):
            load_run(self.plan, job, self.protocol_digest, self.protocol)

    def test_actual_pulse_schema_time_and_amount_are_checked(self):
        job = self.plan["jobs"][0]
        root = Path(job["directory"])
        manifest = json.loads((root / "manifest.json").read_bytes())
        manifest["parameter_overrides"] = {"InjuryTime": 10, "SLS_Concentration": 750}
        (root / "manifest.json").write_text(json.dumps(manifest))
        job["manifest_digest"] = identity(manifest)
        record = json.loads((root / "record.json").read_bytes())
        record["manifest_digest"] = identity(manifest)
        path = root / "project/surfactant_21.csv"
        frame = pd.DataFrame({"Total Amount": [0] * 10 + [750] * 11, "Time": np.arange(21) / 10})
        frame.to_csv(path, index=False)
        record["outputs"]["project/surfactant_21.csv"] = checksum(path)
        (root / "record.json").write_text(json.dumps(record))
        result = load_run(self.plan, job, self.protocol_digest, self.protocol)
        self.assertEqual(result[-1], {"first_positive_hour": 1, "first_positive_total": 750})
        frame.loc[10, "Total Amount"] = 749
        frame.to_csv(path, index=False)
        record["outputs"]["project/surfactant_21.csv"] = checksum(path)
        (root / "record.json").write_text(json.dumps(record))
        with self.assertRaisesRegex(ValueError, "pulse amount"):
            load_run(self.plan, job, self.protocol_digest, self.protocol)


if __name__ == "__main__":
    unittest.main()
