"""Read terminal physical quantities after frozen child/result identity checks."""

import csv
import io
import json

from peykar.simulation.native_jobs import NativeJobService
from peykar.simulation.service import SimulationService
from peykar_core.contracts import NativeRunManifest, Quantity, RunManifest
from peykar_core.errors import Conflict
from peykar_core.hashing import digest


def terminal_values(
    service: SimulationService, template: RunManifest | NativeRunManifest, run_id: str
) -> dict[str, Quantity]:
    native = NativeJobService(service)
    if isinstance(template, RunManifest):
        actual = service.db.manifest(run_id)
        if (
            actual.model_copy(update={"run_id": template.run_id, "created_at": template.created_at})
            != template
        ):
            raise Conflict("Batch child manifest differs from its frozen template")
        result = service.result(run_id)
        if result.seed != template.plan.seed:
            raise Conflict("Batch child seed differs from its frozen allocation")
        return {s.name: Quantity(value=s.values[-1], unit=s.unit) for s in result.outputs}
    actual_native = native.manifest(run_id)
    if (
        actual_native.model_copy(
            update={"run_id": template.run_id, "created_at": template.created_at}
        )
        != template
    ):
        raise Conflict("Native batch child differs from its frozen template")
    native_result = native.result(run_id)
    if native_result.manifest_digest != digest(actual_native):
        raise Conflict("Native result manifest differs")
    blobs = {n: service.artifacts.get(h) for n, h in native_result.artifacts.items()}
    try:
        metadata = json.loads(blobs["quantities.json"])
        rows = list(csv.DictReader(io.StringIO(blobs["native.csv"].decode("utf-8-sig"))))
        if template.profile_snapshot.kind == "copasi_sbml":
            units = {s["name"]: s["unit"] for s in metadata["species"]}
        else:
            units = {n: q["unit"] for n, q in metadata["quantities"].items()}
        return {
            n: Quantity(value=float(value), unit=units[n])
            for n, value in rows[-1].items()
            if n not in ("Time", "IndividualId")
        }
    except (ValueError, TypeError, KeyError, IndexError) as error:
        raise Conflict("Native terminal outputs lack unambiguous quantity units") from error
