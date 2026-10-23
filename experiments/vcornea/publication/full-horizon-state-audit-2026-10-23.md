# Full-horizon figures and original initial-state identifiability

Spatial diagnostics1.1.0 now shows the actual initial injury state, early course
and final180-day post-pulse structure. EGF panels also include the final4320-hour
post-pulse field. Version1.0 figures stopped at20 days even though the complete
final snapshot was already checked and retained in their numeric JSON. No native
inputs, observation formula, comparison criterion or simulation are changed.
Missing final snapshots now fail figure preparation explicitly.

An actual read-only replay on the completed eight-job source2 baseline passed all
cell/count/lattice checks. Every numerical snapshot row is identical to the
earlier report; only display selection and explicit figure-time metadata differ.
[Report](observed-2026-10-23/baseline-hpc2-full-horizon-spatial.json),
[equivalence receipt](observed-2026-10-23/baseline-hpc2-full-horizon-equivalence.json),
[moderate seed11 structure](observed-2026-10-23/baseline-hpc2-full-horizon-moderate-injury-11-structure.png)
and [EGF field](observed-2026-10-23/baseline-hpc2-full-horizon-moderate-injury-11-egf.png)
retain the actual observations. Both moderate structure/field exports were
visually inspected. The final descriptive basal-access width is0µm/4µm for
seeds7/11; membrane sites842/784 versus990 initially. These are2D observations,
not clinical closure, barrier permeability or a new acceptance criterion.
The complete source2 comparison remains negative.

## What the original pressure fields can identify

The earlier immutable source-state report conditioned on known source lambdas.
A separate exact inverse now uses all418 original initial cell observations
from MildComb0 and the separately saved moderate group. It solves
`absolute_pressure = abs(2 * lambda * (volume - target))` for both unknowns.
All possible sign branches of two distinct volume observations are considered;
every candidate must satisfy every remaining cell with exact rational arithmetic.
No source lambda/target numeric value, candidate output or fitted tolerance is
supplied. Under the stated common-parameter model, each of the eight type/group
systems has one solution:

| Original cell type | Initial target in original lattice-volume units | Lambda in original model units |
|---|---:|---:|
| Stem | 40 | 2 |
| Basal | 40 | 2 |
| Wing | 25 | 2 |
| Superficial | 25 | 5 |

[Original-field inverse report](observed-2026-10-23/original-initial-pressure-identifiability.json)
and [exact executed recipe](observed-2026-10-23/original-initial-pressure-identifiability.py.txt)
retain assumptions, ambiguity/inconsistency self-checks and every group result.
The checked ASCII reader independently re-read the original required fields;
all418 rows equal the earlier conditional report. No outlier was discarded.
The [reviewed4.6.0 source formula](https://raw.githubusercontent.com/CompuCell3D/CompuCell3D/397f8b1/CompuCell3D/core/CompuCell3D/Potts3D/Cell.cpp)
is independently pinned in [the source receipt](observed-2026-10-23/original-pressure-formula-source.json).
The installed getter was also checked on empty native cells at three target/
lambda settings; these native checks cover volume0 only. Its read-only volume
setter rejected the initial attempted probe; no simulation/state was changed.

Uniqueness depends on a shared initial lambda and target per cell type, the
reviewed pressure formula and correspondence between saved pressure and observed
volume. It is not recovered per-run metadata. These spatial groups differ from
the selected full scalar cohorts; this does not identify their controls, engine,
seeds or biological validity. In particular the January candidate's25/25 injury
defaults are not relabeled as these separately observed40/40 initial targets.
No running candidate or immutable earlier state report is changed.

## Bounded original-parameter search

All56 original Git commit trees contain32 unique named `Parameters.py` blobs;
16 occur under output directories. Their stored output horizons are100 or14600MCS,
not the selected43200/50400MCS runs. Literal AST inspection found no top-level
seed assignment in these32 files. The complete78574-entry Data ZIP pathname
index has no parameter/seed/manifest/config/batch-named member. A2023 HPC
`valid_combinations.csv` contains a different fifteen-row exploratory parameter
grid, not the selected2024 cohort controls.
[Search receipt](observed-2026-10-23/original-parameter-history-search.json) and
[executed recipe](observed-2026-10-23/original-parameter-history-search.py.txt)
retain original Git blob identities, source dates and literal numeric controls.
Raw blobs remain unchanged in the local acquisition cache. This is a bounded
pathname/AST audit, not a search of arbitrary unnamed blob contents or evidence
that source code never seeds randomness. No controls are imported or executed.

Official4.6.0 dates differ between the
[download page](https://compucell3d.org/SrcBin) (July6,2024) and
[release notes](https://github.com/CompuCell3D/CompuCell3D/blob/master/ReleaseNotes.rst)
(June8,2024). Both precede the July15/16 data-directory dates; chronology does
not exclude4.6.0 and does not identify the original binary. The running pinned
environment is unchanged. [Exact observed chronology](observed-2026-10-23/original-engine-release-chronology.json)
retains this distinction.
