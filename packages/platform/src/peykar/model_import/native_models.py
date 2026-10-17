"""Admit declarative native XML models; never import or execute their code."""

import re
import xml.etree.ElementTree as ET

from peykar.infrastructure.artifacts import ArtifactStore
from peykar.infrastructure.database import Database
from peykar.model_import.native_runtimes import NativeRuntimeService
from peykar_core.contracts import (
    NativeExecutionProfile,
    NativeModelInspection,
    NativeModelPackage,
    NativeModelSpec,
    Reference,
)
from peykar_core.errors import InvalidRequest

MAX_XML_BYTES = 10 * 1024**2


def inspect_xml(format: str, raw: bytes) -> NativeModelInspection:
    if not raw or len(raw) > MAX_XML_BYTES:
        raise InvalidRequest("Native XML requires 1 to 10485760 bytes")
    try:
        text = raw.decode("utf-8-sig")
        if "\x00" in text or "<!DOCTYPE" in text.upper() or "<!ENTITY" in text.upper():
            raise ValueError("DTD/entities are unsupported")
        root = ET.fromstring(text)
        if sum(1 for _ in root.iter()) > 200000:
            raise ValueError("XML tree exceeds admission bounds")
        if any(e.tag.startswith("{http://www.w3.org/2001/XInclude}") for e in root.iter()):
            raise ValueError("External XML includes are unsupported")
    except (ValueError, ET.ParseError) as error:
        raise InvalidRequest(
            "Native model requires bounded UTF-8 XML without DTD/entities/includes"
        ) from error
    namespace = root.tag[1:].split("}")[0] if root.tag.startswith("{") else ""
    name = root.tag.split("}")[-1]
    if format == "pkml":
        if name != "SimulationTransfer" or root.find("Simulation") is None:
            raise InvalidRequest("PKML requires an original SimulationTransfer/Simulation document")
        return NativeModelInspection(
            root=name,
            namespace=namespace,
            warnings=(
                (
                    "PKML units/schedule remain in the raw model; "
                    "solver-reported units are retained at execution."
                ),
            ),
        )
    if (
        format != "sbml"
        or name != "sbml"
        or not re.fullmatch(
            r"http://www\.sbml\.org/sbml/(?:level3/version[12]/core|level2/version[1-5])", namespace
        )
    ):
        raise InvalidRequest("Supported SBML admission requires core Level 2 or Level 3 XML")
    ns = {"s": namespace}
    model = root.find("s:model", ns)
    if model is None:
        raise InvalidRequest("SBML model element is missing")
    if any(e.tag.split("}")[-1] in ("externalModelDefinition", "submodel") for e in root.iter()):
        raise InvalidRequest("External/composed SBML model resolution is unsupported")

    def units(path: str, attribute: str) -> dict[str, str | None]:
        values = {}
        for element in model.findall(path, ns):
            id = element.get("id")
            if not id or id in values:
                raise InvalidRequest("SBML declarations require unique identifiers")
            values[id] = element.get(attribute)
        return values

    parameters = units("s:listOfParameters/s:parameter", "units")
    species = units("s:listOfSpecies/s:species", "substanceUnits")
    compartments = units("s:listOfCompartments/s:compartment", "units")
    if not species:
        raise InvalidRequest("SBML model has no species")
    warnings = [
        "Original SBML unit identifiers retained; no kinetic/tissue mapping inferred.",
        "Native solver validation/execution is separate from XML admission.",
    ]
    if any(
        value is None for value in (*parameters.values(), *species.values(), *compartments.values())
    ):
        warnings.append("Some unit attributes are absent; no default unit is silently assigned.")
    return NativeModelInspection(
        root=name,
        namespace=namespace,
        model_time_unit=model.get("timeUnits"),
        parameter_units=parameters,
        species_units=species,
        compartment_units=compartments,
        warnings=tuple(warnings),
    )


class NativeModelService:
    def __init__(self, db: Database, artifacts: ArtifactStore) -> None:
        self.db, self.artifacts = db, artifacts

    def admit(self, spec: NativeModelSpec, raw: bytes) -> NativeModelPackage:
        inspection = inspect_xml(spec.format, raw)
        package = NativeModelPackage(
            spec=spec, inspection=inspection, content_digest=self.artifacts.put(raw)
        )
        self.db.register("native_model", spec.id, spec.version, package.model_dump_json())
        return package

    def get(self, reference: Reference) -> NativeModelPackage:
        package = NativeModelPackage.model_validate_json(
            self.db.object("native_model", reference.id, reference.version)
        )
        self.artifacts.get(package.content_digest)
        return package

    def discover(self, limit: int = 200, offset: int = 0) -> tuple[NativeModelPackage, ...]:
        return tuple(
            self.get(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("native_model", limit=limit, offset=offset)
        )

    def register_profile(self, profile: NativeExecutionProfile) -> str:
        runtime = NativeRuntimeService(self.db, self.artifacts).get(profile.runtime)
        expected = {"copasi_sbml": "copasi", "osp_pkml": "ospsuite"}[profile.kind]
        if runtime.engine != expected:
            raise InvalidRequest("Execution profile and retained runtime engine differ")
        return self.db.register(
            "native_profile", profile.id, profile.version, profile.model_dump_json()
        )

    def profile(self, reference: Reference) -> NativeExecutionProfile:
        profile = NativeExecutionProfile.model_validate_json(
            self.db.object("native_profile", reference.id, reference.version)
        )
        NativeRuntimeService(self.db, self.artifacts).get(profile.runtime)
        return profile

    def profiles(self) -> tuple[NativeExecutionProfile, ...]:
        return tuple(
            self.profile(Reference(id=r["id"], version=r["version"]))
            for r in self.db.objects("native_profile", limit=200)
        )
