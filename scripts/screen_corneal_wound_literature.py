"""Screen open-access corneal epithelial wound studies for reusable source data.

Explicit offline operator for literature screening only. It queries the Europe PMC
REST API, downloads each candidate's open full-text JATS XML and records the
declared supplementary files, captions, species/method keywords and wound sizes.
It does not download supplementary data, fit models or rank studies by outcome
values. Raw responses are retained with SHA-256 receipts under ``--output``.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from concurrent.futures import ThreadPoolExecutor
from datetime import UTC, datetime
from pathlib import Path
from typing import Any
from xml.etree import ElementTree as ET

SEARCH = "https://www.ebi.ac.uk/europepmc/webservices/rest/search"
FULLTEXT = "https://www.ebi.ac.uk/europepmc/webservices/rest/{pmcid}/fullTextXML"
USER_AGENT = "PeykarLiteratureScreen/1.0 (+https://github.com/sina2266/Peykar-Simulator)"
WOUND_TERMS = (
    '("corneal epithelial wound" OR "corneal epithelial defect" OR '
    '"corneal epithelial debridement" OR "epithelial debridement" OR '
    '"corneal abrasion" OR "corneal wound healing" OR "epithelial wound closure" OR '
    '"corneal re-epithelialization" OR "corneal reepithelialization")'
)
QUERIES = {
    "fluorescein_supplement": f"{WOUND_TERMS} AND fluorescein AND OPEN_ACCESS:y AND IN_EPMC:y "
    "AND HAS_SUPPL:y",
    "fluorescein_open": f"{WOUND_TERMS} AND fluorescein AND OPEN_ACCESS:y AND IN_EPMC:y",
}
SPECIES = {
    "mouse": r"\b(mouse|mice|murine)\b",
    "rat": r"\b(rat|rats)\b",
    "rabbit": r"\b(rabbit|rabbits)\b",
    "pig": r"\b(pig|pigs|porcine|swine|minipig)\b",
    "dog": r"\b(dog|dogs|canine)\b",
    "human": r"\b(patient|patients|human subjects|volunteers)\b",
}
METHODS = {
    "algerbrush": r"alger ?brush",
    "blade": r"\b(blade|scalpel|golf club|beaver)\b",
    "burr": r"\b(burr|diamond)\b",
    "heptanol": r"heptanol",
    "ethanol": r"\b(ethanol|alcohol)\b[^.]{0,40}(epitheli|debrid)",
    "alkali": r"\b(alkali|naoh|sodium hydroxide)\b",
    "excimer": r"\b(excimer|ptk|prk|photorefractive|phototherapeutic)\b",
    "trephine": r"trephin",
    "diabetic": r"\b(diabet|streptozotocin|stz|db/db)\b",
}
DATA_CAPTION = re.compile(
    r"source data|raw data|dataset|data set|underlying data|minimal data|individual data|"
    r"supplementary data|data file|S\d+ (data|file|dataset)|supporting data values",
    re.I,
)
DATA_SUFFIX = (".xlsx", ".xls", ".csv", ".txt", ".tsv", ".ods", ".zip")
SIZE = re.compile(r"(\d+(?:\.\d+)?)\s?-?\s?mm(?![\w/])")


def _get(url: str, *, attempts: int = 3) -> tuple[int, bytes]:
    last: Exception | None = None
    for attempt in range(attempts):
        try:
            request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
            with urllib.request.urlopen(request, timeout=90) as response:
                return int(response.status), response.read(25_000_000)
        except urllib.error.HTTPError as error:
            return int(error.code), b""
        except (urllib.error.URLError, TimeoutError, ConnectionError) as error:
            last = error
            time.sleep(2.0 * (attempt + 1))
    raise RuntimeError(f"Request failed after {attempts} attempts: {url}: {last}")


def _text(node: ET.Element | None) -> str:
    return re.sub(r"\s+", " ", "".join(node.itertext())).strip() if node is not None else ""


def _search(
    query: str, receipts: list[dict[str, Any]], raw: Path, name: str
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    cursor = "*"
    page = 0
    while True:
        params = {
            "query": query,
            "format": "json",
            "pageSize": "1000",
            "resultType": "lite",
            "cursorMark": cursor,
        }
        url = SEARCH + "?" + urllib.parse.urlencode(params)
        status, body = _get(url)
        path = raw / f"search-{name}-{page}.json"
        path.write_bytes(body)
        receipts.append(_receipt(url, status, path, body))
        if status != 200:
            raise RuntimeError(f"Europe PMC search failed with HTTP {status}")
        payload = json.loads(body)
        batch = payload.get("resultList", {}).get("result", [])
        results.extend(batch)
        next_cursor = payload.get("nextCursorMark")
        if not batch or not next_cursor or next_cursor == cursor:
            return results
        cursor = next_cursor
        page += 1
        time.sleep(0.5)


def _receipt(url: str, status: int, path: Path, body: bytes) -> dict[str, Any]:
    return {
        "url": url,
        "utc": datetime.now(UTC).isoformat(),
        "status": status,
        "file": path.name,
        "bytes": len(body),
        "sha256": hashlib.sha256(body).hexdigest(),
    }


def _screen(pmcid: str, body: bytes) -> dict[str, Any]:
    text = body.decode("utf-8", "replace")
    if "<!ENTITY" in text.upper():
        return {"pmcid": pmcid, "parse_error": "entity declarations are not parsed"}
    try:
        root = ET.fromstring(re.sub(r"<!DOCTYPE[^>]*>", "", text, count=1))
    except ET.ParseError as error:
        return {"pmcid": pmcid, "parse_error": str(error)}
    xlink = "{http://www.w3.org/1999/xlink}href"
    plain = _text(root.find(".//body"))
    methods = (
        " ".join(
            _text(sec)
            for sec in root.iter("sec")
            if re.search(r"method|material|procedure|experimental", _text(sec.find("title")), re.I)
        )
        or plain
    )
    supplements = []
    for node in root.iter("supplementary-material"):
        hrefs = [n.get(xlink, "") for n in node.iter() if n.get(xlink)]
        supplements.append(
            {
                "label": _text(node.find("label")),
                "caption": _text(node.find("caption"))[:400],
                "hrefs": sorted(set(hrefs)),
            }
        )
    data_files = [
        s
        for s in supplements
        if any(h.lower().endswith(DATA_SUFFIX) for h in s["hrefs"])
        or DATA_CAPTION.search(s["label"] + " " + s["caption"])
    ]
    wound_figures = []
    for figure in root.iter("fig"):
        caption = _text(figure.find("caption"))
        if re.search(r"fluorescein|wound|defect|re-?epitheliali", caption, re.I) and re.search(
            r"\b(\d+|0)\s?h\b|hour|day", caption, re.I
        ):
            wound_figures.append({"label": _text(figure.find("label")), "caption": caption[:300]})
    sizes = sorted(
        {
            float(m.group(1))
            for m in SIZE.finditer(methods)
            if 0.5 <= float(m.group(1)) <= 12
            and re.search(
                r"trephin|wound|debrid|defect|diameter|abrasion|epitheli",
                methods[max(0, m.start() - 80) : m.end() + 40],
                re.I,
            )
        }
    )
    license_node = root.find(".//license")
    return {
        "pmcid": pmcid,
        "title": _text(root.find(".//article-title"))[:300],
        "journal": _text(root.find(".//journal-title")),
        "doi": next(
            (_text(n) for n in root.iter("article-id") if n.get("pub-id-type") == "doi"), ""
        ),
        "license": (license_node.get(xlink, "") if license_node is not None else "")
        or _text(license_node)[:200],
        "species": [k for k, p in SPECIES.items() if re.search(p, methods, re.I)],
        "methods": [k for k, p in METHODS.items() if re.search(p, methods, re.I)],
        "candidate_wound_diameters_mm": sizes,
        "supplements": supplements,
        "data_like_supplements": len(data_files),
        "wound_time_figures": wound_figures[:12],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--limit", type=int, default=2000)
    parser.add_argument("--workers", type=int, default=6)
    parser.add_argument(
        "--reuse-fulltext",
        type=Path,
        help="Directory of full-text XML bytes from an earlier stopped attempt (rehashed)",
    )
    args = parser.parse_args(argv)
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("--output must be a new or empty directory")
    if not 1 <= args.workers <= 8:
        parser.error("--workers must be within 1..8")
    raw = args.output / "raw"
    (raw / "fulltext").mkdir(parents=True, exist_ok=True)
    receipts: list[dict[str, Any]] = []
    union: dict[str, dict[str, Any]] = {}
    for name, query in QUERIES.items():
        for item in _search(query, receipts, raw, name):
            pmcid = item.get("pmcid")
            if pmcid:
                union.setdefault(pmcid, {**item, "queries": []})["queries"].append(name)

    def fetch(pmcid: str) -> tuple[str, dict[str, Any], bytes]:
        url = FULLTEXT.format(pmcid=pmcid)
        path = raw / "fulltext" / f"{pmcid}.xml"
        cached = args.reuse_fulltext / f"{pmcid}.xml" if args.reuse_fulltext else None
        if cached is not None and cached.is_file() and cached.stat().st_size > 0:
            body = cached.read_bytes()
            path.write_bytes(body)
            receipt = _receipt(url, 200, path, body)
            receipt["reused_from_stopped_attempt"] = str(cached)
            return pmcid, receipt, body
        status, body = _get(url)
        if status == 200 and body:
            path.write_bytes(body)
        time.sleep(0.2)
        return pmcid, _receipt(url, status, path, body), body if status == 200 else b""

    screened = []
    ordered = sorted(union)[: args.limit]
    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        results = {}
        for index, (pmcid, receipt, body) in enumerate(pool.map(fetch, ordered)):
            receipts.append(receipt)
            results[pmcid] = body
            if index % 100 == 0:
                print(f"{index}/{len(ordered)} {pmcid}", flush=True)
    for pmcid in ordered:
        body = results[pmcid]
        row = _screen(pmcid, body) if body else {"pmcid": pmcid, "http": "unavailable"}
        row["queries"] = union[pmcid]["queries"]
        row["pub_year"] = union[pmcid].get("pubYear")
        screened.append(row)
    (args.output / "receipts.json").write_text(json.dumps(receipts, indent=1) + "\n")
    (args.output / "screen.json").write_text(json.dumps(screened, indent=1) + "\n")
    summary = {
        "utc": datetime.now(UTC).isoformat(),
        "queries": QUERIES,
        "union_pmcids": len(union),
        "screened": len(screened),
        "with_data_like_supplements": sum(1 for r in screened if r.get("data_like_supplements")),
        "scope": "Metadata/full-text screening only; no supplementary data or outcomes read.",
    }
    (args.output / "summary.json").write_text(json.dumps(summary, indent=1) + "\n")
    print(json.dumps(summary, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
