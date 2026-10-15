# vCornea integration gate

Integration status: **designed, not executed**. The reference population engine in
this repository is not vCornea and must never be presented as a substitute.

## Pinned admission inspection, 2026-10-15

GitHub's branch and complete recursive-tree APIs resolved commit
`f5c03afb9c4c7d909494146c5db608389e957070` and tree
`ed59f4e7b6b9112114f85fc3f55014ef8b48ab73`. The 39-entry tree has no filename
matching license/licence, copying or notice. This is an inspection result, not a
claim that no permissions can exist elsewhere. Archive licensing terms have not
been retrieved, so source use/redistribution permission remains **unverified**.
An article's access license is not assumed to license the software repository.

The exact paths and Git object IDs are recorded in
`../engine_adapters/compucell3d/vcornea-admission-2026-10-15.json`. No upstream
simulation code, archive or data was copied into this release; no engine was run.
A local scalar export importer now accepts an explicit operator-supplied delimiter,
column mapping and physical time per Monte Carlo step, with dedicated immutable
raw/normalized lineage. A frozen seed-paired scalar regression protocol is also
available. See [local output import](compucell-output.md). Native CompuCell3D output
discovery and a verified biological time mapping are still unimplemented; the
included fixture is invented and no upstream scenario has been executed.

## Engineering continuation, 2026-10-16

The [mapped scalar importer](compucell-output.md) now accepts operator-supplied
columns and a documented MCS-to-time mapping; it does not infer the native vCornea
format. Frozen seed-paired scalar comparisons and [offline artifact digest
admission](external-artifacts.md) are available. Neither reproduces the upstream
runtime or establishes source permissions.

A read-only permission recheck on 2026-10-16 received HTTP 404 from GitHub's
repository-license API. The Zenodo record could not be retrieved through the web
tool, and its JSON API timed out during TLS negotiation on this host. This leaves
archive/source permission unverified; it is not evidence of a license prohibition.
The upstream README still identifies Python 3.12 and CompuCell3D 4.7.0. No upstream
model archive was downloaded, installed or executed during this continuation.

## Sources inspected

- Upstream: https://github.com/VaninJoel/vCornea
- Publication: https://doi.org/10.1371/journal.pcbi.1013410
- Permanent archive linked by upstream: https://doi.org/10.5281/zenodo.16764319
- CompuCell3D: https://compucell3d.org/

The upstream README specifies Python 3.12 and CompuCell3D **4.7.0**. It describes
two project variants under `Local/Project/paper_version_STEM_ONLY/` and
`Local/Project/paper_version/`, each with `vCornea_v2.cc3d`. Its injury scenario
changes `SLS_Concentration` in the scenario's `Simulation/Parameters.py` to 750.0,
1500.0 or 2500.0. These are upstream reference settings, not a general dose-response
or treatment recommendation. Verify the pinned source before mapping each value.

The upstream model is a two-dimensional agent-based representation with EGF
reaction-diffusion dynamics. Do not describe it as a complete eye or whole-cornea
digital twin. Upstream documentation is not independent validation of our adapter.

## Admission workflow

1. Verify redistribution/use license for source, datasets and engine separately.
   License permission is not established merely by a public repository. No external
   source or reference dataset is redistributed in this release.
2. Resolve an immutable commit or archived artifact and record its SHA-256. Do not
   invent a revision or use a moving branch as the reproducibility reference.
3. Reconstruct the documented 4.7.0 environment in a separate environment/container.
   Verify compatibility before considering an engine version upgrade.
4. Run the original published scenario without changing scientific code. Record
   environment, input files, seed behavior, engine command and raw output checksums.
5. Inspect actual raw output schema. Implement an importer with units, time-step
   mapping, biological context and explicit observation operators. Do not guess
   internal field names or map Monte Carlo steps to seconds without justification.
6. Run repeated seeds. Freeze reference distributions, endpoint definitions,
   tolerances and an acceptance protocol before testing an adapter change.
7. Compare imported results and the original analysis workflow. Preserve the raw
   outputs, normalized artifacts and transformation lineage independently.
8. Add independent experimental data only when its context, endpoint and license
   support the intended question. Record unavailable data as a KnowledgeGap.

## Engine adapter requirements

CompuCell3D execution must live outside the management API process. The execution
adapter provides capability discovery, validated inputs, cost/resource estimation,
submission, status, artifacts and cancellation. Native snapshots/restarts must be
declared as supported or unsupported, not inferred. Preserve coordinate systems,
lattice spacing, stochastic seeds, time mapping and parameter ownership.

An initial adapter should reproduce the untouched model first. Model modifications
create a separate package version and an independent scientific evaluation task.
Only after that gate should a molecular coupling model be added.
