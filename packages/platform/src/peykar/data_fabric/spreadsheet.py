"""Bounded inert readers for published supplementary spreadsheets (XLSX/CSV).

The XLSX reader parses only workbook, relationship, shared-string and worksheet
XML parts. It rejects DTDs/entities, oversized archives and duplicate members,
never evaluates formulas (cached values are returned and flagged) and never runs
macros or external links. ``masked_preview`` shows text labels while hiding every
numeric value, so a study can be triaged structurally before outcomes are seen.
"""

from __future__ import annotations

import csv
import io
import re
import zipfile
from dataclasses import dataclass
from typing import Any
from xml.etree import ElementTree as ET

from peykar_core.errors import InvalidRequest

MAIN = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
REL = "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}"
PKG_REL = "{http://schemas.openxmlformats.org/package/2006/relationships}"
MAX_ARCHIVE_BYTES = 60_000_000
MAX_MEMBER_BYTES = 120_000_000
MAX_CELLS = 2_000_000
_REF = re.compile(r"^([A-Z]{1,3})(\d{1,7})$")
_NUMERIC_TEXT = re.compile(r"^[-+±]?\(?\d[\d,.\s]*(e[-+]?\d+)?\)?\s*%?$", re.I)


@dataclass(frozen=True)
class Cell:
    ref: str
    row: int
    column: int
    value: str | float
    is_number: bool
    is_formula: bool


def column_index(letters: str) -> int:
    index = 0
    for letter in letters:
        index = index * 26 + (ord(letter) - 64)
    return index


def column_letters(index: int) -> str:
    letters = ""
    while index:
        index, remainder = divmod(index - 1, 26)
        letters = chr(65 + remainder) + letters
    return letters


def _parse(raw: bytes) -> ET.Element:
    upper = raw[:4096].upper()
    if b"<!DOCTYPE" in upper or b"<!ENTITY" in raw.upper():
        raise InvalidRequest("Spreadsheet XML must not declare DTDs or entities")
    try:
        return ET.fromstring(raw)
    except ET.ParseError as error:
        raise InvalidRequest("Malformed spreadsheet XML part") from error


def read_xlsx(raw: bytes) -> dict[str, list[Cell]]:
    """Return every worksheet's non-empty cells in workbook order."""
    if not raw or len(raw) > MAX_ARCHIVE_BYTES:
        raise InvalidRequest("XLSX input is empty or exceeds its bounded size")
    try:
        archive = zipfile.ZipFile(io.BytesIO(raw))
    except zipfile.BadZipFile as error:
        raise InvalidRequest("Input is not an XLSX (ZIP) workbook") from error
    with archive:
        infos = archive.infolist()
        names = [info.filename for info in infos]
        if len(names) != len(set(names)) or len(names) > 2000:
            raise InvalidRequest("Unexpected workbook member inventory")
        if sum(info.file_size for info in infos) > MAX_MEMBER_BYTES:
            raise InvalidRequest("Workbook expands beyond its bounded size")
        try:
            workbook = _parse(archive.read("xl/workbook.xml"))
            relations = _parse(archive.read("xl/_rels/workbook.xml.rels"))
        except KeyError as error:
            raise InvalidRequest("Workbook parts are missing") from error
        targets = {
            node.get("Id"): node.get("Target", "")
            for node in relations.iter(f"{PKG_REL}Relationship")
        }
        strings: list[str] = []
        if "xl/sharedStrings.xml" in names:
            shared = _parse(archive.read("xl/sharedStrings.xml"))
            strings = ["".join(si.itertext()) for si in shared.iter(f"{MAIN}si")]
        sheets: dict[str, list[Cell]] = {}
        total = 0
        for sheet in workbook.iter(f"{MAIN}sheet"):
            name = sheet.get("name", "")
            target = targets.get(sheet.get(f"{REL}id"), "")
            path = target.lstrip("/") if target.startswith("/") else f"xl/{target}"
            path = re.sub(r"[^/]+/\.\./", "", path)
            if path not in names:
                continue
            cells = []
            for node in _parse(archive.read(path)).iter(f"{MAIN}c"):
                ref = node.get("r", "")
                match = _REF.match(ref)
                if not match:
                    continue
                kind = node.get("t", "n")
                formula = node.find(f"{MAIN}f") is not None
                value_node = node.find(f"{MAIN}v")
                text: str | None = value_node.text if value_node is not None else None
                if kind == "inlineStr":
                    inline = node.find(f"{MAIN}is")
                    text = "".join(inline.itertext()) if inline is not None else None
                if text is None or text == "":
                    continue
                value: str | float
                is_number = False
                if kind == "s":
                    try:
                        value = strings[int(text)]
                    except (ValueError, IndexError) as error:
                        raise InvalidRequest("Invalid shared-string reference") from error
                elif kind in {"str", "inlineStr", "e", "b"}:
                    value = text
                else:
                    try:
                        value = float(text)
                        is_number = True
                    except ValueError:
                        value = text
                cells.append(
                    Cell(
                        ref=ref,
                        row=int(match.group(2)),
                        column=column_index(match.group(1)),
                        value=value,
                        is_number=is_number,
                        is_formula=formula,
                    )
                )
                total += 1
                if total > MAX_CELLS:
                    raise InvalidRequest("Workbook has too many cells")
            sheets[name] = cells
    return sheets


