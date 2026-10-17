"""Offline CellML admission and retained operator execution; never runs model code."""

import csv
import io
import json
import math
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Final

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar.model_import.native_models import MAX_XML_BYTES
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar_core.contracts import (
    CellMLExecutionReceipt,
    CellMLInspection,
    CellMLModelPackage,
    CellMLModelSpec,
    CellMLVariable,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import bytes_digest

NS: Final = "http://www.cellml.org/cellml/2.0#"
BUILTIN_UNITS = set(
    [
        "ampere",
        "becquerel",
        "candela",
        "coulomb",
        "dimensionless",
        "farad",
        "gram",
        "gray",
        "henry",
        "hertz",
        "joule",
        "katal",
        "kelvin",
        "kilogram",
        "litre",
        "lumen",
        "lux",
        "metre",
        "mole",
        "newton",
        "ohm",
        "pascal",
        "radian",
        "second",
        "siemens",
        "sievert",
        "steradian",
        "tesla",
        "volt",
        "watt",
        "weber",
    ]
)


def inspect_cellml(raw: bytes) -> CellMLInspection:
    if not raw or len(raw) > MAX_XML_BYTES:
        raise InvalidRequest("CellML requires 1 to 10485760 bytes")
    try:
        text = raw.decode("utf-8-sig")
        if "\x00" in text or "<!DOCTYPE" in text.upper() or "<!ENTITY" in text.upper():
            raise ValueError("DTD/entities are unsupported")
        root = ET.fromstring(text)
        elements = list(root.iter())
        if len(elements) > 200000 or any(
            e.tag.startswith("{http://www.w3.org/2001/XInclude}") for e in elements
        ):
            raise ValueError("External includes or oversized XML tree")
    except (ValueError, ET.ParseError) as error:
        raise InvalidRequest(
            "CellML requires bounded UTF-8 XML without DTD/entities/includes"
        ) from error
    if root.tag != f"{{{NS}}}model":
        raise InvalidRequest("CellML admission supports the original CellML 2.0 namespace only")
    if any(e.tag == f"{{{NS}}}import" for e in elements):
        raise InvalidRequest(
            "CellML imports require explicit offline flattening; no remote resolution"
        )

    def name(element: ET.Element) -> str:
        value = element.get("name", "")
        if len(value) > 200 or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", value):
            raise InvalidRequest("CellML declarations require bounded CellML identifiers")
        return value

    units = {}
    for declaration in root.findall(f"{{{NS}}}units"):
        key = name(declaration)
        if key in units or key in BUILTIN_UNITS:
            raise InvalidRequest("Duplicate or reserved CellML unit name")
        units[key] = tuple(dict(e.attrib) for e in declaration.findall(f"{{{NS}}}unit"))
    known = set(units) | BUILTIN_UNITS
    for factors in units.values():
        for factor in factors:
            if factor.get("units") not in known:
                raise InvalidRequest("Unknown CellML unit reference")
    variables = []
    components = set()
    for component in root.findall(f"{{{NS}}}component"):
        component_name = name(component)
        if component_name in components:
            raise InvalidRequest("Duplicate CellML component")
        components.add(component_name)
        names = set()
        for variable in component.findall(f"{{{NS}}}variable"):
            variable_name = name(variable)
            if variable_name in names or variable.get("units") not in known:
                raise InvalidRequest("Duplicate variable or unknown CellML units")
            names.add(variable_name)
            variables.append(
                CellMLVariable(
                    component=component_name,
                    name=variable_name,
                    units=variable.attrib["units"],
                    initial_value=variable.get("initial_value"),
                    interface=variable.get("interface"),
                )
            )
    if not variables or len(variables) > 4096:
        raise InvalidRequest("CellML admission requires 1 to 4096 variables")
    return CellMLInspection(
        model_name=name(root),
        namespace=NS,
        units=units,
        variables=tuple(variables),
        warnings=(
            "Original unit definitions and variable names retained; no unit substitution.",
            "Structural inspection is not full CellML validation or biological validation.",
            "Operator solver supports self-contained ODEs without resets or external variables.",
            "Admission never generates or executes code; managed queue support is separate.",
        ),
    )


class CellMLService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def admit(self, spec: CellMLModelSpec, raw: bytes) -> CellMLModelPackage:
        inspection = inspect_cellml(raw)
        package = CellMLModelPackage(
            spec=spec, content_digest=self.artifacts.put(raw), inspection=inspection
        )
        self.db.register("cellml_model", spec.id, spec.version, package.model_dump_json())
        return package

    def get(self, reference: Reference) -> CellMLModelPackage:
        package = CellMLModelPackage.model_validate_json(
            self.db.object("cellml_model", reference.id, reference.version)
        )
        self.artifacts.get(package.content_digest)
        return package

    def discover(self, limit: int = 50, offset: int = 0) -> tuple[CellMLModelPackage, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("cellml_model", limit=limit, offset=offset)
        )

    def import_execution(self, receipt: CellMLExecutionReceipt, bundle: Path) -> str:
        """Trusted offline operator assertion, not independent scientific certification."""
        model = self.get(receipt.model)
        runtime = NativeRuntimeService(self.db, self.artifacts).get(receipt.runtime)
        if (
            model.content_digest != receipt.model_digest
            or runtime.engine != "libcellml"
            or runtime.image_digest != receipt.image_digest
            or runtime.artifact_digests.get("driver.py") != receipt.driver_digest
        ):
            raise Conflict("CellML model/runtime/driver identity mismatch")
        required = {"result.json", "series.csv", "generated.py"}
        if set(receipt.artifacts) != required:
            raise InvalidRequest(
                "CellML execution requires exactly result.json, series.csv, generated.py"
            )
        blobs = {}
        for key, checksum in receipt.artifacts.items():
            path = bundle / checksum
            if path.is_symlink() or not path.is_file() or path.stat().st_size > 32 * 1024**2:
                raise InvalidRequest("Missing, unsafe or oversized CellML result artifact")
            raw = path.read_bytes()
            if bytes_digest(raw) != checksum:
                raise Conflict("CellML result checksum mismatch")
            blobs[key] = raw
        self.validate_result(receipt, blobs)
        result = json.loads(blobs["result.json"])
        variables = {(v.component, v.name): v.units for v in model.inspection.variables}
        if any(
            variables.get((m["component"], m["name"])) != m["units"]
            for m in (result["voi"], *result["columns"])
        ):
            raise Conflict(
                "CellML output variable identities or original units differ from the model"
            )
        for raw in blobs.values():
            self.artifacts.put(raw)
        return self.db.register(
            "cellml_execution", receipt.id, receipt.version, receipt.model_dump_json()
        )

    @staticmethod
    def validate_result(receipt: CellMLExecutionReceipt, blobs: dict[str, bytes]) -> None:
        try:
            result = json.loads(blobs["result.json"])
            controls = receipt.controls
            if (
                result["status"] != "succeeded"
                or result["model_digest"] != receipt.model_digest
                or result["driver_digest"] != receipt.driver_digest
                or result["controls"] != controls.model_dump(mode="json")
                or result["voi"]["units"] != controls.time_unit
            ):
                raise ValueError("Result identities or original time unit differ")
            metadata = result["columns"]
            if not 1 <= len(metadata) <= 4096 or any(
                not all(
                    isinstance(m[k], str) and m[k] for k in ("name", "component", "units", "kind")
                )
                for m in metadata
            ):
                raise ValueError("Invalid CellML column metadata")
            names = [m["component"] + "/" + m["name"] for m in metadata]
            if len(set(names)) != len(names):
                raise ValueError("Ambiguous CellML output identities")
            rows = list(csv.reader(io.StringIO(blobs["series.csv"].decode("utf-8"))))
            if rows[0] != ["time", *names] or len(rows) != controls.intervals + 2:
                raise ValueError("CellML result dimensions differ")
            previous = -math.inf
            for index, row in enumerate(rows[1:]):
                numbers = [float(value) for value in row]
                expected = (
                    controls.start + (controls.end - controls.start) * index / controls.intervals
                )
                if (
                    len(numbers) != len(metadata) + 1
                    or not all(math.isfinite(value) for value in numbers)
                    or numbers[0] <= previous
                    or not math.isclose(numbers[0], expected, rel_tol=1e-12, abs_tol=1e-12)
                ):
                    raise ValueError("CellML samples require a finite exact declared grid")
                previous = numbers[0]
        except (ValueError, KeyError, TypeError, IndexError, UnicodeError) as error:
            raise InvalidRequest("Invalid CellML execution output") from error

    def execution(self, reference: Reference) -> CellMLExecutionReceipt:
        receipt = CellMLExecutionReceipt.model_validate_json(
            self.db.object("cellml_execution", reference.id, reference.version)
        )
        model = self.get(receipt.model)
        runtime = NativeRuntimeService(self.db, self.artifacts).get(receipt.runtime)
        if (
            model.content_digest != receipt.model_digest
            or runtime.image_digest != receipt.image_digest
            or runtime.artifact_digests.get("driver.py") != receipt.driver_digest
        ):
            raise Conflict("CellML retained lineage mismatch")
        for checksum in receipt.artifacts.values():
            self.artifacts.get(checksum)
        return receipt

    def executions(self, limit: int = 50, offset: int = 0) -> tuple[CellMLExecutionReceipt, ...]:
        return tuple(
            self.execution(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("cellml_execution", limit=limit, offset=offset)
        )
