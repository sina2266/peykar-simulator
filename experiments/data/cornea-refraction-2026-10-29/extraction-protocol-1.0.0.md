# Extraction protocol 1.0.0 — refraction after corneal laser refractive surgery

Frozen on 2026-10-29, before any refraction value of an admitted study was read by
the model developer. It governs how published tables become records of schema
`cornea.refraction-record-1.0.0` (validated by
`peykar.validation.cornea_refraction_data`). Extractors make **no modelling
decisions**: no fitting, plotting, comparisons or trend summaries.

## Scope

Refractive change after corneal laser refractive surgery: how the spherical
equivalent (SE, dioptres) moves from its preoperative value to near the target in
the first days and then drifts (regression or overcorrection) over months to years
while the epithelium and stroma remodel. In scope are excimer surface ablation,
LASIK and lenticule extraction for myopia, hyperopia or mixed astigmatism. Out of
scope: phakic or other intraocular lenses, lens or cataract surgery, orthokeratology
and contact lenses, intracorneal ring segments, keratoplasty, cross-linking without
a refractive ablation, therapeutic ablation of scars or disease, and eyes with
earlier corneal surgery or ectatic disease (keratoconus, post-LASIK ectasia, radial
keratotomy, keratoplasty).

## Inclusion

1. In vivo SE of operated eyes reported for the same group at the **preoperative
   reference** and at one or more later follow-ups after surgery. SE is sphere plus
   half the cylinder; a table of sphere only or cylinder only is not admissible. The
   refraction method is recorded per series: `manifest` (subjective), `cycloplegic`,
   `objective` (autorefraction or aberrometry) or `unstated`.
2. Reference (time 0): the preoperative SE of the same group, measured as stated.
   A series without its preoperative SE is excluded, never back-filled. The attempted
   correction is not a substitute for the preoperative SE.
3. Target refraction: the intended postoperative SE must be plano (emmetropia) for
   every eye of the group, stated (`plano`) or not stated and not contradicted
   (`unstated`). Groups with intentionally non-plano targets (monovision, deliberate
   undercorrection, presbyopic treatment) are excluded unless the plano-target eyes
   are reported separately. A "deviation from target" or "achieved minus attempted"
   column is admissible as the SE only when every target is plano, with a note.
4. Group means (± SD/SE) only when no per-eye rows exist. Medians/IQR, ranges,
   proportions within ±0.50 or ±1.00 D, attempted-versus-achieved scatter plots,
   p-values, model-adjusted estimates and "change" columns without the reference are
   not admissible.
5. Exclude enhancement or retreatment groups (retreatment is itself an outcome),
   groups or analysis sets selected by a postoperative outcome ("eyes with
   regression", "eyes needing enhancement", "eyes within ±0.50 D", means over eyes
   without complications at every visit), and unoperated fellow eyes. Record them as
   excluded series. Ordinary attrition (fewer eyes at later visits without an
   outcome-defined exclusion) is kept, with a note.

## Values, times and identities

- Every value cites its source table cell `<table-wrap id>!R<r>C<c>` (rows counted
  over thead then tbody, cells within a row, from 1, no span expansion) of the
  article JATS XML whose SHA-256 is recorded, or `Sheet!A1` of a spreadsheet.
  Signs are kept as printed (a unicode minus is a minus).
- `value_kind`: `se_absolute` (SE in dioptres at every time, month 0 = preoperative)
  or `se_change` (SE minus the preoperative SE in dioptres, change at month 0 = 0;
  admissible only when the same series also reports its preoperative SE, stored as
  `reference_se` with its cell).
- Times are converted to months exactly: hours/730.5, days/30.4375,
  weeks × 7/30.4375, years × 12. Ranges ("1–3 months"), "final visit" without a time
  and intraoperative times are excluded.
- `n` is the number of eyes at the preoperative reference; changes in follow-up
  numbers go to `notes`. Visual acuity, cylinder, keratometry or aberrations, when
  tabulated, are mentioned in `notes` only (no such series in schema 1.0.0).

## Groups and context

- Procedure context:
  - `surface_ablation`: PRK with alcohol, mechanical or laser epithelial removal,
    trans-epithelial PRK, LASEK and epi-LASIK, with or without mitomycin C.
  - `lasik`: LASIK with a microkeratome or femtosecond-laser flap, including
    wavefront-guided, wavefront-optimized, topography-guided and other ablation
    profiles.
  - `lenticule_extraction`: SMILE and other femtosecond lenticule extraction
    (CLEAR, SILK, FLEx).
  - `other`: anything else in scope, such as a combined procedure that defines no
    single arm.
- Refractive error per group: `myopia` (including compound myopic astigmatism),
  `hyperopia` (including compound hyperopic astigmatism) or `mixed_astigmatism`.
  Groups defined by the preoperative refraction (low, moderate, high myopia) are
  separate groups with the range in `description`.
- Target per group: `plano` or `unstated` (see inclusion 3).
- Role: `control` = the study's standard procedure and standard postoperative care
  without an investigational agent, device or technique variant declared as the
  experimental arm; `treated` = the experimental arm. Two standard techniques
  compared without an investigational agent (for example SMILE versus femtosecond
  LASIK, or two excimer platforms) are both `control`, separate groups. A study
  comparing procedures in different contexts declares the procedure per group
  (`groups[].procedure`).
- Species is recorded; human studies are the expected case.

## Sealed-test custody

Studies reserved for the sealed test by the frozen reservation (salt
`peykar-refraction-holdout-v1:`) are extracted by a separate custodian who reports
only structure and SHA-256 values until the evaluation protocol is frozen.
