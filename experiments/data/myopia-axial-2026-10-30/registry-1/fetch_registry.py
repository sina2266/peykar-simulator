"""Fetch the ClinicalTrials.gov records of registry tranche 1 (explicit offline operator).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/registry-1/fetch_registry.py --output <new dir>``.

One query of the ClinicalTrials.gov API v2 (declared in this folder's README): every
page of the search response and every returned study record are saved whole with
SHA-256 receipts. Nothing is parsed beyond the paging token and the NCT numbers, and
nothing is printed but counts.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
import time
import urllib.parse
import urllib.request
from datetime import UTC, datetime
from pathlib import Path

API = "https://clinicaltrials.gov/api/v2/studies"
QUERY = {
    "query.cond": "myopia",
    "query.outc": "axial length",
    "aggFilters": "results:with",
    "countTotal": "true",
    "pageSize": "100",
    "format": "json",
}
USER_AGENT = "PeykarRegistryScreen/1.0 (+https://github.com/sina2266/peykar-simulator)"


def _get(url: str) -> bytes:
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(request, timeout=60) as response:  # noqa: S310 - fixed https host
        return bytes(response.read())


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.output.exists():
        parser.error("--output must not exist")
    raw = args.output / "raw"
    raw.mkdir(parents=True)
    receipts = []
    token = None
    page = 0
    studies = 0
    while True:
        params = dict(QUERY)
        if token:
            params["pageToken"] = token
        url = f"{API}?{urllib.parse.urlencode(params)}"
        body = _get(url)
        path = raw / f"search-{page}.json"
        path.write_bytes(body)
        receipts.append(
            {
                "url": url,
                "file": path.name,
                "sha256": hashlib.sha256(body).hexdigest(),
                "utc": datetime.now(UTC).isoformat(),
            }
        )
        data = json.loads(body)
        for study in data.get("studies", []):
            nct = study["protocolSection"]["identificationModule"]["nctId"]
            text = json.dumps(study, indent=1, sort_keys=True).encode()
            record = raw / f"{nct}.json"
            record.write_bytes(text)
            receipts.append(
                {"file": record.name, "sha256": hashlib.sha256(text).hexdigest(), "from": path.name}
            )
            studies += 1
        token = data.get("nextPageToken")
        page += 1
        if not token:
            break
        time.sleep(0.5)
    summary = {
        "utc": datetime.now(UTC).isoformat(),
        "api": API,
        "query": QUERY,
        "pages": page,
        "studies": studies,
        "scope": "Registry records saved whole; no value parsed or printed.",
    }
    (args.output / "receipts.json").write_text(json.dumps(receipts, indent=1) + "\n")
    (args.output / "summary.json").write_text(json.dumps(summary, indent=1) + "\n")
    print(json.dumps({"pages": page, "studies": studies}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
