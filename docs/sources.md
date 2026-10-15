# Research and source register

Accessed 2026-10-15. These primary sources informed design decisions. They do not
certify Peykar's implementation or establish biological validity. The delivered
`uv.lock`, not a documentation site's latest label, defines installed Python versions.

| Area | Primary source | Applied decision |
|---|---|---|
| Modular Python workspace | https://docs.astral.sh/uv/concepts/projects/workspaces/ | Three distributions, shared lock and exact sync |
| Reproducible dependencies | https://docs.astral.sh/uv/concepts/projects/layout/ | Commit lockfile; use frozen CI installs |
| Contract validation | https://docs.pydantic.dev/latest/concepts/strict_mode/ | Reject unknown fields, finite values, strict seed/budget types |
| App structure | https://fastapi.tiangolo.com/tutorial/bigger-applications/ | Transport routers and application service boundaries |
| Lifecycle | https://fastapi.tiangolo.com/advanced/events/ | Application factory and lifespan initialization |
| Heavy computation | https://fastapi.tiangolo.com/tutorial/background-tasks/ | Separate worker; no simulation inside FastAPI BackgroundTasks |
| Plugins | https://docs.python.org/3.12/library/importlib.metadata.html | Installed entry points with explicit allowlist |
| Durable local storage | https://docs.python.org/3.12/library/sqlite3.html | Transactions, independent connections and explicit schema version |
| Units | https://pint.readthedocs.io/en/stable/user/defining-quantities.html | Unit conversion separate from semantics |
| Numerical integration | https://docs.scipy.org/doc/scipy/reference/generated/scipy.integrate.solve_ivp.html | Future adaptive ODE engine needs solver-specific event/error contracts |
| Model exchange | https://sbml.org/documents/specifications/ | Future SBML importer; do not claim all model types fit SBML |
| Experiment description | https://sed-ml.org/ | Keep model and experiment descriptions distinct; future exchange mapping |
| Co-simulation | https://fmi-standard.org/docs/3.0.2/ | Explicit step/state/event capabilities, no implicit feedback assumptions |
| Provenance | https://www.w3.org/TR/prov-o/ | Activity/entity relations in provenance export |
| Research artifacts | https://www.researchobject.org/ro-crate/specification/1.1/ | Future packaged provenance export; no conformance claim today |
| Imaging data | https://ngff.openmicroscopy.org/latest/ | Planned OME-Zarr representation; no large imaging importer today |
| First scientific model | https://github.com/VaninJoel/vCornea | Separate untouched reference reproduction from candidate modification |
| vCornea publication | https://doi.org/10.1371/journal.pcbi.1013410 | Defined tissue-model scope; no complete-eye claims |

No source code from specialist engines was copied. Citation of a standard does not
mean the current JSON contracts implement that standard. Schema/export adapters
must undergo their own interoperability tests before any conformance statement.

## Licensing sources for 0.2.0 and 0.3.0

The licensing decision and consulted legal primary texts are recorded separately
in licensing-rationale.md. The custom Peykar terms are not an official license
from any of those organizations. Original project licensing, external software
licensing, scientific validity and copyright ownership are separate questions.
