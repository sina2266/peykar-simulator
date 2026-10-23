# Corneal publication reconstruction

The immediate target is the two configurations provided by the authors: tissue
formation from stem cells, then chemical injury/recovery. The injury configuration
has slight, mild and moderate severities. Reconstructed formation now passes its
full-horizon subset; injury execution and the full frozen gate remain pending. See the
[operator instructions](../../../runtime_profiles/vcornea-publication/README.md).

Original authors: Joel Vanin, Michael Getz, Catherine Mahony, Thomas B. Knudsen
and James A. Glazier. Final paper:
[PLOS Computational Biology, 2025, e1013410](https://doi.org/10.1371/journal.pcbi.1013410).
Archived code/data: [Zenodo 16884414](https://doi.org/10.5281/zenodo.16884414),
dated before the final article. Both declare CC-BY-4.0. Retain attribution and
[original terms](https://creativecommons.org/licenses/by/4.0/legalcode.en).

## Original material actually recovered

| Archived group selected by the original plotting script | Replicates | Full raw horizon |
|---|---:|---:|
| `STEM_ONLY_proper_thickness_6months/Comb0` | 21 | 4,320 hours / 43,200 MCS |
| `(Slight) 750_G/Comb0` | 13 | 5,040 hours / 50,400 MCS |
| `(Mild) 1500_G/Comb1` | 16 | 5,040 hours / 50,400 MCS |
| `(Moderate) 2500_G/Comb1` | 15 | 5,040 hours / 50,400 MCS |

All 174 original scalar members (counts, binned height and supplied chemical
tracking) and two original analysis files were acquired with ZIP CRC, byte size
and local SHA-256 checks. The complete 6.75 GB ZIP's MD5 remains unverified;
selected-member verification is the declared scope. Raw members remain under
`.peykar/external/vcornea-publication-cohorts-16884414/`, with a portable
[176-member catalog](source-catalog-1.0.0.json) for reacquisition.

`upstream/plot_manager.py` and `upstream/membrane_height.csv` are **unchanged
original bytes**. Their math was actually executed headlessly on all 65 published
replicates, producing five original count/thickness figures and four full-horizon
count-mean CSVs. Exact outputs and replicate summaries are in
[the original analysis receipt](original-analysis-2026-10-18.json); local rendered
files are in `.peykar/cornea-original-analysis-output/native-replay-1/`.
Only the display hook is replaced by PNG/PDF export. Original count plots stop
at 4,320 hours; the additional numeric export retains the full injury horizon.
Q-Q plots and hourly samples do not establish independent biological replication.

All 17 final-paper supplementary DOCX files are retained unchanged under
`paper-supplements/`, with publisher URLs, sizes and hashes in the same catalog.
The [71-entry source audit](source-audit-1.0.0.json) reads table values and source
ASTs without importing downloaded model code.

## Source conflicts and declared reconstruction choices

| Control | Archived Local | Archived HPC | Final paper/tables | Current primary replay |
|---|---|---|---|---|
| Engine | Current project used 4.7.0 | Original complete environment not archived | Article §2.1 says 4.6.0 | Official macOS ARM64 4.6.0 revision2, commit label `397f8b1`; dependencies reconstructed |
| Formation basal target volume / EGF half-max / superficial EGF diffusion | 25 / 7 / 20 | **40 / 8 / 12**, in the batch script named like the archived cohort | S8/S9 give 25 / 7 / 20 | Separate `hpc-2.0.0` profile restores the published batch defaults; Local history retained |
| Formation duration | 50,400 MCS | 50,400 MCS | Six-month analysis | 43,200 MCS, matching every selected original raw series |
| Injury time / duration | Convenience 500 / 7,700 MCS | Convenience 500 / 7,700 MCS | S10 and source comments give 7,200 / 50,400 MCS | 7,200 / 50,400 MCS |
| Chemical source Y | 65 lattice sites | 65 lattice sites | S10 says 75 | 65 to reconstruct the archived executable; conflict remains explicit |
| Injury selector | `False` selects chemical exposure in actual code | Same | S11 description reverses the Boolean meaning | Follow executable semantics; never switch to ablation from that description |
| SLS threshold | Numeric 2.0 | Numeric 2.0 | S11 lists Boolean `True` | Preserve the numeric source threshold |
| Injury initial state | `Epithelium.piff` | Same | Per-run state/seeds absent | Original 46 superficial /127 wing /32 basal /4 stem cells |

The archive has no per-run seed/parameter snapshots. Seeds **7 and 11 are new
explicit seeds**, not recovered historical seeds. The HPC preset is a supported
source-based reconstruction, not proof that it generated every old replicate.
Final-table and archived executable configurations must not be silently conflated.
No parameter was fitted to the new candidate outputs and the frozen tolerance
file has not been changed after execution began.

## Comparison and biological interpretation

[Comparison 1.0.0](comparison-1.0.0.json) was frozen before full candidate runs.
It pins all 130 original count/height artifacts, hourly reference means/sample SDs,
between-replicate stable means and ten regional thickness estimates. Count means
use a 3-SD envelope plus one-cell quantization, hourly trajectories require 90%
coverage in the corresponding 3-SD envelope plus one cell, and height-derived
means use 3 SD plus 2 µm. These are explicit engineering variability checks,
not clinical acceptance, confidence intervals or statistical equivalence tests.
Each seed is assessed separately; an ensemble mean cannot hide a failed seed.

Original raw `Time` is already hours (10 MCS/hour). The article §2.2 maps each
lattice site to 2 µm and describes a 200×90 cross-section. Thickness comparison
uses the original `(mean top-cell Y - original regional membrane Y) * 2` formula.
That observation is distinct from a clinical OCT segmentation or whole-eye area.
Additional lattice/EGF images and four-connected fluid access to the original
basal surface are descriptive measurements, without new acceptance thresholds.
Population-count return is reported separately from structural re-epithelialization.

The article's S12 explicitly says turnover was used for calibration and injury
closure for parameter screening. Its thickness/recovery ranges cannot become an
independent holdout by being copied into Peykar. The previously admitted Rho2015
scratch monolayer assay has a different context. Independent tissue/clinical
validation remains a separate, unclosed gate.

Peykar's lifetime repairs and observation wrappers are separate derived work;
original source and analysis files remain unchanged. The two method excerpts in
`tests/fixtures/vcornea-*.py.txt` retain the same upstream attribution/CC-BY scope.

## Observed continuation, 2026-10-23

The two later-source `hpc-2.0.0` formation runs actually completed at43,200MCS
with captured exit0. Their [unchanged frozen comparison](observed-2026-10-23/formation-hpc2-comparison.md)
is negative: both pass2/4 stable count means and only1/10 regional thickness
checks. Seed7 passes3/4 hourly trajectories; seed11 passes4/4. Superficial counts
are47.59/48.01 versus reference50.12; basal counts27.62/28.19 versus30.88.
The [numeric report](observed-2026-10-23/formation-hpc2-comparison.json) retains
each seed and failure. This diagnostic subset does not close the full publication gate.

The old outer supervisor ended without capturing four injury exits. Those jobs
have complete-looking output files but remain **unverified**, retained in the
[interrupted inventory](interrupted-output-inventory-2026-10-23.json). The new
`.peykar/cornea-hpc46-resumed-20261023` series reuses only the two genuine completed
formation runs, retries those four jobs and starts the two previously unstarted
moderate jobs. Every fresh job copies a pinned guard that independently records
its actual child exit/source/runtime/output hashes; files alone can never imply success.
All three follow-on analyses are explicitly scheduled in
`.peykar/cornea-hpc46-analysis-20261023`, without changing acceptance criteria.

Original DML metadata identifies July15/16 **2024** output directories. The complete
preceding July14 source commit `b6123adf992e5b48442c0ebedd34ba7cd3162ef2` was
recovered directly from the original licensed code ZIP's embedded Git objects.
All50 files are now pristine permanent source in
[`vendor/models/vcornea-history-b6123adf`](../../../vendor/models/vcornea-history-b6123adf),
with [original archive/blob evidence](../../../engine_adapters/compucell3d/vcornea-history-b6123adf.json).
Its full HPC project,95 literal batch controls, Temperature10 and EGF diffusion10
are a separate `historical-hpc-3.0.0` candidate; later source uses8/12 for the latter
two controls. Native lifetime repair remains explicit. The original height formula
is observed on ten bins instead of five to match the existing frozen measurement
grid; two actual20MCS runs with five/ten bins had identical counts, cells and fields.
The [AST-normalized mechanism comparison](observed-2026-10-23/mechanism-source-diff.txt)
also shows changed total-versus-volume-normalized EGF/SLS sensing, stem growth
multiplier2→1, superficial sloughing probability and tear-volume replacement.
This is why changing only three parameters in later mechanics cannot represent
the recovered historical model. Original formation XML requests four CC3D workers;
BLAS/OpenMP environment1 is not a native CPU cap or bitwise stochastic guarantee.
Full formation seeds7/11 completed in
`.peykar/cornea-historical-formation46-20261023`, with successful diagnostic-only follow-on in
`.peykar/cornea-historical-formation-analysis-20261023` (details below). No parameters were fitted
to the failed results. Exact historical per-run source/seed identity remains unknown.

Historical injury is not silently mixed into this candidate: its initial PIFF has
3/35/132/39 stem/basal/wing/superficial cells, while retained injury data starts
4/32/127/46. That source/state correspondence must be resolved before an old-source
injury execution can be called a recovered scenario.

Ten original VTK snapshots and three descriptors were acquired and actually read,
with [a separate catalog](spatial-source-catalog-1.0.0.json),
[numeric field inspection](observed-2026-10-23/original-spatial-inspection.json) and
[an actual injury image](observed-2026-10-23/original-mild-comb0-324h.png).
MildComb0 is a separate4,620h group with chemical pulse300h; selected MildComb1
uses5,040h/pulse720h. Moderate spatial files end1,058h (DML identifies Comb3/2),
and CONTROL images end4,265h (Comb0/9). None establishes a spatial identity for the
four selected complete scalar cohorts. These are original simulation observations,
not independent experimental measurements. DML's Version1.0 is its schema version,
not the engine version. The [dated engineering receipt](observed-2026-10-23/engineering.json)
retains actual short-probe results and the declared limitations.

## Observed injury-state reconstruction, version4.0.0

Further [original-state inspection](source-state-correspondence-1.0.0.json) resolves
useful constraints without claiming an exact historical per-run snapshot:

- For both retained injury VTKs atMCS0, every cell ID/type agrees with the separately
  archived injury PIFF (SHA256 `906902426fb315c670f41337a8e91aeef4d32bcea7280f0bf8af9a790509e6c4`).
  Saved pixel shapes differ after a sweep:160/168 type sites and748/730 ID sites.
  The result establishes identity/type correspondence, not exact pre-sweep geometry.
- Conditioned on the lambda values shared by the reviewed sources, each group's
  original pressure/actual-volume observations have a common initial target of
  **40 stem /40 basal /25 wing /25 superficial** lattice sites. The inverse uses
  `abs(2*lambda*(volume-target))`, retaining sign ambiguity until multiple cells
  identify a common target. Lambda itself, original seeds and hidden state are not
  recovered. In particular the later-source basal25 control is inconsistent with
  those spatial observations under its own lambda2.
- Original cell-lineage fields are present, consistent with the historical
  initializer removed from the later source. Original MildComb0 at3000MCS has
  exactly one positive SLS site at **[100,75]**, total1500. Its timing/group stays
  separate from the frozen selected7200MCS cohorts.

`observed-injury-4.0.0` is therefore a **declared derived reconstruction**: complete
historical HPC injury mechanisms/defaults (40/8/10, Temperature10, Y75), the separately
evidenced original PIFF, and explicit frozen7200/50400MCS/dose controls. Original
historical controls and every override remain in its manifest. It is not presented
as executing the untouched old Git tree or recovering original seeds. Version3's
formation recipe and rejection of silently substituted injury states remain intact.

An [actual600MCS moderate-injury probe](observed-2026-10-23/derived-injury-engineering.json)
completed exit0 in130.00s with pulse deliberately at20MCS. It retained the61-hourly-row
count grid,610 ten-bin height rows,601 per-MCS SLS rows, actual pulse[100,75]/2500,
and lattice/cell-volume agreement. Membrane sites990→869, epithelial sites5013→2758.
These are engineering observations, not a full-horizon or biological acceptance.

Six full new-seed7/11 injury jobs are already prepared/frozen in
`.peykar/cornea-observed-injury46-20261023` (supervisor53938). They wait for both
existing native series to genuinely complete before execution, at most four jobs
with original four-worker CC3D XML each. Each job has a24-hour deadline; wait24h.
The explicitly started follow-on is
`.peykar/cornea-reconstructed-combined-analysis-retry1-20261023` (supervisor54075),
with bounded48h wait. It combines those six completed records with the two historical
formation records, retains each original plan/source identity and applies the full
unchanged four-group comparison, original math and spatial analysis. The first
follow-on started before initial plan publication and failed; its log/state remain,
and a targeted regression now verifies the bounded preparation wait.
No full injury-candidate success or independent biological validation is asserted.

## Completed historical formation subset, October23

Both `historical-hpc-3.0.0` jobs genuinely completed43200MCS/180simulated days:
seed7 in5963.56s and seed11 in5962.51s, each with actual exit0 and a durable
completion proof. The [frozen comparison](observed-2026-10-23/formation-historical3-comparison.md)
passes **4/4 count means,4/4 hourly trajectories and10/10 thickness bins for each
seed**. Protocol SHA256 stays1189fd62ece9d90113e6096c51fa4490e6385901b0390e0a8520cb106c40bb43.

| Stable mean, cells | Original21-run mean | New seed7 | New seed11 |
|---|---:|---:|---:|
| Superficial |50.12444|49.80529|49.73462|
| Wing |143.30549|142.66318|145.45485|
| Basal |30.88240|31.02931|30.87726|
| Stem |4.75527|4.83277|4.74797|

The original count/thickness mathematics also completed, producing two figures
and full4321-hourly-row trajectories. Both original figures, a reference-overlay
count figure and both [actual layer figures](observed-2026-10-23/formation-historical3-seed7-structure.png)
were visually inspected. Independent cell volumes/lattice occupancy agree at all
six observed times per seed. The descriptive top-connected fluid-access proxy
has no exposed original basal columns at days15/180 in either seed; this is a2D
simulation observation, not clinical healing. [Execution/source proofs](observed-2026-10-23/formation-historical3-execution.json)
and [spatial numbers](observed-2026-10-23/formation-historical3-spatial.json) are retained.

This is the successful **formation subset**, labelled `diagnostic_passed` by the
operator. The full four-group gate and injury/recovery remain pending. Old failed
later-source results, exact source versions, unknown original seeds/build and
the declared injury reconstruction are preserved. The [additional history audit](source-history-audit-2026-10-23.md)
records original stash/notebook evidence without replacing current model controls.

## Original spatial file completeness and sensing

Correct ZIP/CRC/SHA does not guarantee an internally complete scientific file.
Original `CONTROL/Images/LatticeData/Step_042650.vtk` declares10 fields but ends
inside Pressure after9597/18000 values; three later fields are absent. The
[field inventory](observed-2026-10-23/original-spatial-field-completeness.json)
retains that gap. A checked ASCII reader now accepts only actual complete finite
required fields and reports incomplete optional fields. It never fills missing
data or relies on a native reader's preallocated array length. All10 earlier
cell-type/EGF/SLS image byte hashes are unchanged with this direct reader.

The [original sensing audit](observed-2026-10-23/original-sensing.json) exports
all1218 positive epithelial cells in six original snapshots. Reported EGF divided
by the saved cell-summed field has median approximately1; division by the cell-mean
field has median23–24. All except one cell have relative sum discrepancies at most
2.611e-7. MildComb0/46200MCS stemID8002 differs by1.807958 (relative0.893025).
That actual outlier remains in the [cell-level CSV](observed-2026-10-23/original-egf-by-cell.csv);
steppable timing/inheritance is a possible explanation, not a proven cause. The
finalCONTROL sensing field remains unavailable. These are descriptive source
correspondence observations, with no new candidate acceptance or parameter fitting.

The [experimental context audit](experimental-context-audit-2026-10-23.md) keeps
the original rabbit literature and calibration-use conflict separate from these
simulation observations; no matching raw independent tissue validation is claimed.

## Complete later-source baseline and current derived execution

The [completed baseline audit](completed-baseline-audit-2026-10-23.md) records all
eight genuinely completed later-source jobs. Its unchanged full comparison is
`not_reproduced`: both moderate jobs pass, whereas the other six fail. All six
full injury chemical traces have50401 finite/nonnegative samples and actual
declared pulse sites[100,65]. Original pulse/producer/recovery/structural audits
remain separate from candidate acceptance and biological validation.

Both prerequisite native series have finished. Four `observed-injury-4.0.0` jobs
are running and two moderate jobs wait for capacity; the existing combined
follow-on retains the successful historical formation pair. Do not restart the
completed baseline or duplicate healthy jobs. Full reconstructed acceptance is
still pending.

Some original-analysis exports clipped titles/axis labels. The display wrapper
now fits layouts/bounds without changing the authors' code or math. Replaying
all65 original and8 baseline runs retained identical summaries and all eight
full-horizon mean CSV byte sequences. Corrected figures and exact equivalence
proofs are linked from the completed baseline audit; earlier exports remain intact.
