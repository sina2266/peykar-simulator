# Extraction protocol 1.0.0 — maximum keratometry after cross-linking or ring segments

Frozen on 2026-10-30, before any Kmax value of an admitted study was read by the model
developer. It governs how published tables become records of schema
`cornea.kmax-record-1.0.0` (validated by `peykar.validation.cornea_kmax_data`, built by
`scripts/build_kmax_record.py`). Extractors make **no modelling decisions**: no fitting,
plotting, comparisons or trend summaries.

## Scope

How the maximum keratometry (Kmax) of a group of keratoconic or ectatic eyes moves from
its baseline value over weeks to years after corneal collagen cross-linking (any
protocol), after intracorneal (intrastromal) ring segments, after both, or under
observation without treatment. Kmax is the main measure of keratoconus progression and
of the corneal flattening these treatments aim at. In scope are human eyes with
keratoconus, pellucid marginal degeneration or ectasia after refractive surgery. Out of
scope: keratoplasty, refractive surgery alone, cataract or lens surgery, contact-lens
fitting studies without one of the in-scope treatments or an untreated observation
group, and phakic intraocular lenses alone.

## Inclusion

1. In vivo **maximum keratometry** of the same group of eyes at the **baseline**
   (preoperative, or the start of observation) and at one or more later follow-ups:
   the steepest anterior corneal power of the topography or tomography map, labelled
   Kmax, K max, maximum or maximal keratometry, or maximum K. Mean keratometry (Km,
   Kmean), steep-meridian keratometry of the central zone (K2, Ksteep, steep K), flat
   keratometry (K1), posterior keratometry and simulated keratometry (SimK) are not
   Kmax and are not admissible.
2. Units: dioptres. Values in millimetres of radius are not admissible.
3. Reference (time 0): the baseline Kmax of the same group. A series without its
   baseline Kmax is excluded, never back-filled.
4. Group means (± SD/SE) only when no per-eye rows exist. Medians/IQR, ranges,
   proportions (for example "progression of at least 1 D"), p-values, model-adjusted
   estimates and "change" columns without the baseline are not admissible.
5. Exclude groups or analysis sets selected by a post-treatment outcome ("eyes that
   stabilized", "progressors", "responders", eyes with complete data chosen after the
   outcome), retreated eyes counted inside a primary-treatment group, and fellow eyes
   that are neither treated nor an observation group declared before follow-up. Record
   them as excluded series. Ordinary attrition (fewer eyes at later visits without an
   outcome-defined exclusion) is kept, with a note. Groups defined before treatment
   (age, baseline Kmax, cone location, protocol) are kept as separate groups.
6. A whole cohort tabulated together with its pre-treatment subgroups is extracted as
   the subgroups, so no eye is counted twice.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted over
  thead then tbody, cells within a row, from 1, no span expansion) of the article JATS
  XML whose SHA-256 is recorded. Signs are kept as printed (a unicode minus is a
  minus); the builder reads decimal commas and a leading point without its zero.
- `value_kind`: `kmax_absolute` (Kmax at every time, month 0 = baseline) or
  `kmax_change` (Kmax minus the baseline Kmax, change at month 0 = 0, cited as
  `{"definition": "definition:<where>", "value": 0}`; admissible only when the same
  series also reports its baseline Kmax, stored as `reference_cell`). A table that gives
  only a *reduction* (positive numbers for flattening) cannot be used as
  `kmax_change`; without an absolute series and its baseline, the study gets no record.
- Times are converted to months exactly: days/30.4375, weeks × 7/30.4375, years × 12.
  Ranges ("1–3 months"), "final visit" or "last follow-up" without a time and
  intraoperative times are excluded.
- `n` is the number of eyes at baseline; changes in follow-up numbers go to `notes`.
- `device` per series: `pentacam` (Pentacam or other Oculus Scheimpflug), `sirius`,
  `galilei`, `orbscan`, `placido` (a Placido-disc topographer), `other` or `unstated`.
- Visual acuity, refraction, corneal thickness, endothelial cell density and other
  keratometry readings, when tabulated, are mentioned in `notes` only.

## Groups and context

- Procedure context per group:
  - `cxl_epi_off`: epithelium-off cross-linking at the standard irradiance (3 mW/cm²
    for 30 minutes, the Dresden protocol) or with an unstated irradiance;
  - `cxl_accelerated`: epithelium-off cross-linking at a stated irradiance above
    3 mW/cm² (continuous or pulsed);
  - `cxl_epi_on`: transepithelial (epithelium-on) cross-linking, including
    iontophoresis;
  - `cxl_combined`: cross-linking with another corneal procedure in the same group (a
    ring segment, a topography-guided or other excimer ablation, a phakic lens);
  - `icrs`: intracorneal ring segments without cross-linking;
  - `observation`: no treatment during the tabulated follow-up (untreated, sham or
    delayed-treatment control groups before their treatment);
  - `other`: anything else in scope (for example customized or topography-guided
    cross-linking that fits none of the above).
- Indication per group: `keratoconus`, `ectasia_after_refractive` (ectasia after LASIK,
  PRK or SMILE), `pellucid` (pellucid marginal degeneration), `mixed` (several reported
  together) or `unstated`.
- Age group per group: `pediatric` (every patient 18 years or younger, or a group
  described as paediatric), `adult` (every patient older than 18), `mixed` or
  `unstated`.
- Role: `control` = a standard treatment or observation without an investigational
  agent, device or protocol declared as the experimental arm; `treated` = the
  experimental arm. Two standard protocols compared without an investigational agent
  (for example epithelium-off against accelerated cross-linking) are both `control`,
  separate groups, each with its own procedure.
- Species: human only.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-kmax-holdout-v1:`) are extracted by separate custodians who report only
structure and SHA-256 values until the evaluation protocol is frozen.
