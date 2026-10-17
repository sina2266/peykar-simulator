"""Explicit native ports and immutable SBML initial-concentration derivation."""

import json
import re
import xml.etree.ElementTree as ET
from xml.parsers import expat

from peykar.model_import.native_models import NativeModelService, inspect_xml
from peykar.simulation.service import SimulationService
from peykar_core.contracts import (
    FlowTransfer,
    NativeDerivationReceipt,
    NativeInterfaceSpec,
    NativeModelPackage,
    NativeModelPort,
    NativeModelSpec,
    Quantity,
    Reference,
)
from peykar_core.errors import Conflict, InvalidRequest
from peykar_core.hashing import digest
from peykar_core.units import convert, validate_unit


def replace_initial_values(raw: bytes, changes: dict[str, str]) -> bytes:
    """Replace only admitted numeric attribute spans; preserve every other byte."""
    namespace = ET.fromstring(raw).tag[1:].split("}")[0] + "}"
    parser = expat.ParserCreate(namespace_separator="}")
    path: list[str] = []
    replacements: list[tuple[int, int, bytes]] = []

    def start(name: str, attributes: dict[str, str]) -> None:
        path.append(name)
        id = attributes.get("id", "")
        if (
            path != [namespace + n for n in ("sbml", "model", "listOfSpecies", "species")]
            or id not in changes
        ):
            return
        begin = parser.CurrentByteIndex
        end, quote = begin, 0
        while end < len(raw):
            char = raw[end]
            if quote:
                if char == quote:
                    quote = 0
            elif char in (34, 39):
                quote = char
            elif char == 62:
                break
            end += 1
        match = re.search(
            rb"""(?<![\w:.-])initialConcentration\s*=\s*(["'])(.*?)\1""", raw[begin:end], re.DOTALL
        )
        if match is None:
            raise InvalidRequest("Initial concentration attribute is absent")
        replacements.append(
            (begin + match.start(2), begin + match.end(2), changes[id].encode("ascii"))
        )

    parser.StartElementHandler = start

    def end_element(name: str) -> None:
        path.pop()

    parser.EndElementHandler = end_element
    parser.Parse(raw, True)
    if len(replacements) != len(changes):
        raise InvalidRequest("Each target must identify exactly one core SBML species")
    for begin, end, value in sorted(replacements, reverse=True):
        raw = raw[:begin] + value + raw[end:]
    return raw


def concentration_species(raw: bytes, species_id: str) -> tuple[ET.Element, ET.Element]:
    inspection = inspect_xml("sbml", raw)
    if "/level3/" not in inspection.namespace:
        raise InvalidRequest(
            "Initial concentration derivation currently supports explicit SBML L3 units"
        )
    root = ET.fromstring(raw)
    ns = {"s": inspection.namespace}
    model = root.find("s:model", ns)
    if model is None:
        raise InvalidRequest("SBML model is absent")
    species = model.find(f"s:listOfSpecies/s:species[@id='{species_id}']", ns)
    if species is None:
        raise InvalidRequest("Unknown SBML species")
    compartment_id = species.get("compartment")
    compartment = next(
        (
            c
            for c in model.findall("s:listOfCompartments/s:compartment", ns)
            if c.get("id") == compartment_id
        ),
        None,
    )
    if (
        compartment is None
        or compartment.get("spatialDimensions") != "3"
        or compartment.get("constant") not in ("true", "1")
        or (compartment.get("units") or model.get("volumeUnits")) != "litre"
        or (species.get("substanceUnits") or model.get("substanceUnits")) != "mole"
        or species.get("hasOnlySubstanceUnits") not in ("false", "0")
    ):
        raise InvalidRequest("Require explicit molar concentration in a fixed 3D litre compartment")
    return root, species


def input_species(raw: bytes, species_id: str) -> tuple[ET.Element, ET.Element]:
    root, species = concentration_species(raw, species_id)
    if (
        "initialConcentration" not in species.attrib
        or "initialAmount" in species.attrib
        or species.get("constant") not in ("false", "0")
        or species.get("boundaryCondition") not in ("false", "0")
        or any(
            (e.tag.split("}")[-1] == "initialAssignment" and e.get("symbol") == species_id)
            or (
                e.tag.split("}")[-1] in ("assignmentRule", "eventAssignment")
                and e.get("variable") == species_id
            )
            for e in root.iter()
        )
    ):
        raise InvalidRequest(
            "Input must be an explicit dynamic initial concentration without overriding assignments"
        )
    return root, species


