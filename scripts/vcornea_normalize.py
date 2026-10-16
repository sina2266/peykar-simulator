"""Decode pinned vCornea thickness Parquet inside the isolated scientific runtime.

Pandas and its Parquet reader belong to that runtime, never the management API.
This script loads data only; it does not import any upstream simulation module.
"""

import argparse
import hashlib
import io
import json
import math
import os
import tempfile
from pathlib import Path
from typing import Any

UPSTREAM_COMMIT = "f5c03afb9c4c7d909494146c5db608389e957070"
MAX_BYTES = 100 * 1024 * 1024
MAX_ROWS = 1_000_010


def normalized_document(raw: bytes, records: list[tuple[float, int, float]]) -> dict[str, Any]:
    """Validate rows from the trusted Parquet reader without altering their order."""
    if not 0 < len(raw) <= MAX_BYTES or raw[:4] != b"PAR1" or raw[-4:] != b"PAR1":
        raise ValueError("Expected ordinary Parquet bytes within the 100 MiB limit")
    if not 20 <= len(records) <= MAX_ROWS or len(records) % 10:
        raise ValueError("Thickness output requires 2..100001 complete ten-bin time groups")
    previous_time = -1.0
    current_time = -1.0
    for index, (time, bin_index, height) in enumerate(records):
        if (
            not math.isfinite(time)
            or time < 0
            or not time.is_integer()
            or type(bin_index) is not int
            or bin_index != index % 10
            or not math.isfinite(height)
            or height < 0
        ):
            raise ValueError("Invalid Time, Bin or Height in pinned native output")
        if bin_index == 0:
            if time <= previous_time:
                raise ValueError("Native time groups must increase strictly")
            current_time = previous_time = time
        elif time != current_time:
            raise ValueError("Native bins must share their time group")
    return {
        "schema_version": "1.0.0",
        "format": "vcornea.thickness/1.0.0",
        "schema_reference_commit": UPSTREAM_COMMIT,
        "raw_digest": hashlib.sha256(raw).hexdigest(),
        "normalizer_digest": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "columns": ["Time", "Bin", "Height"],
        "rows": records,
    }


def convert(file: Path) -> bytes:
    with file.open("rb") as stream:
        raw = stream.read(MAX_BYTES + 1)
    if not 0 < len(raw) <= MAX_BYTES:
        raise ValueError("Parquet input exceeds the 100 MiB limit or is empty")
    # Import only after basic byte bounds. The parent runner supplies memory/CPU limits.
    import pandas as pd  # type: ignore[import-not-found]

    table = pd.read_parquet(io.BytesIO(raw))
    if list(table.columns) != ["Time", "Bin", "Height"] or len(table) > MAX_ROWS:
        raise ValueError("Expected bounded pinned Time,Bin,Height Parquet schema")
    if any(table[name].dtype.kind not in "iuf" for name in table.columns):
        raise ValueError("Native Parquet columns must be numeric, never strings or booleans")
    records = []
    for time, bin_value, height in table.itertuples(index=False, name=None):
        if not math.isfinite(float(bin_value)) or not float(bin_value).is_integer():
            raise ValueError("Bin values must be integers")
        records.append((float(time), int(bin_value), float(height)))
    document = normalized_document(raw, records)
    return (
        json.dumps(document, sort_keys=True, separators=(",", ":"), allow_nan=False) + "\n"
    ).encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    arguments = parser.parse_args()
    try:
        data = convert(arguments.file)
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        descriptor, temporary = tempfile.mkstemp(dir=arguments.output.parent)
        try:
            with os.fdopen(descriptor, "wb") as stream:
                stream.write(data)
                stream.flush()
                os.fsync(stream.fileno())
            try:
                os.link(temporary, arguments.output)
            except FileExistsError:
                if arguments.output.read_bytes() != data:
                    raise ValueError("Output path already contains different bytes") from None
        finally:
            Path(temporary).unlink(missing_ok=True)
    except (ValueError, OSError, ImportError) as exc:
        parser.exit(1, f"Native Parquet conversion failed: {exc}\n")
    print(json.dumps({"output": str(arguments.output), "bytes": len(data)}))


if __name__ == "__main__":
    main()
