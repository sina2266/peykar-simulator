"""Structure screen of the registry tranche-1 records (no value read or printed).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/registry-1/registry_screen.py --raw <fetch dir>/raw
--output <file>``.

Applies rule 2 of this folder's README to every saved ClinicalTrials.gov record:

(a) the eligibility admits only participants younger than 19 years (a posted maximum age
    of at most 18 years);
(b) at least one posted outcome measure whose title names axial length or axial
    elongation and whose unit of measure is millimetres;
(c) the NCT number appears in no source article of the 59 consumed record studies or of
    the 4 screen-2 admits (searched in their screened full texts), and in no other kept
    record.

The output lists, per record, the NCT number, the maximum age, the titles, time frames
and units of the axial-length outcome measures (structure, like table captions), the
studies whose article names the trial, and the decision. No measurement is read.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
RESERVATION = HERE.parent / "screen-2/split-reservation-1.0.0.json"
FULLTEXT = (
    ROOT / ".peykar/myopia-literature-screen-2026-10-30/raw/fulltext",
    ROOT / ".peykar/myopia-literature-screen-2-2026-10-30/raw/fulltext",
)
AXIAL = re.compile(r"axial\s+(length|elongation)", re.I)
MILLIMETRE = re.compile(r"\b(mm|millimet)", re.I)
UNITS = {"year": 1.0, "month": 1.0 / 12.0, "week": 7.0 / 365.25, "day": 1.0 / 365.25}


def age_years(text: str | None) -> float | None:
    if not text:
        return None
    match = re.match(r"^\s*(\d+(?:\.\d+)?)\s*([A-Za-z]+)", text)
    if match is None:
        return None
    unit = match.group(2).lower().rstrip("s")
    return float(match.group(1)) * UNITS[unit] if unit in UNITS else None


def axial_outcomes(study: dict[str, Any]) -> list[dict[str, str]]:
    module = study.get("resultsSection", {}).get("outcomeMeasuresModule", {})
    out = []
    for measure in module.get("outcomeMeasures", []):
        title = measure.get("title", "")
        unit = measure.get("unitOfMeasure", "")
        if AXIAL.search(title) and MILLIMETRE.search(unit):
            out.append({"title": title, "time_frame": measure.get("timeFrame", ""), "unit": unit})
    return out


def article_texts() -> dict[str, str]:
    """The screened full text of every study of the screen-2 reservation."""
    studies = json.loads(RESERVATION.read_text())["studies"]
    texts = {}
    for item in studies:
        pmcid = item["study_key"].removeprefix("myo-").upper()
        path = next((d / f"{pmcid}.xml" for d in FULLTEXT if (d / f"{pmcid}.xml").is_file()), None)
        if path is None:
            raise SystemExit(f"no screened full text for {item['study_key']}")
        texts[item["study_key"]] = path.read_text(errors="replace")
    return texts


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--raw", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    texts = article_texts()
    rows = []
    seen: set[str] = set()
    for path in sorted(args.raw.glob("NCT*.json")):
        study = json.loads(path.read_text())
        protocol = study["protocolSection"]
        nct = protocol["identificationModule"]["nctId"]
        eligibility = protocol.get("eligibilityModule", {})
        maximum = age_years(eligibility.get("maximumAge"))
        outcomes = axial_outcomes(study)
        linked = sorted(k for k, text in texts.items() if nct in text)
        reasons = []
        if maximum is None or maximum >= 19.0:
            reasons.append("eligibility admits participants aged 19 or older (or no maximum age)")
        if not outcomes:
            reasons.append("no posted axial-length outcome measure in millimetres")
        if linked:
            reasons.append("the trial is named in a consumed or screen-2 article")
        if nct in seen:
            reasons.append("duplicate NCT number")
        seen.add(nct)
        rows.append(
            {
                "nct": nct,
                "file": path.name,
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                "brief_title": protocol["identificationModule"].get("briefTitle"),
                "maximum_age": eligibility.get("maximumAge"),
                "axial_outcome_measures": outcomes,
                "named_in_articles_of": linked,
                "decision": "keep" if not reasons else "drop",
                "reasons": reasons,
            }
        )
    output = {
        "schema": "eye.myopia-axial-registry-screen-1.0.0",
        "utc": datetime.now(UTC).isoformat(),
        "rule": "registry-1/README.md rule 2 (a)-(c)",
        "reservation_searched": str(RESERVATION.relative_to(ROOT)),
        "outcome_values_read": False,
        "records": rows,
    }
    args.output.write_text(json.dumps(output, indent=1, ensure_ascii=False) + "\n")
    kept = sum(r["decision"] == "keep" for r in rows)
    print(json.dumps({"records": len(rows), "kept": kept, "dropped": len(rows) - kept}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