class NativeInterfaceService:
    def __init__(self, service: SimulationService) -> None:
        self.service = service
        self.models = NativeModelService(service.db, service.artifacts)

    def register(self, spec: NativeInterfaceSpec) -> str:
        model = self.models.get(spec.model)
        names = [p.port.name for p in spec.ports]
        targets = [p.species_id for p in spec.ports if p.kind == "sbml_initial_concentration"]
        if len(set(names)) != len(names) or len(set(targets)) != len(targets):
            raise InvalidRequest("Native interface port names/input species must be unique")
        if any(not e.strip() or len(e) > 2000 for e in (*spec.evidence, *spec.limitations)):
            raise InvalidRequest(
                "Native interface evidence/limitations require bounded nonblank text"
            )
        raw = self.service.artifacts.get(model.content_digest)
        for p in spec.ports:
            validate_unit(p.port.unit)
            if p.kind == "sbml_initial_concentration":
                if model.spec.format != "sbml":
                    raise InvalidRequest("PKML input rewriting is unsupported")
                input_species(raw, str(p.species_id))
                convert(Quantity(value=1, unit=p.port.unit), "mole/liter")
            elif model.spec.format == "sbml":
                if not p.species_id:
                    raise InvalidRequest("COPASI output ports require the original SBML species ID")
                concentration_species(raw, p.species_id)
                convert(Quantity(value=1, unit=p.port.unit), "mole/liter")
            elif p.species_id:
                raise InvalidRequest("PKML outputs use native quantity paths, not SBML species")
        return self.service.db.register(
            "native_interface", spec.id, spec.version, spec.model_dump_json()
        )

    def get(self, ref: Reference) -> NativeInterfaceSpec:
        value = NativeInterfaceSpec.model_validate_json(
            self.service.db.object("native_interface", ref.id, ref.version)
        )
        self.models.get(value.model)
        return value

    def read(self, port: NativeModelPort, run_id: str) -> Quantity:
        from peykar.simulation.native_jobs import NativeJobService
        from peykar.simulation.terminal_values import terminal_values

        jobs = NativeJobService(self.service)
        manifest = jobs.manifest(run_id)
        result = jobs.result(run_id)
        metadata = json.loads(self.service.artifacts.get(result.artifacts["quantities.json"]))
        if manifest.profile_snapshot.kind == "copasi_sbml":
            matches = [
                s
                for s in metadata.get("species", [])
                if s.get("sbml_id") == port.species_id and s.get("name") == port.quantity_key
            ]
            if len(matches) != 1:
                raise Conflict("Native CSV column does not identify the declared SBML species")
        values = terminal_values(self.service, manifest, run_id)
        if port.quantity_key not in values:
            raise Conflict("Native result lacks its declared output quantity")
        value = values[str(port.quantity_key)]
        return Quantity(value=convert(value, port.port.unit), unit=port.port.unit)

    def derive(
        self,
        parent: NativeModelPackage,
        interface: NativeInterfaceSpec,
        transfers: tuple[FlowTransfer, ...],
    ) -> tuple[NativeModelPackage, NativeDerivationReceipt, bytes]:
        raw = self.service.artifacts.get(parent.content_digest)
        if parent.spec.format != "sbml" or interface.model != Reference(
            id=parent.spec.id, version=parent.spec.version
        ):
            raise InvalidRequest("Derivation requires the exact admitted SBML parent/interface")
        changes: dict[str, str] = {}
        for transfer in transfers:
            port = next((p for p in interface.ports if p.port.name == transfer.target_port), None)
            if (
                port is None
                or port.kind != "sbml_initial_concentration"
                or port.species_id != transfer.target_selector
            ):
                raise InvalidRequest(
                    "Transfer does not identify a declared SBML initial concentration"
                )
            input_species(raw, str(port.species_id))
            value = convert(transfer.target_quantity, "mole/liter")
            if value < 0:
                raise InvalidRequest("Negative native initial concentrations are unsupported")
            if str(port.species_id) in changes:
                raise InvalidRequest("Multiple writers target the same SBML species")
            changes[str(port.species_id)] = format(value, ".17g")
        child_raw = replace_initial_values(raw, changes)
        child_digest = self.service.artifacts.put(child_raw)
        receipt = NativeDerivationReceipt(
            parent=parent,
            interface=interface,
            transfers=transfers,
            child_content_digest=child_digest,
        )
        checksum = digest(receipt)
        spec = NativeModelSpec(
            id="derived." + checksum,
            version="1.0.0",
            title="Derived SBML initial concentrations",
            format="sbml",
            context=parent.spec.context,
            license=parent.spec.license,
            source=(
                f"Derivation sha256:{checksum}; original attribution in "
                f"parent {parent.spec.id}@{parent.spec.version} and receipt."
            ),
            limitations=parent.spec.limitations,
        )
        package = NativeModelPackage(
            spec=spec, content_digest=child_digest, inspection=inspect_xml("sbml", child_raw)
        )
        return package, receipt, child_raw
