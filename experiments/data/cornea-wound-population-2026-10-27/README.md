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

## Release 1.1.0 (2026-10-28): article tables, more species and injuries

Supplementary spreadsheets were nearly exhausted after round 1, so two more
outcome-blind rounds were run:

1. **Round 2** (`screening/round2/queries.json`): broad wound-area terms and
   human surgical closure terms, supplements only. 2,015 new full texts, 167
   candidates, 78 with spreadsheet members — none admissible (mostly skin
   wounds, omics or in vitro).
2. **Article-body tables** (`scripts/screen_corneal_tables.py`): every table of
   all 3,996 full texts whose caption/header mentions wound area or closure and a
   time unit was written as a masked preview (data cells hidden).
3. **Round 3** (`screening/round3/queries.json`): title/abstract queries without
   a supplement filter, 772 new full texts; no admission.

[Triage 1.1.0](triage-1.1.0.json) (rounds 2 and table route, 310 entries) admitted
10 studies; [triage 1.2.0](triage-1.2.0.json) (round 3, 36 entries) admitted
none. The [1.1.0 split](split-reservation-1.1.0.json) uses the same salted hash
rule as 1.0.0 and reserved one of the ten for the sealed test.

| Study | Context | Split | Notes |
|---|---|---|---|
| [Propolis rat 2013](development/records-1.1.0/propolis-rat-alkali-bmc-cam-2013.json) | rat, 1 mm silver-nitrate cautery (`other`) | development | vehicle vs propolis, means, CC BY 2.0 |
| [Aloe vera rabbit 2020](development/records-1.1.0/aloe-vera-rabbit-alkali-jovr-2020.json) | rabbit alkali burn, 6 mm | development | saline control, steroid therapy, aloe |
| [EPO rabbit 2023](development/records-1.1.0/epo-rabbit-alkali-jovr-2023.json) | rabbit alkali burn, 8 mm | development | saline control vs erythropoietin, 60 days |
| FL2 rat 2021 | rat alkali burn, whole cornea (5 mm **assumed**) | development | CC BY-NC-ND: record private; triage key says "mouse", Methods say rats |
| [PJE blue-light rat 2023](development/records-1.1.0/pje-bluelight-rat-antioxidants-2023.json) | rat debridement (4 mm **assumed**) | development | normal control vs blue light + extract |
| [PJE particulate rat 2021](development/records-1.1.0/pje-upm-rat-antioxidants-2021.json) | rat debridement, 4 mm | development | normal control vs particulate matter + extract |
| [Nicergoline diabetic rat 2021](development/records-1.1.0/nicergoline-diabetic-rat-bmc-ophthalmol-2021.json) | rat debridement, 3 mm | development | all diabetic, so all treated |
| [tPRK human 2022](development/records-1.1.0/tprk-human-bmc-ophthalmol-2022.json) | human PRK (`keratectomy`) | development | four standard-care technique groups, 33–39 eyes each |
| [Insulin CXL human 2025](development/records-1.1.0/insulin-cxl-human-medicina-2025.json) | human epithelium-off CXL (`other`), 8.5 mm | development | standard care vs insulin, 4 patients each |
| i-PRF CXL human 2025 | human CXL, 8 mm | **sealed test** | custodian record, private until unsealed |

Records are built from **cell-citing specs** in `development/specs-1.1.0/`
(`table-id!R<row>C<col>` of the article's JATS XML, copied byte-for-byte under
`development/raw/`) by `scripts/build_corneal_table_record.py`. The rules are
[extraction protocol 1.1.0](../../vcornea/repair-front/population-model/extraction-protocol-1.1.0.md)
(tables, group means only, definitional baseline for values defined relative to
the initial defect, human standard-care controls) and its amendment
[1.1.1](../../vcornea/repair-front/population-model/extraction-protocol-1.1.1.md),
written **after** the two PJE tables failed validation: their "healed %" has a
measured non-zero hour 0, so they are stored as `100 − healed` and normalized by
their own hour 0. The human control rule differs from the animal one on purpose
(patients always get standard post-operative steroid/antibiotic care); human
records stay in the `human` context.

Under the new definitional-baseline rule the custodian also re-examined the two
1.0.0 sealed studies that had been excluded before any evaluation. The IL-10
mouse study (per-mouse % of each mouse's own T0 area, 55 series) became
admissible and joins the 1.1.0 sealed test; the norepinephrine study stays
excluded (no time labels on the assigned sheet; other sheets have an undefined
percentage denominator and no 0 h). An earlier repository check of the nicotine
alkali study (Dryad `mk4r5`) found only neovascular length and qPCR, so it is not
a wound-area source.

Known disclosures: while reading Methods of development studies before the
round-3 decisions, the developer saw day-1/day-2 values of the insulin-CXL
development study, and later the PJE tables; no sealed value was read.

## Legacy `.xls` files

Some supplements are BIFF `.xls`. They are converted offline by the separately
pinned `runtime_profiles/xls-reader` (xlrd 2.0.1, hash-checked install, not in
`uv.lock`) into an inert JSON cell inventory read by
`peykar.data_fabric.spreadsheet.read_cells_json`. Several `.xls` supplements were
actually tab-separated text; those conversions are recorded as errors.
