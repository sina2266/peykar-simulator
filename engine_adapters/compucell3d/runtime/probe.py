"""Verify installed native imports and Parquet I/O; no biological model is executed."""

import json
import platform
import tempfile
from pathlib import Path

import cc3d
import numpy
import pandas
import pyarrow
from cc3d import CompuCellSetup
from cc3d.cpp import CompuCell
from PyQt5 import QtCore

if cc3d.__version__ != "4.7.0" or cc3d.__revision__ != "5":
    raise RuntimeError("Unexpected CompuCell3D identity")
if platform.machine() != "x86_64":
    raise RuntimeError("This lock requires Linux x86-64")
if CompuCellSetup is None or CompuCell is None:
    raise RuntimeError("Native engine imports failed")

with tempfile.TemporaryDirectory(prefix="peykar-cc3d-probe-") as directory:
    path = Path(directory) / "probe.parquet"
    expected = pandas.DataFrame({"Time": [1.0, 2.0], "Bin": [0, 1], "Height": [3.0, 4.0]})
    expected.to_parquet(path, index=False)
    actual = pandas.read_parquet(path)
    if not actual.equals(expected):
        raise RuntimeError("Parquet round trip differs")

print(
    json.dumps(
        {
            "engine": "CompuCell3D",
            "version": cc3d.__version__,
            "revision": cc3d.__revision__,
            "internal_commit_label": cc3d.__githash__,
            "python": platform.python_version(),
            "architecture": platform.machine(),
            "numpy": numpy.__version__,
            "pandas": pandas.__version__,
            "pyarrow": pyarrow.__version__,
            "qt": QtCore.qVersion(),
            "native_imports": "passed",
            "parquet_roundtrip": "passed",
            "biological_model_executed": False,
            "scientific_validity": "not_established",
        },
        sort_keys=True,
    )
)
