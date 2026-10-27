# Legacy `.xls` reader profile

Optional operator profile for reading BIFF `.xls` supplementary spreadsheets.
It is installed into a separate virtual environment, never into `uv.lock`, the
API image or the Worker:

```bash
uv venv --python 3.12 .peykar/xls-reader-venv
VIRTUAL_ENV=.peykar/xls-reader-venv uv pip install --require-hashes -r runtime_profiles/xls-reader/requirements.txt
.peykar/xls-reader-venv/bin/python runtime_profiles/xls-reader/convert_xls.py FILE.xls ...
```

`convert_xls.py` writes `FILE.xls.cells.json` (sheet, A1 reference, value, xlrd
type, source SHA-256). Formulas are not evaluated; files that are not BIFF (for
example tab-separated text named `.xls`) are recorded with an error instead of
being guessed. Peykar reads the JSON with
`peykar.data_fabric.spreadsheet.read_cells_json`. xlrd 2.0.1 is BSD-licensed
(Chris Withers / python-excel.org); its wheel and sdist hashes are pinned.
