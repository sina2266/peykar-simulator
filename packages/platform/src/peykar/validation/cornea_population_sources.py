"""Assemble population-model series from consumed and newly admitted sources.

The three earlier consumed studies keep their reviewed adapters; new studies come
from validated study records. Role rules are declared here once: vehicle, saline,
PBS and prophylactic antibiotic-only care count as control; any steroid/NSAID or
other active treatment, genotype change or disease condition counts as treated.
"""

from __future__ import annotations

import json
from collections.abc import Sequence
from pathlib import Path
from typing import Any

from peykar.model_learning.corneal_population import WoundSeries
from peykar.validation.cornea_online_evidence import corneal_online_series
from peykar.validation.cornea_population_data import to_wound_series, validate_study

RABBIT_CONTEXT = "rabbit/epithelial_debridement"
# 2019 "Control" eyes received dexamethasone/chloramphenicol and diclofenac (steroid +
# NSAID), so under the declared rule no 2019 group is an untreated control.
CONSUMED_CONTROL_GROUPS = {
    "abdel-naby2017": {"Control"},
    "suarez-barrio2019": set(),
    "chung2021": {"Blade", "Ocu", "Amo"},
}


def consumed_series(*, rabbit2017: bytes, ijms2019: bytes, chung2021: bytes) -> list[WoundSeries]:
    """The 16 previously consumed rabbit series as population-model inputs."""
    evidence = corneal_online_series(rabbit2017=rabbit2017, ijms2019=ijms2019, chung2021=chung2021)
    result = []
    for item in evidence["series"]:
        study = item["series_id"].split("/")[0]
        observed = [
            row
            for row in item["numeric_observations"]
            if row["eligible_for_conditioning"] and row["hour"] > 0
        ]
        aggregate = None if item["series_nature"] == "individual_rabbit" else 0
        result.append(
            WoundSeries(
                series_id=item["series_id"],
                study_id=study,
                context=RABBIT_CONTEXT,
                radius0_um=float(item["baseline_radius_um"]),
                hours=tuple(float(row["hour"]) for row in observed),
                fractions=tuple(float(row["area_fraction"]) for row in observed),
                aggregate_n=aggregate,
                role="control" if item["group"] in CONSUMED_CONTROL_GROUPS[study] else "treated",
                group=item["group"],
                baseline_observed=study != "chung2021",
                metadata={"source": "consumed", "doi": item["study_doi"]},
            )
        )
    return result


def record_series(paths: Sequence[Path], *, split: str) -> tuple[list[WoundSeries], list[str]]:
    """Validated record files of one declared split; returns series and file SHA-256s."""
    from hashlib import sha256

    series: list[WoundSeries] = []
    hashes = []
    for path in sorted(paths):
        raw = path.read_bytes()
        record: dict[str, Any] = json.loads(raw)
        checked = validate_study(record)
        if checked["split"] != split:
            raise ValueError(f"{path.name} is declared {checked['split']}, not {split}")
        series.extend(to_wound_series(record))
        hashes.append(f"{path.name}:{sha256(raw).hexdigest()}")
    return series, hashes


def default_consumed(root: Path) -> list[WoundSeries]:
    experiments = root / "experiments"
    return consumed_series(
        rabbit2017=(
            experiments / "data/cornea-evidence-20261024/raw/rabbit-figure2.xlsx"
        ).read_bytes(),
        ijms2019=(
            experiments / "vcornea/repair-front/conditioned-forecast/ijms2019-original.xml"
        ).read_bytes(),
        chung2021=(experiments / "vcornea/repair-front/chung2021-manuscript.xml").read_bytes(),
    )
