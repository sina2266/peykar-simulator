"""Check a native agent's numerical output envelope without importing a solver."""

import csv
import io
import json
import math

from peykar_core.contracts import NativeRunManifest, Quantity
from peykar_core.errors import InvalidRequest
from peykar_core.units import convert


def validate_native_outputs(manifest: NativeRunManifest, blobs: dict[str, bytes]) -> None:
    if set(blobs) != {"native.csv", "quantities.json"}:
        raise InvalidRequest("Native completion requires numerical CSV and quantity metadata")
    try:
        metadata = json.loads(blobs["quantities.json"])
        text = blobs["native.csv"].decode("utf-8-sig")
        if "\x00" in text:
            raise ValueError("NUL in native output")
        reader = csv.reader(io.StringIO(text), strict=True)
        header = next(reader)
        if (
            len(header) < 2
            or len(header) != len(set(header))
            or "Time" not in header
            or any(not name.strip() for name in header)
        ):
            raise ValueError("Native output headers differ")
        time_index = header.index("Time")
        times: list[float] = []
        individual_ids: set[float] = set()
        for row in reader:
            if len(row) != len(header) or len(times) >= 100001:
                raise ValueError("Native output shape differs")
            values = [float(value) for value in row]
            if not all(math.isfinite(value) for value in values):
                raise ValueError("Native numerical output is nonfinite")
            if manifest.profile_snapshot.kind == "osp_pkml" and "IndividualId" in header:
                individual = values[header.index("IndividualId")]
                if individual < 0 or not individual.is_integer():
                    raise ValueError("PKML individual identity is invalid")
                individual_ids.add(individual)
            times.append(values[time_index])
        if (
            len(times) < 2
            or type(metadata["samples"]) is not int
            or metadata["samples"] != len(times)
        ):
            raise ValueError("Native sample metadata differs")
        if metadata["scientific_validity"] != "not_established":
            raise ValueError("Execution cannot claim biological validity")
        kind = manifest.profile_snapshot.kind
        if kind == "copasi_sbml":
            intervals, requested_duration = manifest.plan.intervals, manifest.plan.duration
            if intervals is None or requested_duration is None:
                raise ValueError("Native time controls are missing")
            if metadata["engine"] != "COPASI" or len(times) != intervals + 1:
                raise ValueError("Native engine/sample grid differs")
            unit = metadata["model_units"]["time_unit"]
            physical = [convert(Quantity(value=t, unit=unit), "second") for t in times]
            duration = convert(requested_duration, "second")
            tolerance = max(1e-10, abs(duration) * 1e-9)
            if any(abs(t - duration * i / intervals) > tolerance for i, t in enumerate(physical)):
                raise ValueError("Native physical sample grid differs from its frozen plan")
        elif metadata["engine"] != "OSPSuite" or any(
            b < a for a, b in zip(times, times[1:], strict=False)
        ):
            raise ValueError("PKML engine/time sequence differs")
        else:
            quantities = metadata["quantities"]
            # OSPSuite reports IndividualId as a dimensionless identity column,
            # outside metaData. This reviewed driver runs one original simulation.
            columns = set(header) - {"IndividualId"}
            if len(individual_ids) > 1:
                raise ValueError("This PKML driver does not combine multiple individuals")
            if not isinstance(quantities, dict) or set(quantities) != columns:
                raise ValueError("PKML quantity metadata differs from its CSV columns")
            if any(
                not isinstance(item, dict)
                or not isinstance(item.get("unit"), str)
                or not item["unit"].strip()
                or not isinstance(item.get("dimension"), str)
                or not item["dimension"].strip()
                for item in quantities.values()
            ):
                raise ValueError("PKML quantity units/dimensions are absent")
    except (ValueError, TypeError, KeyError, StopIteration, UnicodeDecodeError, csv.Error) as error:
        raise InvalidRequest(
            "Native numerical output does not match its declared envelope"
        ) from error
