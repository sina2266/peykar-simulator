# P1 integration candidates and reference evidence

Assessment date: 2026-10-16. This is a bounded admission assessment for reproducing
vCornea, not a catalogue of installed or scientifically validated models. Exact
revisions, byte counts, licenses, evidence hashes and decisions are recorded in
[`engine_adapters/candidates.json`](../engine_adapters/candidates.json). Downloaded
metadata and the small supporting files are cached in `.peykar/external/research/`.
They are separate from the live simulation/dataset registries. No candidate model
was installed or executed by this assessment.

## Selected reference: archived vCornea simulation results

The paper cites the concept DOI `10.5281/zenodo.16764318`. Its currently latest
immutable version is **16884414**, published 2025-08-15 with open access and a
record-level **CC-BY-4.0** declaration. Pin this exact record rather than following
`latest` during replay. The earlier README-linked version 16764319 is superseded.
The data archive is identical across the two versions; the code archive changed.
These observations come from the [exact record API](https://zenodo.org/api/records/16884414)
and the [version history](https://zenodo.org/api/records/16764319/versions).

| File in record 16884414 | Bytes | Publisher MD5 |
|---|---:|---|
| `vCornea.zip` | 228,842,524 | `3be635253cd2ab8029ebddbb5e0f8ad8` |
| `Data.zip` | 6,754,563,353 | `43ea77b42fb6d16ac1d1d83f69717ab9` |

Compute a local SHA-256 after downloading and preserve the upstream MD5 separately;
a remote MD5 declaration does not replace a locally verified SHA-256. This
assessment verified the existing complete code archive without downloading it again;
its SHA-256 is `855245e200cc322371b133a8c9e9fc0e81310ab7f3603ebf49afc4afcaf718dc`.
The data reference was acquired as six complete ZIP members using HTTP Range,
without requiring the entire 6.75 GB archive. The archive
license is evidence for the record's files. It does not silently add a standalone
license to every GitHub revision or override third-party dependency notices.

The upstream [README](https://github.com/VaninJoel/vCornea) identifies `Data.zip`
as raw **simulation** output supporting the publication. Its recommended role is
`benchmark` with origin `simulation`. It is the appropriate P1 replay reference,
not independent experimental data. Both archived versions predate the final
December 2025 paper; agreement between archive scenario, published figure and
analysis script still needs explicit inspection.

The actual ZIP64 inventory contains 79,153 entries, including 78,574 files. The
following complete count/thickness pairs are now stored below
`.peykar/external/vcornea-reference-16884414/`:

| Native member directory | Complete files | Count samples |
|---|---|---:|
| `Data/(Slight) 750_G/Comb0/1/Simulation/` | `cell_count_50401.csv`, `thickness_rep_50401.parquet` | 5,041 |
| `Data/(Mild) 1500_G/Comb0/0/Simulation/` | `cell_count_46201.csv`, `thickness_rep_46201.parquet` | 4,621 |
| `Data/(Moderate) 2500_G/Comb1/1/Simulation/` | `cell_count_50401.csv`, `thickness_rep_50401.parquet` | 5,041 |

Member lengths and ZIP CRC32 values passed; SHA-256 values and original archive
offsets are in the acquisition manifest and candidate assessment. The acquisition
transferred 16,317,077 bytes including the central directory. The whole-archive MD5
is explicitly **unverified**. CRC checks and local hashes do not authenticate the
complete remote archive.

Count headers are `Superficial,Wing,Basal,Stem,Time`. The source writes `Time` in
**hours**, with one-hour samples; these values must be converted using 3,600
seconds/hour, not the 360 seconds/MCS lattice-step mapping. Replicate directory IDs
are not verified random seeds. Parameter files are absent from `Data.zip`; match
the selected output to the code archive and explicit scenario configuration before
claiming a seeded replay. Freeze that unmodified scenario, its initial state,
parameters, seed behavior and analysis-defined endpoint. This is an engineering
reference, separate from biological validation.

## Reusable acquisition commands

The acquisition tool verifies the pinned record's complete code ZIP or downloads
explicit data member paths. It rejects ignored/incorrect byte ranges, oversized
reads/members, duplicate names, encrypted/unsupported members, special files,
parent traversal, symlinks and `.git` path components. It never extracts source
code or executes an archive member. Existing different files are preserved and
reported as conflicts. The slow full-data transfer was stopped and its separate
214,036,480-byte `.part` file preserved. It is not running or complete; selected
member acquisition does not overwrite it.

```bash
uv run python scripts/acquire_vcornea.py code --destination .peykar/external/archives
uv run python scripts/acquire_vcornea.py list --contains cell_count_
uv run python scripts/acquire_vcornea.py members \
  --destination .peykar/external/vcornea-reference-16884414 \
  --member 'Data/(Mild) 1500_G/Comb0/0/Simulation/cell_count_46201.csv'
```

The `code` command checks an existing matching ZIP instead of downloading it again.
The `members` command writes a content-identified acquisition manifest after all
selected files pass CRC/size checks and publication. It records the full archive's
MD5 as a declaration, with `archive_md5_verified: false`. Successfully acquired
files can remain after a later selected-member failure; no complete acquisition
manifest is manufactured for that failed invocation.

## EGF and reaction–diffusion decision

Use **CompuCell3D 4.7.0 with vCornea's existing EGF field** for this epic.
The official tag resolves to commit `cea3fa8323e25394db290c0899b7fbd68f17d338`;
its [source license](https://github.com/CompuCell3D/CompuCell3D/blob/cea3fa8323e25394db290c0899b7fbd68f17d338/CompuCell3D/License.txt)
is MIT. Runtime dependencies retain their own licenses. The native CPU PDE solvers
already provide the needed reaction–diffusion machinery; an additional spatial
engine would expand P1 without establishing a missing mechanism.

The [vCornea publication, section 2.5](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1013410)
states that its growth-factor field abstracts several signaling effects and that
its elevated EGF decay incorporates processes including uptake and endocytosis.
Adding an EGFR/uptake model to the unchanged equations risks counting those effects
twice. Such a change needs a new model version, explicit process ownership, a
calibrated interface and its own held-out evidence. It is outside untouched P1 replay.

Small publisher supplements now provide concrete mapping evidence:

| Source | Observation | Admission consequence |
|---|---|---|
| [S5 Text](https://journals.plos.org/ploscompbiol/article/file?id=10.1371/journal.pcbi.1013410.s005&type=supplementary) | 1 MCS = 6 minutes; lattice spacing 2 μm; effective EGF sinks are described | The published mapping is 360 seconds/MCS; confirm the selected archive uses it |
| [S7 Table](https://journals.plos.org/ploscompbiol/article/file?id=10.1371/journal.pcbi.1013410.s012&type=supplementary) | EGF and movement fields are labeled nM; chemical injury uses a scaled amount per voxel | Preserve distinct quantity semantics; normalized figure colors do not establish an absolute concentration mapping |
| [S12 Table](https://journals.plos.org/ploscompbiol/article/file?id=10.1371/journal.pcbi.1013410.s017&type=supplementary) | Turnover ranges were calibration targets; epithelial healing ranges also informed screening | These reused ranges cannot serve as newly independent validation data |

The downloaded supplements are 34,156, 16,191 and 33,636 bytes respectively, with
local SHA-256 values in the machine-readable assessment. They are mathematical
formulations and literature summaries, not raw donor-level observations.

## Complementary models and tools

| Candidate | Verified evidence | Decision and remaining gap |
|---|---|---|
| COPASI 4.47, Build 309 | [Official release](https://github.com/copasi/COPASI/releases/tag/Build-309), commit `59c6c01dc3a8fffee7482d1f620ee393cbff8027`; [Artistic-2.0 license](https://copasi.org/Download/License/); source archive 21,209,201 bytes with publisher SHA-256 | Defer installation. A useful later SBML/ODE reference tool; no spatial corneal adapter or calibrated coupling is supplied by installation alone |
| Schoeberl2002 EGF MAPK, BioModels `BIOMD0000000019` | [Official model identifier](https://www.ebi.ac.uk/biomodels/BIOMD0000000019); [BioModels CC0 policy](https://www.ebi.ac.uk/training/online/courses/biomodels-quick-tour/what-is-biomodels/) | Not admitted. Model/file endpoints returned HTTP 403; exact revision, file hash, compartments, units and applicable cell context remain unverified. Repository policy alone is not an inspected per-file admission record |

Neither candidate is claimed to be validated for corneal epithelium. A later EGF
epic should first define which existing vCornea process is being replaced, the
observation to predict and a suitable cell/context-specific experiment. It can then
select an adequately documented molecular model, pin its SBML and solver, reproduce
its original benchmark and test the coupling's numerical and biological behavior.

## Small experimental ingestion workflow

An openly licensed, small quantitative source is [Rho, Park and Kang (2015),
PLOS ONE 10:e0138020](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0138020),
under CC-BY-4.0. Its Results paragraph reports scratch-wound closure means and SDs
for SV40-immortalized human corneal epithelial cells at 24 and 48 hours, with
GM-CSF doses 0, 0.1, 1 and 10 μg/mL. These are aggregate experimental measurements,
not raw donor observations. The source XML is 60,282 bytes; an eight-row transcription,
a two-row untreated-control CSV and an import descriptor are cached alongside it.
Percent means were divided by 100 for the control fraction; no interpolation,
graph digitization or invented initial observation was used.

The small attributed CSV/spec/provenance files are retained under
[`experiments/data/rho2015-scratch/`](../experiments/data/rho2015-scratch/README.md).
The control import was actually run against the separate E1 state directory:

```bash
PEYKAR_DATA_DIR=.peykar/vcornea-epic \
  uv run peykar import-csv experiments/data/rho2015-scratch/control.csv \
  --spec experiments/data/rho2015-scratch/import.json
```

The command stored two observed control means with exact raw/normalized digests.
The descriptor labels the data `experimental`/`benchmark`, includes the cell-line
and assay context, and uses a publication/experiment independence group. Raw means,
SD and the transformation record remain separate. The current scalar evaluator
does not use the retained SD as an uncertainty model.

The dataset is **not admitted for vCornea validation**: a cultured monolayer and
GM-CSF perturbation differ from stratified tissue and the model's EGF abstraction.
A justified wound-area observation operator, matched injury/initial conditions,
donor/experiment provenance and independent evaluation protocol are still absent.
Record those gaps instead of renaming the experiment as an EGF benchmark or
reporting scientific validation from a successful CSV import.
