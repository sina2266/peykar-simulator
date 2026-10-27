"""Acquire data-like supplementary files of screened corneal wound studies.

Explicit offline operator, second screening stage. For each candidate selected
from ``screen_corneal_wound_literature.py`` metadata it downloads the Europe PMC
supplementary archive (inline images excluded), records the archive SHA-256 and
member inventory, and keeps only spreadsheet/text data members. Other members
(PDF/figures/video) are not stored. A *masked* preview of every spreadsheet shows
text labels with all numbers hidden, so studies can be triaged by structure
before any outcome value is read. No fitting, scoring or extraction happens here.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import re
import sys
import time
import urllib.error
import urllib.request
import zipfile
from concurrent.futures import ThreadPoolExecutor
from datetime import UTC, datetime
from pathlib import Path, PurePosixPath
from typing import Any

from peykar.data_fabric.spreadsheet import masked_preview, read_csv, read_xlsx

SUPPLEMENTS = (
    "https://www.ebi.ac.uk/europepmc/webservices/rest/{pmcid}/supplementaryFiles"
    "?includeInlineImage=false"
)
USER_AGENT = "PeykarLiteratureScreen/1.0 (+https://github.com/sina2266/Peykar-Simulator)"
KEEP = (".xlsx", ".csv", ".tsv", ".txt", ".xls")
MAX_ARCHIVE = 250_000_000
MAX_MEMBER = 40_000_000
KEYWORDS = re.compile(
    r"wound|defect|fluoresc|area|heal|closure|re-?epith|epitheli|debrid|\bh\b|hour|hr|day|time",
    re.I,
)


def candidate(row: dict[str, Any]) -> bool:
    """Metadata-only candidate rule, declared before any supplement is read."""
    if row.get("parse_error") or not row.get("species"):
        return False
    if not row.get("wound_time_figures"):
        return False
    return any(
        any(href.lower().endswith(KEEP + (".zip",)) for href in s.get("hrefs", []))
        for s in row.get("supplements", [])
    )


def _download(pmcid: str) -> tuple[int, bytes]:
    url = SUPPLEMENTS.format(pmcid=pmcid)
    for attempt in range(3):
        try:
            request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
            with urllib.request.urlopen(request, timeout=240) as response:
                body = response.read(MAX_ARCHIVE + 1)
                return int(response.status), body
        except urllib.error.HTTPError as error:
            return int(error.code), b""
        except (urllib.error.URLError, TimeoutError, ConnectionError):
            time.sleep(5.0 * (attempt + 1))
    return 0, b""


def _members(raw: bytes) -> list[tuple[str, bytes]]:
    kept = []
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        for info in archive.infolist():
            name = PurePosixPath(info.filename)
            if info.is_dir() or ".." in name.parts or name.is_absolute():
                continue
            lower = name.name.lower()
            if lower.endswith(".zip") and info.file_size <= MAX_MEMBER:
                try:
                    inner = zipfile.ZipFile(io.BytesIO(archive.read(info)))
                except zipfile.BadZipFile:
                    continue
                with inner:
                    for nested in inner.infolist():
                        nested_name = PurePosixPath(nested.filename)
                        if (
                            not nested.is_dir()
                            and nested_name.name.lower().endswith(KEEP)
                            and nested.file_size <= MAX_MEMBER
                            and ".." not in nested_name.parts
                        ):
                            kept.append((f"{name.name}!/{nested_name.name}", inner.read(nested)))
            elif lower.endswith(KEEP) and info.file_size <= MAX_MEMBER:
                kept.append((name.name, archive.read(info)))
    return kept


def _inventory(raw: bytes) -> list[dict[str, Any]]:
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        return [{"name": i.filename, "bytes": i.file_size} for i in archive.infolist()]


def _preview(name: str, data: bytes) -> dict[str, Any]:
    lower = name.lower()
    try:
        if lower.endswith(".xlsx"):
            sheets = read_xlsx(data)
        elif lower.endswith((".csv", ".tsv", ".txt")):
            sheets = read_csv(data)
        else:
            return {"unsupported_format": lower.rsplit(".", 1)[-1]}
    except Exception as error:  # noqa: BLE001 - record any reader failure verbatim
        return {"reader_error": f"{type(error).__name__}: {error}"[:300]}
    preview = masked_preview(sheets)
    labels = " ".join(" ".join(sheet["rows"]) for sheet in preview.values())
    return {
        "sheets": preview,
        "keyword_hits": sorted({m.group(0).lower() for m in KEYWORDS.finditer(labels)}),
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--screen", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--workers", type=int, default=4)
    parser.add_argument(
        "--pmcids",
        type=Path,
        help="Optional JSON list of PMCIDs selected by a second, declared metadata rule",
    )
    args = parser.parse_args(argv)
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("--output must be a new or empty directory")
    rows = json.loads(args.screen.read_text())
    if args.pmcids:
        wanted = set(json.loads(args.pmcids.read_text())["pmcids"])
        selected = [row for row in rows if row["pmcid"] in wanted]
    else:
        selected = [row for row in rows if candidate(row)]
    (args.output / "files").mkdir(parents=True)
    (args.output / "previews").mkdir()

    def acquire(row: dict[str, Any]) -> dict[str, Any]:
        pmcid = row["pmcid"]
        status, raw = _download(pmcid)
        receipt: dict[str, Any] = {
            "pmcid": pmcid,
            "doi": row.get("doi"),
            "title": row.get("title"),
            "journal": row.get("journal"),
            "license": row.get("license"),
            "species": row.get("species"),
            "methods": row.get("methods"),
            "candidate_wound_diameters_mm": row.get("candidate_wound_diameters_mm"),
            "url": SUPPLEMENTS.format(pmcid=pmcid),
            "utc": datetime.now(UTC).isoformat(),
            "status": status,
            "archive_bytes": len(raw),
            "archive_sha256": hashlib.sha256(raw).hexdigest() if raw else None,
        }
        if status != 200 or not raw or len(raw) > MAX_ARCHIVE:
            receipt["outcome"] = "archive_unavailable_or_oversized"
            return receipt
        try:
            receipt["archive_inventory"] = _inventory(raw)
            members = _members(raw)
        except zipfile.BadZipFile:
            receipt["outcome"] = "archive_not_zip"
            return receipt
        folder = args.output / "files" / pmcid
        kept = []
        previews = {}
        for name, data in members:
            safe = re.sub(r"[^A-Za-z0-9._!-]", "_", name.replace("!/", "__"))
            folder.mkdir(exist_ok=True)
            (folder / safe).write_bytes(data)
            kept.append(
                {
                    "member": name,
                    "stored_as": safe,
                    "bytes": len(data),
                    "sha256": hashlib.sha256(data).hexdigest(),
                }
            )
            previews[name] = _preview(name, data)
        (args.output / "previews" / f"{pmcid}.json").write_text(
            json.dumps({"pmcid": pmcid, "title": row.get("title"), "previews": previews}, indent=1)
            + "\n"
        )
        receipt["kept_members"] = kept
        receipt["outcome"] = "data_members_kept" if kept else "no_data_members"
        return receipt

    receipts = []
    with ThreadPoolExecutor(max_workers=max(1, min(args.workers, 6))) as pool:
        for index, receipt in enumerate(pool.map(acquire, selected)):
            receipts.append(receipt)
            print(
                f"{index + 1}/{len(selected)} {receipt['pmcid']} {receipt['outcome']}", flush=True
            )
    (args.output / "receipts.json").write_text(json.dumps(receipts, indent=1) + "\n")
    summary = {
        "utc": datetime.now(UTC).isoformat(),
        "screen": str(args.screen),
        "screen_sha256": hashlib.sha256(args.screen.read_bytes()).hexdigest(),
        "candidates": len(selected),
        "with_data_members": sum(r.get("outcome") == "data_members_kept" for r in receipts),
        "candidate_rule": (
            f"explicit list {args.pmcids}"
            if args.pmcids
            else "species keyword + fluorescein/wound time figure + spreadsheet-like SI"
        ),
        "scope": "Masked structural previews only; no outcome value has been read or scored.",
    }
    (args.output / "summary.json").write_text(json.dumps(summary, indent=1) + "\n")
    print(json.dumps(summary, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
