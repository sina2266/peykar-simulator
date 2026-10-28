# Extraction protocol 1.1.0 — corneal wound-closure population dataset

Frozen on 2026-10-28, before any outcome value of a study admitted by
[triage 1.1.0](../../../data/cornea-wound-population-2026-10-27/triage-1.1.0.json)
was read. Everything in [protocol 1.0.0](extraction-protocol-1.0.0.md) still
applies (inclusion, exact time conversion, per-animal preference, control rule for
animals, sealed custody). Version 1.1.0 only adds the rules below, needed for
article-body tables and human clinical studies. Records keep schema
`cornea.wound-study-record-1.0.0`. Extractors still make **no modelling
decisions**: they do not fit, plot, compare with a model or summarize trends.

## 1. Article-body tables

- The full-text JATS XML of the article (Europe PMC/PMC copy) is a valid source
  file; its SHA-256 goes in `source_files`.
- A value cites `<table-wrap id>!R<r>C<c>`: `r` counts every `<tr>` of the table
  (thead rows first, then tbody), `c` counts the cells of that row, both from 1,
  without expanding row/column spans. The cell text is copied into `notes` when
  it carries a ± term, a superscript letter or another annotation.

## 2. Which statistics are admissible

- Per-animal/eye/patient rows first; otherwise **group means** only. SD/SEM and
  the stated `n` are recorded; `n` is the number at hour 0.
- Not admissible: medians/IQR/ranges, healing *rates* or slopes, cumulative or
  summed areas, change-from-baseline rows, scores/grades, counts of healed eyes,
  times to closure and p-values. If a table mixes means and medians across time
  points, only the mean cells are used and the others are listed in `notes`.

## 3. Hour 0

- Absolute areas (any unit, including "percent of the corneal surface") still
  need their own measured hour-0 value ("operation day", "immediately after
  surgery", "day 0", "baseline" at wounding). Without it the series is excluded.
- **Definitional baseline (new):** when the article defines the reported value as
  a percentage/ratio of the initial (or of the stated theoretical initial) defect
  area, an hour-0 entry of 100 % remaining (0 % healed) is added with
  `source_cells` = `definition:<where the definition is stated>`. This applies
  only to such relative quantities.
- If both an absolute series without hour 0 and a relative series exist for the
  same group, the relative series with its definitional baseline is used.

## 4. Times

`operation day`, `postoperative immediate`, `day 0` → 0 h; `post-op day N`,
`day N`, `DN` → 24·N h; hours as stated. Ranges such as "5–7 days" are excluded.

## 5. Injury families and geometry

- Alkali (NaOH) burns: `injury_family: alkali_burn`, diameter = filter-paper or
  trephine diameter from Methods, `basement_membrane: unknown`.
- Photorefractive/phototherapeutic keratectomy (PRK, PTK, transepithelial PRK):
  `keratectomy`, `basement_membrane: removed`, diameter = stated epithelial
  removal/ablation-zone diameter.
- Epithelium-off corneal cross-linking (riboflavin/UVA after epithelial removal):
  `injury_family: other`, `basement_membrane: unknown`, diameter = stated
  epithelial removal diameter. It is not pooled with plain debridement.

## 6. Roles in human clinical studies

Human patients always receive post-operative care. In a human record, `control`
means an arm that receives **only the study's standard post-operative regimen**
(antibiotic, corticosteroid/NSAID, lubricant, bandage contact lens as stated) and
no investigational agent; every investigational arm is `treated`. Two surgical
techniques compared without an investigational agent are both `control`, kept
as separate groups. The regimen is recorded in `notes`. Because the context is
the species group, human controls are never pooled with animal controls except
through the declared all-context fallback of the evaluation.

Animal roles are unchanged: an untreated, vehicle, PBS/saline, empty-carrier or
non-targeting (scrambled/control siRNA) negative-control arm is `control`;
steroids, NSAIDs, drugs, disease (diabetes), genotype changes and environmental
insults (blue light, particulate matter) are `treated`.

## 7. Out of scope (recorded as exclusions)

Infectious keratitis ulcers, chronic persistent/neurotrophic epithelial defects,
limbal-stem-cell-deficiency models and in vitro/ex vivo assays.
