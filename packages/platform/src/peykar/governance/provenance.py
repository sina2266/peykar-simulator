from typing import Any

from peykar.simulation.service import SimulationService
from peykar_core.hashing import digest


def provenance(service: SimulationService, run_id: str) -> dict[str, Any]:
    """PROV-O vocabulary mapping, not a conformant RO-Crate or SED-ML export."""
    manifest, record = service.db.manifest(run_id), service.db.run(run_id)
    return {
        "@context": {"prov": "http://www.w3.org/ns/prov#"},
        "@id": f"urn:uuid:{run_id}",
        "@type": "prov:Activity",
        "prov:used": [
            {"@id": f"urn:sha256:{manifest.model_digest}", "@type": "prov:Entity"},
            {"@id": f"urn:sha256:{digest(manifest.plan)}", "@type": "prov:Entity"},
        ],
        "prov:generated": (
            [{"@id": f"urn:sha256:{record.result_digest}", "@type": "prov:Entity"}]
            if record.result_digest
            else []
        ),
        "manifest_digest": record.manifest_digest,
        "validity": manifest.model_snapshot.validity.model_dump(mode="json"),
        "events": service.db.events(run_id),
    }
