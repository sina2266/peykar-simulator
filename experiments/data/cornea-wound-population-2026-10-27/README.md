# Corneal wound-closure population dataset, 2026-10-27

A systematic, reproducible search for **longitudinal in-vivo corneal epithelial
wound-area data with machine-readable source values**, built to test whether one
population model can predict new studies. It replaces the earlier keyword-only
repository searches (Hugging Face, Dryad, DataCite), which were restricted to
rabbit mechanical debridement and found almost nothing.

## How the studies were found

1. `scripts/screen_corneal_wound_literature.py` queried Europe PMC for open-access
   articles mentioning corneal epithelial wounds and fluorescein (1,993 unique
   PMCIDs, all full texts screened). Only metadata was recorded: species/method
   keywords, wound sizes, declared supplementary files and figure captions.
   [Summary](screening/literature-screen-summary.json),
   [index](screening/literature-screen-index.json),
   [receipts with SHA-256](screening/literature-screen-receipts.json). A first,
   sequential attempt was stopped for speed and is recorded privately
   (`.peykar/corneal-literature-screen-2026-10-27/STOPPED.json`).
2. A declared metadata rule selected 87 candidates with spreadsheet-like
   supplements and wound/fluorescein time figures, plus 10 title-matched extras.
   `scripts/acquire_corneal_wound_supplements.py` downloaded each Europe PMC
   supplementary archive, kept only spreadsheet/text members and wrote **masked
   previews in which every number is hidden**. Repository links (Figshare, Zenodo,
   OSF, Mendeley Data) found in data-availability statements were checked too.
3. [Triage](triage-1.0.0.json) used only those masked labels, captions and Methods:
   6 studies admitted, 92 excluded with reasons (mostly omics/qPCR tables, in vitro
   scratch assays or no data member).
4. Before any new outcome value was read,
   [the split was frozen](split-reservation-1.0.0.json) by a hash rule
   (`sha256(salt + doi) mod 3 == 0` → sealed test). The three earlier consumed
   studies always stay in development.

## What was admitted

| Study | Species / wound | Split | Status |
|---|---|---|---|
| Abdel-Naby 2017, Suárez-Barrio 2019, Chung 2021 | rabbit, 6–9 mm | development | consumed earlier; existing adapters |
| [DIO mouse, PLOS ONE 2020](development/records/dio-mouse-plos-one-2020.json) | mouse, 2 mm | development | 16 corneas, 0/18/24/30 h, CC0 |
| [Ketamine rat, Sci Rep 2022](development/records/ketamine-rat-sci-rep-2022.json) | rat, whole cornea | development | 15 rats, daily 0–7 d, CC BY 4.0 |
| Zeb1, Commun Biol 2023 | mouse, 2 mm | sealed test | 16 eyes, 0/24/48/72 h, CC BY 4.0 |
| LSC dedifferentiation, Nat Commun 2026 | mouse, 2 mm | sealed test | 48 series, 0/16/24/48/72 h, CC BY-NC-ND 4.0 (private) |
| Norepinephrine, Commun Biol 2025 | mouse | sealed test | **excluded**: no identifiable time labels or hour-0 values |
| IL-10 microbes, Cell Rep 2026 | mouse, 2.5 mm | sealed test | **excluded**: deposited sheet lacks hour-0 rows |

Development records cite every value's source cell; raw files are copied
byte-for-byte under `development/raw/`. Data-quality findings are kept in each
record: the DIO normal-diet 18 h pixel values repeat exactly between corneas
No1–4 and No5–8 (probable copy error in the deposited file, retained, not used to
exclude); the rat sizes are integers with missing days and occasional increases;
the rat wound diameter (5 mm whole cornea) is an **explicit assumption**.

Sealed records were produced by a separate custodian agent and stay outside the
repository (`.peykar/cornea-sealed-test-2026-10-27/`). Their SHA-256 values are
pinned in the frozen evaluation protocol. The model developer saw only counts,
covariates and hashes before the protocol was frozen. The LSC record is derived
from CC BY-NC-ND material and must stay private; only metrics are published.

## Role rule

`control` = healthy young-adult wild-type animals with no treatment, vehicle,
saline/PBS or sham, including prophylactic antibiotic-only care. Steroid/NSAID
care (so all 2019 groups), drugs, genotype changes, diet/disease and bacterial
colonization are `treated`. See `peykar.validation.cornea_population_sources`.

## Legacy `.xls` files

Some supplements are BIFF `.xls`. They are converted offline by the separately
pinned `runtime_profiles/xls-reader` (xlrd 2.0.1, hash-checked install, not in
`uv.lock`) into an inert JSON cell inventory read by
`peykar.data_fabric.spreadsheet.read_cells_json`. Several `.xls` supplements were
actually tab-separated text; those conversions are recorded as errors.
