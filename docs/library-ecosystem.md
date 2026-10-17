# Managed source libraries

Peykar now keeps source inspection separate from runtime installation, adapter
capability and scientific validity. A clone alone enables none of the latter.
Nine exact source snapshots from the original architecture have been acquired and
inspected in addition to the separately preserved vCornea Git/archive admission.

| Library | Pin | Original license evidence | Source state |
|---|---|---|---|
| CompuCell3D | `f401d6d087414b86e21d78b9bcf9410123c05245` | `CompuCell3D/License.txt`, MIT core; dependencies separate | Inspected; separate 4.7.0 runtime already works |
| PhysiCell | 1.14.2, `dbd3499250141b27600e91e501c54c46f68f2763` | `licenses/PhysiCell.txt`, BioFVM BSD-3-Clause and bundled notices | Inspected; runtime adapter open |
| VCell | 8.2.1.01, `b6a707a7ea6088358e10b88d68a0f8d6195e9741` | Original-code MIT scope in `license.txt`; components separate | Inspected; native solvers/build open |
| COPASI | Build-309, `59c6c01dc3a8fffee7482d1f620ee393cbff8027` | `license.txt`, Artistic-2.0 | Inspected; runtime adapter open |
| Chaste | 2026.1, `99f8a5061faf856d338440851ffb4b9f8fbd30d9` | BSD-3-Clause `LICENSE`; component terms separate | Inspected; runtime adapter open |
| NEURON | 9.0.2, `ba5783378c0aa1d9ac95b4454bea94b15c63f75d` | `Copyright`, BSD-3-Clause scope plus file notices | Inspected; 13 Git links unresolved |
| PK-Sim | v12.3.173, `b8f13cc6b2317488e9fa3a969701aef4b2132e5c` | Bundled OSP PDF: GPLv2 plus Clarifying Addendum | Inspected; 5 Git links unresolved |
| MoBi | v12.3.136, `7f1094cb5b858d001afa5f2fff1c0b6509a345c5` | Bundled OSP PDF: GPLv2 plus Clarifying Addendum | Inspected; 6 Git links unresolved |
| GEARS | `f374e43e197b295016d80395d7a54ddb81cc6769` | MIT `LICENSE` | Inspected; ML runtime/dataset/adapter open |

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

Acquisition is an explicit operator command, never triggered by the API. It fetches
only exact commits over HTTPS, disables hooks, does not initialize submodules,
install dependencies or execute upstream code, and preserves conflicting existing
checkouts. These are complete tracked top-level working trees from depth-one Git
fetches, not complete upstream histories or recursively installed environments.

The nine snapshots contain **36,291 tracked entries / 1,308,090,422 working-tree
bytes** excluding Git metadata and missing submodule content. **24 Git links** are
recorded; no LFS pointer was found in these pinned top-level trees. Separate native
solver repositories mentioned by an upstream README are not silently treated as
vendored or installed. Current source files remain pristine.

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
