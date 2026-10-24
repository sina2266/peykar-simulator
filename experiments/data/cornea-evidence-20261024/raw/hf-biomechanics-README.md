---
license: cc-by-4.0
language:
- en
pretty_name: Corneal Biomechanics and Surgically Induced Corneal Astigmatism (CSIA)
tags:
- ophthalmology
- cataract-surgery
- corneal-biomechanics
- corvis-st
- toric-iol
- tabular
- medical
size_categories:
- n<1K
task_categories:
- tabular-regression
configs:
- config_name: development
  data_files: data/development_cohort_202_eyes.csv
- config_name: external_validation
  data_files: data/external_validation_54_eyes.csv
---

# Corneal Biomechanics and Surgically Induced Corneal Astigmatism (CSIA)

De-identified tabular dataset accompanying the study of preoperative corneal biomechanical parameters (Corvis ST) and surgically induced corneal astigmatism (CSIA) after 2.2 mm clear corneal incision cataract surgery. The data support every figure, table, and statistical result in the manuscript and reproduce them end to end via the analysis code.

**Analysis code (GitHub):** https://github.com/Usama1002/corneal-biomechanical-prediction-model

## What is in this dataset

Two cohorts, both operated by the same surgeon using an identical 2.2 mm clear corneal incision at 135 degrees.

| Config | File | Eyes | Patients | Purpose |
|---|---|---|---|---|
| `development` | `data/development_cohort_202_eyes.csv` | 202 (92 OD, 110 OS) | 194 (8 bilateral) | Model development and association analysis |
| `external_validation` | `data/external_validation_54_eyes.csv` | 54 (29 + 25) | 45 (9 bilateral) | Independent out-of-sample validation |

Patient identity was established from full names and replaced with anonymous codes; eyes from the same patient share a code (bilateral patients contribute two rows). The two external validation series are pooled and identified by the `cohort` column. The external eyes share no patient with the development cohort.

## Provenance and measurements

Patients underwent phacoemulsification with a 2.2 mm clear corneal incision at 135 degrees. Preoperative dynamic corneal response parameters were measured with Corvis ST, axial length and white-to-white with IOL Master 700, and total corneal astigmatism with swept-source anterior segment OCT (CASIA2) before surgery and at one month postoperatively. CSIA was computed from pre- and postoperative corneal astigmatism by Alpins vector analysis and decomposed into double-angle Cartesian components.

## Column dictionary

| Column | Description | Units |
|---|---|---|
| `patient_id` | Anonymous patient code (bilateral eyes share a code; validation codes are series-prefixed) | code |
| `sex` | Sex | M / F |
| `age` | Age | years |
| `eye` | Operated eye, OD (right) or OS (left) | OD / OS |
| `iol_type` | Implanted intraocular lens model code | code |
| `iol_diopter` | Intraocular lens power | D |
| `PCT135` | Peripheral corneal thickness at the 135 degree meridian (CASIA2) | um |
| `AL` | Axial length (IOL Master 700) | mm |
| `WTW` | White-to-white corneal diameter (IOL Master 700) | mm |
| `AL1`, `AL2` | Corvis ST applanation length at first and second applanation | mm |
| `AV1`, `AV2` | Corvis ST applanation velocity at first and second applanation | m/s |
| `PD` | Peak distance at highest concavity | mm |
| `HCR` | Highest concavity radius | mm |
| `HCDA` | Highest concavity deformation amplitude | mm |
| `IOPnc` | Uncorrected intraocular pressure | mmHg |
| `bIOP` | Biomechanically corrected intraocular pressure | mmHg |
| `CCT` | Central corneal thickness | um |
| `SPA1` | Stiffness parameter at first applanation | mmHg/mm |
| `ARTh` | Ambrosio relational thickness, horizontal | um |
| `DA_Ratio` | Deformation amplitude ratio | ratio |
| `IR` | Integrated radius | mm^-1 |
| `CBI` | Corneal biomechanical index | index |
| `SSI` | Stress-strain index | index |
| `CBiF` | Corneal biomechanical factor | index |
| `CSIA_mag` | Surgically induced corneal astigmatism magnitude | D |
| `CSIA_meridian` | CSIA meridian | degrees (0 to 180) |
| `J0`, `J45` | Double-angle Cartesian components of the CSIA vector | D |
| `sex_binary` | 1 if female, else 0 | binary |
| `eye_binary` | 1 if OS, else 0 | binary |
| `age_tertile` | Age tertile of the development cohort (young / middle / old) | category |
| `age_tertile_boundaries` | Tertile cutpoints (years), metadata | text |
| `cohort` | External validation series (external_validation config only) | text |

`J0`, `J45`, `sex_binary`, `eye_binary`, `age_tertile`, and `age_tertile_boundaries` are deterministic functions of the other columns and are provided for convenience; the analysis code recomputes them.

## Reproducing the analysis

Clone the code repository, place these CSV files under `data/processed/`, and run the pipeline; every figure, table, and statistic regenerates from the de-identified data with a fixed random seed.

```bash
git clone https://github.com/Usama1002/corneal-biomechanical-prediction-model
cd corneal-biomechanical-prediction-model
pip install -r experiments/requirements.txt
python experiments/run_all.py
```

## Ethics and de-identification

The study was approved by the institutional ethics committee, and the requirement for informed consent was waived due to the retrospective design. Public release of the de-identified data is permitted under the approval. The released data contain no names, no dates, and no direct identifiers; ages are recorded in whole years (maximum 87).

## License

Released under CC-BY-4.0. The accompanying analysis code is released separately under the MIT License at the GitHub repository above.

## Citation

If you use this dataset, please cite the accompanying manuscript and link this dataset and the code repository. A formal citation will be added on publication.