def read_csv(raw: bytes) -> dict[str, list[Cell]]:
    if not raw or len(raw) > MAX_ARCHIVE_BYTES:
        raise InvalidRequest("CSV input is empty or exceeds its bounded size")
    text = raw.decode("utf-8-sig", errors="replace")
    dialect = "excel-tab" if text.count("\t") > text.count(",") else "excel"
    cells = []
    for r, row in enumerate(csv.reader(io.StringIO(text), dialect=dialect), start=1):
        for c, entry in enumerate(row, start=1):
            entry = entry.strip()
            if not entry:
                continue
            try:
                value: str | float = float(entry)
                number = True
            except ValueError:
                value, number = entry, False
            cells.append(Cell(f"{column_letters(c)}{r}", r, c, value, number, False))
    return {"csv": cells}


def masked_preview(sheets: dict[str, list[Cell]], *, max_rows: int = 60) -> dict[str, Any]:
    """Text labels with every numeric value replaced by ``#`` (formulas by ``=``)."""
    preview = {}
    for name, cells in sheets.items():
        rows: dict[int, list[str]] = {}
        for cell in cells:
            if cell.row > max_rows:
                continue
            if cell.is_number:
                shown = "=" if cell.is_formula else "#"
            elif _NUMERIC_TEXT.match(str(cell.value).strip()):
                shown = "#t"  # numbers stored as text are hidden too
            else:
                shown = str(cell.value)[:40]
            rows.setdefault(cell.row, []).append(f"{cell.ref}:{shown}")
        preview[name] = {
            "cells": len(cells),
            "numeric_cells": sum(c.is_number for c in cells),
            "max_row": max((c.row for c in cells), default=0),
            "max_column": column_letters(max((c.column for c in cells), default=0)),
            "rows": [" | ".join(rows[r]) for r in sorted(rows)],
        }
    return preview


def read_cells_json(raw: bytes) -> dict[str, list[Cell]]:
    """Read the inert JSON inventory written by ``runtime_profiles/xls-reader``."""
    import json

    if not raw or len(raw) > MAX_ARCHIVE_BYTES:
        raise InvalidRequest("Cell inventory is empty or exceeds its bounded size")
    try:
        payload = json.loads(raw)
    except json.JSONDecodeError as error:
        raise InvalidRequest("Cell inventory is not JSON") from error
    sheets: dict[str, list[Cell]] = {}
    for name, entries in payload.get("sheets", {}).items():
        cells = []
        for entry in entries:
            match = _REF.match(str(entry.get("ref", "")))
            if not match:
                raise InvalidRequest("Cell inventory has an invalid reference")
            value = entry.get("value")
            is_number = entry.get("type") == "number" and isinstance(value, (int, float))
            cells.append(
                Cell(
                    ref=entry["ref"],
                    row=int(match.group(2)),
                    column=column_index(match.group(1)),
                    value=float(value) if is_number else str(value),
                    is_number=is_number,
                    is_formula=False,
                )
            )
        sheets[str(name)] = cells
    return sheets
