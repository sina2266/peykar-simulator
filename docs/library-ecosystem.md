# Managed source libraries

Peykar now keeps source inspection separate from runtime installation, adapter
capability and scientific validity. A clone alone enables none of the latter.
All named source snapshots are now permanent files in `vendor/`, alongside
recursive dependencies, separate VCell native solvers and OSP/MaBoSS sources.
See [installed native profiles and actual execution](../runtime_profiles/README.md).

| Library | Pin | Original license evidence | Source state |
|---|---|---|---|
| CompuCell3D | `f401d6d087414b86e21d78b9bcf9410123c05245` | `CompuCell3D/License.txt`, MIT core; dependencies separate | Inspected; separate 4.7.0 runtime already works |
| PhysiCell | 1.14.2, `dbd3499250141b27600e91e501c54c46f68f2763` | `licenses/PhysiCell.txt`, BioFVM BSD-3-Clause and bundled notices | Complete source; native operator profile executes |
| VCell | 8.2.1.01, `b6a707a7ea6088358e10b88d68a0f8d6195e9741` | Original-code MIT scope in `license.txt`; components separate | Complete source/solvers; original OMEX executes |
| COPASI | Build-309, `59c6c01dc3a8fffee7482d1f620ee393cbff8027` | `license.txt`, Artistic-2.0 | Complete source; native operator profile executes |
| Chaste | 2026.1, `99f8a5061faf856d338440851ffb4b9f8fbd30d9` | BSD-3-Clause `LICENSE`; component terms separate | Complete source; native operator profile executes |
| NEURON | 9.0.2, `ba5783378c0aa1d9ac95b4454bea94b15c63f75d` | `Copyright`, BSD-3-Clause scope plus file notices | Complete recursive source; native HH clamp executes |
| PK-Sim | v12.3.173, `b8f13cc6b2317488e9fa3a969701aef4b2132e5c` | Bundled OSP PDF: GPLv2 plus Clarifying Addendum | Complete recursive source; OSP headless PKML executes |
| MoBi | v12.3.136, `7f1094cb5b858d001afa5f2fff1c0b6509a345c5` | Bundled OSP PDF: GPLv2 plus Clarifying Addendum | Complete recursive source; OSP headless PKML executes |
| GEARS | `f374e43e197b295016d80395d7a54ddb81cc6769` | MIT `LICENSE` | Complete source; CPU gradients and real H5AD reader pass; original learned benchmark open |

[Source pins](../engine_adapters/ecosystem-sources.json),
[inspection specs](../engine_adapters/source-specs/), and
[actual receipts](../engine_adapters/ecosystem-source-receipts-2026-10-16.json)
retain identities and license hashes. Existing project licenses are unchanged;
third-party components keep their own terms. This inventory records declared
terms and scope, not a legal determination of distribution compatibility.

## Acquisition and verification

```bash
uv run python scripts/acquire_ecosystem_sources.py \
  --catalog engine_adapters/ecosystem-sources.json \
  --destination .peykar/external/ecosystem/sources --jobs 3
PEYKAR_DATA_DIR=.peykar/ecosystem uv run peykar libraries inspect \
  engine_adapters/source-specs/physicell.json \
  --source .peykar/external/ecosystem/sources/MathCancer--PhysiCell
PEYKAR_DATA_DIR=.peykar/ecosystem uv run peykar libraries list
```

Acquisition is explicit, never triggered by the API. The fetch script retrieves
exact commits without hooks/build execution. `fetch_source_dependencies.py`
separately resolves reviewed Git links, then `vendor_sources.py` verifies source
blobs and materializes ordinary files with original modes/notices. Conflicting
existing destinations are preserved. See the [vendor guide](../vendor/README.md)
for all commands; these tracked source trees exclude upstream Git history.

The current distribution contains **25 source identities / 52,167 files /
1,799,838,581 bytes**, with **37 materialized Git dependencies**, zero unresolved
links and zero LFS pointers. It includes eight separate VCell solver repositories,
MaBoSS and OSP R/helper sources. The earlier 36,291-entry/24-unresolved-link receipt
is retained as historical source version 1.0.0; complete inventories use 2.0.0.
Original source remains pristine; build/execution compatibility changes are separate.

`LibraryService.inspect` verifies HEAD and every tracked blob, executable mode,
symlink target and license file. It never follows a tracked symlink to read an
external file. Only Git-declared text CRLF/LF transformations may differ from a Git
blob; both actual-byte SHA and the original blob identity are retained. MoBi's
actual checkout exposed this case and was admitted without rewriting its bytes.
Untracked files are not part of an inspection receipt. SHA-1 is used only to match
Git's legacy object identity; retained artifacts use SHA-256.

## Offline promotion and discovery

An inspection receipt retains a content-addressed file inventory and exact license
bytes. A trusted operator can move that bundle to another local deployment:

```bash
peykar libraries import-receipt receipt.json --bundle /path/to/digest-named-artifacts
```

The importer checks all inventory/license hashes, tracked counts, path uniqueness
and byte totals before registration. It imports a recorded inspection assertion;
it cannot independently reconstruct missing source blobs from hashes. A changed
immutable source version conflicts instead of overwriting its prior receipt.

Authenticated reads:

- `GET /v1/libraries?capability=pbpk`
- `GET /v1/libraries/{id}/versions/{version}`

There is no HTTP source-path import, automatic Git clone, dependency installation
or executable module loading. Runtime dependencies remain outside core/API.

`NativeRuntimeService` imports actual operator execution evidence with source
references and image/output hashes. `/v1/runtimes` lists 11 retained checks;
it does not certify installation on another host or enable remote execution.

## Improvement and interaction policy

Preserve upstream snapshots; put changes in a separately versioned patch/adapter
with a pinned base and benchmark objective. Optimize only against measured costs
and frozen numerical/scientific acceptance. A model mechanism change requires new
scientific evaluation, while a build/performance patch still needs regression.

COPASI/VCell operate at different model/solver boundaries from PhysiCell/CC3D,
NEURON and OSP. A connector must own units, compartments/geometry, time, state and
process semantics. GEARS' inspected upstream README explicitly restricts cell-type
transfer; transcriptional predictions do not supply tissue kinetics. See the
[whole-platform roadmap](roadmap.md) and [workflow contract](experiment-workflows.md).
