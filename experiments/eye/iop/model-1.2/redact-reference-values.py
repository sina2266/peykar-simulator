"""Remove the preoperative reference IOP of non-publishable studies from a public report.

The frozen 1.2 publisher (`scripts/publish_iop_population_1_2.py`) reuses the 1.1
redaction. That redaction predates the `reference_value` field, which the 1.2 harness
writes into every initial-state entry, so the publisher left this one field in the
entries of studies whose licence does not permit republishing derived values. This
script removes it from those entries (the studies marked `values_redacted`) in the
folder's `report-public.json` and changes nothing else. It reads only the public file
and is idempotent. Run from the repository root, once per published folder:

    uv run --frozen python experiments/eye/iop/model-1.2/redact-reference-values.py \
        experiments/eye/iop/model-1.2/sealed-test-2026-10-29
"""

import json
import sys
from pathlib import Path


def main() -> None:
    path = Path(sys.argv[1]) / "report-public.json"
    report = json.loads(path.read_text())
    removed = studies = 0
    for study in report["studies"]:
        if not study.get("values_redacted"):
            continue
        studies += 1
        for entry in study["initial_state"]:
            if "reference_value" in entry:
                del entry["reference_value"]
                removed += 1
    path.write_text(json.dumps(report, indent=1) + "\n")
    print(f"{path}: redacted studies {studies}; reference values removed {removed}")


if __name__ == "__main__":
    main()
