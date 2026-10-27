"""Inert spreadsheet reader and masked previews; synthetic workbooks only."""

import io
import zipfile

import pytest

from peykar.data_fabric.spreadsheet import masked_preview, read_csv, read_xlsx
from peykar_core.errors import InvalidRequest

WORKBOOK = (
    '<?xml version="1.0"?><workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/'
    '2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">'
    '<sheets><sheet name="Fig 2" sheetId="1" r:id="rId1"/></sheets></workbook>'
)
RELS = (
    '<?xml version="1.0"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/'
    'relationships"><Relationship Id="rId1" Type="worksheet" Target="worksheets/sheet1.xml"/>'
    "</Relationships>"
)
STRINGS = (
    '<?xml version="1.0"?><sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">'
    "<si><t>Wound area</t></si><si><t>Vehicle</t></si></sst>"
)
SHEET = (
    '<?xml version="1.0"?><worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/'
    '2006/main"><sheetData><row r="1"><c r="A1" t="s"><v>0</v></c></row>'
    '<row r="2"><c r="A2" t="s"><v>1</v></c><c r="B2"><v>0.734</v></c>'
    '<c r="C2"><f>B2/2</f><v>0.367</v></c></row></sheetData></worksheet>'
)


def workbook(sheet=SHEET):
    buffer = io.BytesIO()
    with zipfile.ZipFile(buffer, "w") as archive:
        archive.writestr("xl/workbook.xml", WORKBOOK)
        archive.writestr("xl/_rels/workbook.xml.rels", RELS)
        archive.writestr("xl/sharedStrings.xml", STRINGS)
        archive.writestr("xl/worksheets/sheet1.xml", sheet)
    return buffer.getvalue()


def test_reads_labels_numbers_and_flags_cached_formulas():
    sheets = read_xlsx(workbook())
    cells = {cell.ref: cell for cell in sheets["Fig 2"]}
    assert cells["A1"].value == "Wound area"
    assert cells["B2"].value == pytest.approx(0.734) and cells["B2"].is_number
    assert cells["C2"].is_formula and cells["C2"].value == pytest.approx(0.367)


def test_masked_preview_never_shows_numeric_values():
    preview = masked_preview(read_xlsx(workbook()))
    text = " ".join(preview["Fig 2"]["rows"])
    assert "Vehicle" in text and "0.734" not in text and "0.367" not in text
    assert preview["Fig 2"]["numeric_cells"] == 2


def test_rejects_entities_and_non_zip_input():
    with pytest.raises(InvalidRequest):
        read_xlsx(workbook(sheet='<!DOCTYPE x [<!ENTITY a "b">]><worksheet/>'))
    with pytest.raises(InvalidRequest):
        read_xlsx(b"not a workbook")


def test_csv_reader_detects_numbers():
    sheets = read_csv(b"group,0h,24h\nvehicle,1.0,0.42\n")
    numbers = [c for c in sheets["csv"] if c.is_number]
    assert [c.value for c in numbers] == [1.0, 0.42]


def test_masked_preview_hides_numbers_stored_as_text():
    sheets = read_csv(b"group,label\nvehicle,'0.42\n")
    sheets["csv"].append(sheets["csv"][0].__class__("C2", 2, 3, "26116.296", False, False))
    text = " ".join(masked_preview(sheets)["csv"]["rows"])
    assert "26116.296" not in text and "#t" in text and "vehicle" in text
